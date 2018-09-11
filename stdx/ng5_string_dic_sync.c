#include <stdx/ng5_vector.h>
#include <stdx/ng5_spinlock.h>
#include <stdx/ng5_string_map.h>
#include <stdx/ng5_string_dic_sync.h>
#include <stdlib.h>
#include <stdx/ng5_string_map_smart.h>
#include <stdx/ng5_trace_alloc.h>
#include <stdx/ng5_bolster.h>
#include <stdx/ng5_time.h>
#include <stdx/ng5_bloomfilter.h>

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char                               *str;
    bool                                in_use;
};

struct naive_extra {
    ng5_vector_t of_type(entry)        contents;
    ng5_vector_t of_type(string_id_t)  freelist;
    struct string_map                index;
    struct ng5_spinlock                     lock;
};

static int this_drop(struct string_dic *self);
static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings,
        size_t nthreads);
static int this_remove(struct string_dic *self, string_id_t *strings, size_t num_strings);
static int this_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys);
static int this_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys);
static char **this_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids);
static int this_free(struct string_dic *self, void *ptr);

static int this_reset_counters(struct string_dic *self);
static int this_counters(struct string_dic *self, struct string_map_counters *counters);

static void lock(struct string_dic *self);
static void unlock(struct string_dic *self);

static int extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);
static struct naive_extra *this_extra(struct string_dic *self);

static int freelist_pop(string_id_t *out, struct string_dic *self);
static int freelist_push(struct string_dic *self, string_id_t idx);

int string_dic_create_sync(struct string_dic* dic, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads, const ng5_allocator_t* alloc)
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
    ng5_spinlock_lock(&extra->lock);
}

static void unlock(struct string_dic *self)
{
    assert(self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    ng5_spinlock_unlock(&extra->lock);
}

static int extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    self->extra = allocator_malloc(&self->alloc, sizeof(struct naive_extra));
    struct naive_extra *extra = this_extra(self);
    ng5_spinlock_create(&extra->lock);
    check_success(ng5_vector_create(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    check_success(ng5_vector_create(&extra->freelist, &self->alloc, sizeof(string_id_t), capacity));
    struct entry empty = {
        .str    = NULL,
        .in_use = false
    };
    for (size_t i = 0; i < capacity; i++) {
        check_success(ng5_vector_push(&extra->contents, &empty, 1));
        freelist_push(self, i);
    }
    unused(nthreads);

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    check_success(allocator_trace(&hashtable_alloc));
#else
    check_success(allocator_this_or_default(&hashtable_alloc, &self->alloc));
#endif

    check_success(string_hashtable_create_scan1_cache(&extra->index, &hashtable_alloc, num_index_buckets,
              num_index_bucket_cap));
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
    if (unlikely(ng5_vector_is_empty(&extra->freelist))) {
        size_t num_new_pos;
        check_success(ng5_vector_grow(&num_new_pos, &extra->freelist));
        check_success(ng5_vector_grow(NULL, &extra->contents));
        assert (extra->freelist.cap_elems == extra->contents.cap_elems);
        struct entry empty = {
            .in_use = false,
            .str    = NULL
        };
        while (num_new_pos--) {
            size_t new_pos = ng5_vector_len(&extra->contents);
            check_success(ng5_vector_push(&extra->freelist, &new_pos, 1));
            check_success(ng5_vector_push(&extra->contents, &empty, 1));
        }
    }
    *out = *(string_id_t *) ng5_vector_pop(&extra->freelist);
    return STATUS_OK;
}

static int freelist_push(struct string_dic *self, string_id_t idx)
{
    assert (self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    check_success(ng5_vector_push(&extra->freelist, &idx, 1));
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

    ng5_vector_drop(&extra->freelist);
    ng5_vector_drop(&extra->contents);
    string_lookup_drop(&extra->index);
    allocator_free(&self->alloc, self->extra);

    return STATUS_OK;
}

static int this_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings,
        size_t nthreads)
{
    trace(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", num_strings);
    timestamp_t begin = time_current_time_ms();

    unused(nthreads);

    check_tag(self->tag, STRING_DIC_NAIVE)
    lock(self);

    struct naive_extra *extra          = this_extra(self);

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    check_success(allocator_trace(&hashtable_alloc));
#else
    check_success(allocator_this_or_default(&hashtable_alloc, &self->alloc));
#endif


    string_id_t  *ids_out                  = allocator_malloc(&hashtable_alloc, num_strings * sizeof(string_id_t));
    bool         *found_mask;
    string_id_t  *values;
    size_t        num_not_found;

    /* query index for strings to get a boolean mask which strings are new and which must be added */
    /* This is for the case that the string dictionary is not empty to skip processing of those new elements
     * which are already contained */
    trace(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

    /* NOTE: palatalization of the call to this function decreases performance */
    string_lookup_get_safe_bulk(&values, &found_mask, &num_not_found, &extra->index, strings, num_strings);

    /* OPTIMIZATION: use a bloomfilter to check whether a string (which has not appeared in the
     * dictionary before this batch but might occur multiple times in the current batch) was seen
     * before (with a slight prob. of doing too much work) */
    ng5_bloomfilter_t bloomfilter;
    ng5_bloomfilter_create(&bloomfilter, 22 * num_not_found);

    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < num_strings; i++) {

        if (found_mask[i]) {
            ids_out[i] = values[i];
        } else {
            /* This path is taken only for strings that are not already contained in the dictionary. However,
             * since this insertion batch may contain duplicate string, querying for already inserted strings
             * must be done anyway for each string in the insertion batch that is inserted. */

            string_id_t        string_id;
            const char        *key = (const char *)(strings[i]);

            bool               found = false;
            string_id_t        value;

            /* Query the bloomfilter if the key was already seend. If the filter returns "yes", a lookup
             * is requried since the filter maybe made a mistake. Of the filter returns "no", the
             * key is new for sure. In this case, one can skip the lookup into the buckets. */
            hash_t bloom_key = hash_fnv(strlen(key), key); /* using a hash of a key instead of the string key itself avoids reading the entire string for computing k hashes inside the bloomfilter */
            if (ng5_bloomfilter_test_and_set(&bloomfilter, &bloom_key, sizeof(hash_t))) {
                /* ensure that the string really was seen (due to collisions in the bloom filter the key might not
                 * been actually seen) */

                /* query index for strings to get a boolean mask which strings are new and which must be added */
                /* This is for the case that the string was not already contained in the string dictionary but may have
                 * duplicates in this insertion batch that are already inserted */
                string_lookup_get_safe_exact(&value, &found, &extra->index, key);  /* OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
            }

            if (found) {
                ids_out[i] = value;
            } else {

                /* register in contents list */
                panic_if(freelist_pop(&string_id, self) != STATUS_OK, "slot management broken");
                struct entry *entries = (struct entry *) ng5_vector_data(&extra->contents);
                struct entry *entry   = entries + string_id;
                assert (!entry->in_use);
                entry->in_use         = true;
                entry->str            = strdup(strings[i]);
                ids_out[i]            = string_id;

                /* add for not yet registered pairs to buffer for fast import */
                string_lookup_put_fast_exact(&extra->index, strings[i], string_id);
            }
        }
    }

    /* set potential non-null out parameters */
    optional_set_else(out, ids_out, allocator_free(&self->alloc, ids_out));

    /* cleanup */
    allocator_free(&hashtable_alloc, found_mask);
    allocator_free(&hashtable_alloc, values);
    ng5_bloomfilter_drop(&bloomfilter);

    unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    info(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

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

    /* remove strings from contents ng5_vector, and skip duplicates */
    for (size_t i = 0; i < num_strings; i++) {
        string_id_t string_id = strings[i];
        struct entry *entry   = (struct entry *) ng5_vector_data(&extra->contents) + string_id;
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
    timestamp_t begin = time_current_time_ms();
    trace(STRING_DIC_SYNC_TAG, "'locate_safe' function invoked for %zu strings", num_keys)

    check_non_null(self);
    check_non_null(out);
    check_non_null(found_mask);
    check_non_null(num_not_found);
    check_non_null(keys);
    check_non_null(num_keys);
    check_tag(self->tag, STRING_DIC_NAIVE)

    lock(self);
    struct naive_extra *extra = this_extra(self);
    int status = string_lookup_get_safe_bulk(out, found_mask, num_not_found, &extra->index, keys, num_keys);
    unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    trace(STRING_DIC_SYNC_TAG, "'locate_safe' function done: %f seconds spent here", (end-begin)/1000.0f)

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

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    allocator_trace(&hashtable_alloc);
#else
    allocator_this_or_default(&hashtable_alloc, &self->alloc);
#endif

    struct naive_extra *extra = this_extra(self);
    char **result = allocator_malloc(&hashtable_alloc, num_ids * sizeof(char *));
    struct entry *entries = (struct entry *) ng5_vector_data(&extra->contents);

    /* Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    ng5_vector_advise(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < num_ids; i++) {
        string_id_t string_id = ids[i];
        assert(string_id < ng5_vector_len(&extra->contents));
        assert(entries[string_id].in_use);
        result[i] = entries[string_id].str;
    }

    unlock(self);
    return result;
}

static int this_free(struct string_dic *self, void *ptr)
{
    unused(self);

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    check_success(allocator_trace(&hashtable_alloc));
#else
    check_success(allocator_this_or_default(&hashtable_alloc, &self->alloc));
#endif

    return allocator_free(&hashtable_alloc, ptr);
}

static int this_reset_counters(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    check_success(string_lookup_reset_counters(&extra->index));
    return STATUS_OK;
}

static int this_counters(struct string_dic *self, struct string_map_counters *counters)
{
    check_tag(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    check_success(string_lookup_counters(counters, &extra->index));
    return STATUS_OK;
}