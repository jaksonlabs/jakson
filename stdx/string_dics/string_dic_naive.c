#include <stdx/vector.h>
#include <stdx/asnyc.h>
#include <stdx/string_hashtable.h>
#include <stdx/string_dics/string_dic_naive.h>
#include <stdx/string_hashtables/simple_scan1-parallel.h>

struct entry {
    char                               *str;
    bool                                in_use;
};

struct naive_extra {
    struct vector of_type(entry)        contents;
    struct vector of_type(string_id_t)  freelist;
    struct string_hashtable             index;
    struct spinlock                     lock;
};

static int this_drop(struct string_dic *self);
static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings);
static int this_remove(struct string_dic *self, string_id_t *strings, size_t num_strings);
static int this_locate_test(struct string_dic *self, string_id_t **out, bool **found_mask,
                            size_t *num_not_found, char *const *keys, size_t num_keys);
static int this_locate_blind(struct string_dic *self, string_id_t **out, char *const *keys,
                             size_t num_keys);
static int this_extract(struct string_dic *self, const char **strings, size_t *num_out,
                        const string_id_t *ids, size_t num_ids);
static int this_free(struct string_dic *self, void *ptr);

static void lock(struct string_dic *self);
static void unlock(struct string_dic *self);

static int extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);
static struct naive_extra *this_extra(struct string_dic *self);

static int freelist_pop(string_id_t *out, struct string_dic *self);
static int freelist_push(struct string_dic *self, string_id_t idx);

int string_dic_create_naive(struct string_dic *dic, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads, const struct allocator *alloc)
{
    check_non_null(dic);
    check_success(allocator_this_or_default(&dic->alloc, alloc));

    dic->tag          = STRING_DIC_NAIVE;
    dic->drop         = this_drop;
    dic->insert       = this_insert;
    dic->remove       = this_remove;
    dic->locate_test  = this_locate_test;
    dic->locate_blind = this_locate_blind;
    dic->extract      = this_extract;

    check_success(extra_create(dic, capacity, num_index_buckets, num_index_bucket_cap, nthreads));
    return STATUS_OK;
}

static void lock(struct string_dic *self)
{
    assert(self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    spinlock_lock(&extra->lock);
}

static void unlock(struct string_dic *self)
{
    assert(self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    spinlock_unlock(&extra->lock);
}

static int extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    self->extra = allocator_malloc(&self->alloc, sizeof(struct naive_extra));
    struct naive_extra *extra = this_extra(self);
    spinlock_create(&extra->lock);
    check_success(vector_create(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    check_success(vector_create(&extra->freelist, &self->alloc, sizeof(string_id_t), capacity));
    struct entry empty = {
        .str    = NULL,
        .in_use = false
    };
    for (size_t i = 0; i < capacity; i++) {
        check_success(vector_push(&extra->contents, &empty, 1));
        check_success(vector_push(&extra->freelist, &i, 1));
    }
    check_success(string_hashtable_create_scan1_parallel(&extra->index, &self->alloc, num_index_buckets,
            num_index_bucket_cap, 1.7f, nthreads));
    return STATUS_OK;
}

static struct naive_extra *this_extra(struct string_dic *self)
{
    assert (self->tag == STRING_DIC_NAIVE);
    return (struct naive_extra *) self->extra;
}

static int freelist_pop(string_id_t *out, struct string_dic *self)
{
    assert (self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    if (vector_is_empty(&extra->freelist)) {
        size_t num_new_pos;
        check_success(vector_grow(&num_new_pos, &extra->freelist));
        check_success(vector_grow(NULL, &extra->contents));
        assert (extra->freelist.cap_elems == extra->contents.cap_elems);
        struct entry empty = {
            .in_use = false,
            .str    = NULL
        };
        while (num_new_pos--) {
            check_success(vector_push(&extra->freelist, &num_new_pos, 1));
            check_success(vector_push(&extra->contents, &empty, 1));
        }
    }
    *out = *(string_id_t *) vector_pop(&extra->freelist);
    return STATUS_OK;
}

static int freelist_push(struct string_dic *self, string_id_t idx)
{
    assert (self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    check_success(vector_push(&extra->freelist, &idx, 1));
    assert (extra->freelist.cap_elems == extra->contents.cap_elems);
    return STATUS_OK;
}

static int this_drop(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_NAIVE)

    struct naive_extra *extra = this_extra(self);

    struct entry *entries = (struct entry *) extra->contents.base;
    for (size_t i = 0; i < extra->contents.num_elems; i++) {
        struct entry *entry = entries + i;
        if (entry->in_use) {
            assert (entry->str);
            allocator_free (&self->alloc, entry->str);
            entry->str = NULL;
        }
    }

    vector_drop(&extra->freelist);
    vector_drop(&extra->contents);
    string_hashtable_drop(&extra->index);

    return STATUS_OK;
}

static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_NAIVE)

    size_t              num_not_found;
    bool               *found_mask;
    uint64_t           *values;

    struct naive_extra *extra          = this_extra(self);
    string_id_t        *ids_out        = allocator_malloc(&self->alloc, num_strings * sizeof(string_id_t));

    /* query index for strings to get a boolean mask which strings are new and which must be added */
    string_id_map_get_test(&values, &found_mask, &num_not_found, &extra->index, strings, num_strings);

    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < num_strings; i++) {
        if (found_mask[i]) {
            ids_out[i] = values[i];
        } else {
            string_id_t string_id;
            check_success(freelist_pop(&string_id, self));
            struct entry *entries = (struct entry *) vector_data(&extra->contents);
            struct entry *entry   = entries + string_id;
            assert (!entry->in_use && !entries->str);
            entry->in_use         = true;
            entry->str            = strdup(strings[i]);
            ids_out[i]            = string_id;
        }
    }

    /* set potential non-null out parameters */
    optional_set_else(out, ids_out, allocator_free(&self->alloc, ids_out));

    /* clean up */
    string_id_map_free(values, &extra->index);
    string_id_map_free(found_mask, &extra->index);

    return STATUS_OK;

}

static int this_remove(struct string_dic *self, string_id_t *strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
}

static int this_locate_test(struct string_dic *self, string_id_t **out, bool **found_mask,
                            size_t *num_not_found, char *const *keys, size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
}

static int this_locate_blind(struct string_dic *self, string_id_t **out, char *const *keys,
                             size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_NAIVE)

    bool   *found_mask;
    size_t  num_not_found;

    /* use safer but in principle more slower implementation */
    int     result         = this_locate_test(self, out, &found_mask, &num_not_found, keys, num_keys);

    /* cleanup */
    this_free(self, found_mask);

    return  result;
}

static int this_extract(struct string_dic *self, const char **strings, size_t *num_out,
                        const string_id_t *ids, size_t num_ids)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
}

static int this_free(struct string_dic *self, void *ptr)
{
    return allocator_free(&self->alloc, ptr);
}
