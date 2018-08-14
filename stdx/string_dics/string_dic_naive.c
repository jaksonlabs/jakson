#include <stdx/vector.h>
#include <stdx/async.h>
#include <stdx/string_lookup.h>
#include <stdx/string_dics/string_dic_naive.h>
#include <stdx/string_lookups/simple_scan1-parallel.h>
#include <stdlib.h>
#include <stdx/string_lookups/simple_scan1-cache.h>

struct entry {
    char                               *str;
    bool                                in_use;
};

struct naive_extra {
    struct vector of_type(entry)        contents;
    struct vector of_type(string_id_t)  freelist;
    struct string_lookup                index;
    struct spinlock                     lock;
};

static int this_drop(struct string_dic *self);
static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings);
static int this_remove(struct string_dic *self, string_id_t *strings, size_t num_strings);
static int this_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys);
static int this_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys);
static char **this_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids);
static int this_free(struct string_dic *self, void *ptr);

static int this_reset_counters(struct string_dic *self);
static int this_counters(struct string_dic *self, struct string_lookup_counters *counters);

static void lock(struct string_dic *self);
static void unlock(struct string_dic *self);

static int extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);
static struct naive_extra *this_extra(struct string_dic *self);

static int freelist_pop(string_id_t *out, struct string_dic *self);
static int freelist_push(struct string_dic *self, string_id_t idx);

int string_dic_create_naive(struct string_dic* dic, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads, const struct allocator* alloc)
{
    check_non_null(dic);
    check_success(allocator_this_or_default(&dic->alloc, alloc));

    dic->tag            = STRING_DIC_NAIVE;
    dic->drop           = this_drop;
    dic->insert         = this_insert;
    dic->remove         = this_remove;
    dic->locate_safe    = this_locate_safe;
    dic->locate_fast    = this_locate_fast;
    dic->extract        = this_extract;
    dic->free           = this_free;
    dic->reset_counters = this_reset_counters;
    dic->counters       = this_counters;

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
        freelist_push(self, i);
    }
    unused(nthreads);
      check_success(string_hashtable_create_scan1_cache(&extra->index, &self->alloc, num_index_buckets,
              num_index_bucket_cap, 1.7f));
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
    if (unlikely(vector_is_empty(&extra->freelist))) {
        size_t num_new_pos;
        check_success(vector_grow(&num_new_pos, &extra->freelist));
        check_success(vector_grow(NULL, &extra->contents));
        assert (extra->freelist.cap_elems == extra->contents.cap_elems);
        struct entry empty = {
            .in_use = false,
            .str    = NULL
        };
        while (num_new_pos--) {
            size_t new_pos = vector_len(&extra->contents);
            check_success(vector_push(&extra->freelist, &new_pos, 1));
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

    lock(self);

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
    string_lookup_drop(&extra->index);

    unlock(self);

    return STATUS_OK;
}

static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
    lock(self);



    struct naive_extra *extra          = this_extra(self);
    string_id_t        *ids_out        = allocator_malloc(&self->alloc, num_strings * sizeof(string_id_t));


    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < num_strings; i++) {

        size_t              num_not_found;
        bool               *found_mask;
        string_id_t        *values;

        /* query index for strings to get a boolean mask which strings are new and which must be added */
        string_lookup_get_safe(&values, &found_mask, &num_not_found, &extra->index, strings + i, 1);

        if (found_mask[0]) {
            ids_out[i] = values[0];

            /* cleanup */
         //   string_lookup_free(values, &extra->index);    // TODO: Memory Leak!
        //    string_lookup_free(found_mask, &extra->index);
        } else {
            string_id_t string_id;

            /* register in contents list */
            panic_if(freelist_pop(&string_id, self) != STATUS_OK, "slot management broken");
            struct entry *entries = (struct entry *) vector_data(&extra->contents);
            struct entry *entry   = entries + string_id;
            assert (!entry->in_use);
            entry->in_use         = true;
            entry->str            = strdup(strings[i]);
            ids_out[i]            = string_id;

            /* add for not yet registered pairs to buffer for fast import */
            string_lookup_put_fast(&extra->index, &strings[i], &string_id, 1);
        }

        string_lookup_free(values, self);
        string_lookup_free(found_mask, self);
    }

    /* set potential non-null out parameters */
    optional_set_else(out, ids_out, allocator_free(&self->alloc, ids_out));

    unlock(self);

    return STATUS_OK;

}

static int this_remove(struct string_dic *self, string_id_t *strings, size_t num_strings)
{
    check_non_null(self);
    check_non_null(strings);
    check_non_null(num_strings);
    check_tag(self->tag, STRING_DIC_NAIVE)
    lock(self);

    struct naive_extra *extra = this_extra(self);

    size_t num_strings_to_delete = 0;
    char **strings_to_delete = allocator_malloc(&self->alloc, num_strings * sizeof(char *));
    string_id_t *string_ids_to_delete = allocator_malloc(&self->alloc, num_strings * sizeof(string_id_t));

    /* remove strings from contents vector, and skip duplicates */
    for (size_t i = 0; i < num_strings; i++) {
        string_id_t string_id = strings[i];
        struct entry *entry   = (struct entry *) vector_data(&extra->contents) + string_id;
        if (likely(entry->in_use)) {
            strings_to_delete[num_strings_to_delete]    = entry->str;
            string_ids_to_delete[num_strings_to_delete] = strings[i];
            entry->str    = NULL;
            entry->in_use = false;
            num_strings_to_delete++;
            check_success(freelist_push(self, string_id));
        }
    }

    /* remove from index */
    check_success(string_lookup_remove(&extra->index, strings_to_delete, num_strings_to_delete));

    /* free up resources for strings that should be removed */
    for (size_t i = 0; i < num_strings_to_delete; i++) {
        free (strings_to_delete[i]);
    }

    /* cleanup */
    allocator_free(&self->alloc, strings_to_delete);
    allocator_free(&self->alloc, string_ids_to_delete);

    unlock(self);
    return STATUS_OK;
}

static int this_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys)
{
    check_non_null(self);
    check_non_null(out);
    check_non_null(found_mask);
    check_non_null(num_not_found);
    check_non_null(keys);
    check_non_null(num_keys);
    check_tag(self->tag, STRING_DIC_NAIVE)

    lock(self);
    struct naive_extra *extra = this_extra(self);
    int status = string_lookup_get_safe(out, found_mask, num_not_found, &extra->index, keys, num_keys);
    unlock(self);
    return status;
}

static int this_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_NAIVE)

    bool   *found_mask;
    size_t  num_not_found;

    /* use safer but in principle more slower implementation */
    int     result         = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /* cleanup */
    this_free(self, found_mask);

    return  result;
}

static char **this_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids)
{
    if (unlikely(!self || !ids || num_ids == 0 || self->tag != STRING_DIC_NAIVE)) {
        return NULL;
    }

    lock(self);

    struct naive_extra *extra = this_extra(self);
    char **result = allocator_malloc(&self->alloc, num_ids * sizeof(char *));
    struct entry *entries = (struct entry *) vector_data(&extra->contents);

    /* Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    vector_madvise(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < num_ids; i++) {
        string_id_t string_id = ids[i];
        assert(string_id < vector_len(&extra->contents));
        assert(entries[string_id].in_use);
        result[i] = entries[string_id].str;
    }

    unlock(self);
    return result;
}

static int this_free(struct string_dic *self, void *ptr)
{
    return allocator_free(&self->alloc, ptr);
}

static int this_reset_counters(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    check_success(string_lookup_reset_counters(&extra->index));
    return STATUS_OK;
}

static int this_counters(struct string_dic *self, struct string_lookup_counters *counters)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    check_success(string_lookup_counters(counters, &extra->index));
    return STATUS_OK;
}