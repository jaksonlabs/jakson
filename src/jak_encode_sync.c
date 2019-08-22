/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <jak_vector.h>
#include <jak_spinlock.h>
#include <jak_str_hash.h>
#include <jak_encode_sync.h>
#include <jak_str_hash_mem.h>
#include <jak_time.h>
#include <jak_bloom.h>
#include <jak_hash_fnv.h>
#include <jak_hash_add.h>
#include <jak_hash_xor.h>
#include <jak_hash_rot.h>
#include <jak_hash_sax.h>

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char *str;
    bool in_use;
};

struct sync_extra {
    struct jak_vector ofType(entry) contents;
    struct jak_vector ofType(string_id_t_t) freelist;
    struct jak_str_hash index;
    struct spinlock lock;
};

static bool this_drop(struct jak_string_dict *self);

static bool this_insert(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t num_strings,
                        size_t num_threads);

static bool this_remove(struct jak_string_dict *self, jak_archive_field_sid_t *strings, size_t num_strings);

static bool this_locate_safe(struct jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                             char *const *keys, size_t num_keys);

static bool this_locate_fast(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys);

static char **this_extract(struct jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids);

static bool this_free(struct jak_string_dict *self, void *ptr);

static bool this_reset_counters(struct jak_string_dict *self);

static bool this_counters(struct jak_string_dict *self, struct jak_str_hash_counters *counters);

static bool this_num_distinct(struct jak_string_dict *self, size_t *num);

static bool this_get_contents(struct jak_string_dict *self, struct jak_vector ofType (char *) *strings,
                              struct jak_vector ofType(jak_archive_field_sid_t) *string_ids);

static void lock(struct jak_string_dict *self);

static void unlock(struct jak_string_dict *self);

static int create_extra(struct jak_string_dict *self, size_t capacity, size_t num_index_buckets, size_t num_index_bucket_cap,
                        size_t num_threads);

static struct sync_extra *this_extra(struct jak_string_dict *self);

static int freelist_pop(jak_archive_field_sid_t *out, struct jak_string_dict *self);

static int freelist_push(struct jak_string_dict *self, jak_archive_field_sid_t idx);

int encode_sync_create(struct jak_string_dict *dic, size_t capacity, size_t num_indx_buckets, size_t num_index_bucket_cap,
                       size_t num_threads, const struct jak_allocator *alloc)
{
        JAK_ERROR_IF_NULL(dic);

        JAK_check_success(jak_alloc_this_or_std(&dic->alloc, alloc));

        dic->tag = SYNC;
        dic->drop = this_drop;
        dic->insert = this_insert;
        dic->remove = this_remove;
        dic->locate_safe = this_locate_safe;
        dic->locate_fast = this_locate_fast;
        dic->extract = this_extract;
        dic->free = this_free;
        dic->resetCounters = this_reset_counters;
        dic->counters = this_counters;
        dic->num_distinct = this_num_distinct;
        dic->get_contents = this_get_contents;

        JAK_check_success(create_extra(dic, capacity, num_indx_buckets, num_index_bucket_cap, num_threads));
        return true;
}

static void lock(struct jak_string_dict *self)
{
        JAK_ASSERT(self->tag == SYNC);
        struct sync_extra *extra = this_extra(self);
        spin_acquire(&extra->lock);
}

static void unlock(struct jak_string_dict *self)
{
        JAK_ASSERT(self->tag == SYNC);
        struct sync_extra *extra = this_extra(self);
        spin_release(&extra->lock);
}

static int create_extra(struct jak_string_dict *self, size_t capacity, size_t num_index_buckets, size_t num_index_bucket_cap,
                        size_t num_threads)
{
        self->extra = jak_alloc_malloc(&self->alloc, sizeof(struct sync_extra));
        struct sync_extra *extra = this_extra(self);
        spin_init(&extra->lock);
        JAK_check_success(vec_create(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
        JAK_check_success(vec_create(&extra->freelist, &self->alloc, sizeof(jak_archive_field_sid_t), capacity));
        struct entry empty = {.str    = NULL, .in_use = false};
        for (size_t i = 0; i < capacity; i++) {
                JAK_check_success(vec_push(&extra->contents, &empty, 1));
                freelist_push(self, i);
        }
        JAK_UNUSED(num_threads);

        struct jak_allocator hashtable_alloc;
#if defined(JAK_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
        CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
        JAK_check_success(jak_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif

        JAK_check_success(strhash_create_inmemory(&extra->index,
                                                  &hashtable_alloc,
                                                  num_index_buckets,
                                                  num_index_bucket_cap));
        return true;
}

static struct sync_extra *this_extra(struct jak_string_dict *self)
{
        JAK_ASSERT (self->tag == SYNC);
        return (struct sync_extra *) self->extra;
}

static int freelist_pop(jak_archive_field_sid_t *out, struct jak_string_dict *self)
{
        JAK_ASSERT (self->tag == SYNC);
        struct sync_extra *extra = this_extra(self);
        if (JAK_UNLIKELY(vec_is_empty(&extra->freelist))) {
                size_t num_new_pos;
                JAK_check_success(vec_grow(&num_new_pos, &extra->freelist));
                JAK_check_success(vec_grow(NULL, &extra->contents));
                JAK_ASSERT (extra->freelist.cap_elems == extra->contents.cap_elems);
                struct entry empty = {.in_use = false, .str    = NULL};
                while (num_new_pos--) {
                        size_t new_pos = vec_length(&extra->contents);
                        JAK_check_success(vec_push(&extra->freelist, &new_pos, 1));
                        JAK_check_success(vec_push(&extra->contents, &empty, 1));
                }
        }
        *out = *(jak_archive_field_sid_t *) vec_pop(&extra->freelist);
        return true;
}

static int freelist_push(struct jak_string_dict *self, jak_archive_field_sid_t idx)
{
        JAK_ASSERT (self->tag == SYNC);
        struct sync_extra *extra = this_extra(self);
        JAK_check_success(vec_push(&extra->freelist, &idx, 1));
        JAK_ASSERT (extra->freelist.cap_elems == extra->contents.cap_elems);
        return true;
}

static bool this_drop(struct jak_string_dict *self)
{
        JAK_check_tag(self->tag, SYNC)

        struct sync_extra *extra = this_extra(self);

        struct entry *entries = (struct entry *) extra->contents.base;
        for (size_t i = 0; i < extra->contents.num_elems; i++) {
                struct entry *entry = entries + i;
                if (entry->in_use) {
                        JAK_ASSERT (entry->str);
                        jak_alloc_free(&self->alloc, entry->str);
                        entry->str = NULL;
                }
        }

        vec_drop(&extra->freelist);
        vec_drop(&extra->contents);
        strhash_drop(&extra->index);
        jak_alloc_free(&self->alloc, self->extra);

        return true;
}

static bool this_insert(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t num_strings,
                        size_t num_threads)
{
        JAK_trace(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", num_strings);
        timestamp_t begin = time_now_wallclock();

        JAK_UNUSED(num_threads);

        JAK_check_tag(self->tag, SYNC)
        lock(self);

        struct sync_extra *extra = this_extra(self);

        struct jak_allocator hashtable_alloc;
#if defined(JAK_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
        CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
        JAK_check_success(jak_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif

        jak_archive_field_sid_t *ids_out = jak_alloc_malloc(&hashtable_alloc, num_strings * sizeof(jak_archive_field_sid_t));
        bool *found_mask;
        jak_archive_field_sid_t *values;
        size_t num_not_found;

        /** query index for strings to get a boolean mask which strings are new and which must be added */
        /** This is for the case that the string dictionary is not empty to skip processing of those new elements
         * which are already contained */
        JAK_trace(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

        /** NOTE: palatalization of the call to this function decreases performance */
        strhash_get_bulk_safe(&values, &found_mask, &num_not_found, &extra->index, strings, num_strings);

        /** OPTIMIZATION: use a struct jak_bitmap to check whether a string (which has not appeared in the
         * dictionary before this batch but might occur multiple times in the current batch) was seen
         * before (with a slight prob. of doing too much work) */
        struct jak_bitmap bitmap;
        jak_bloom_create(&bitmap, 22 * num_not_found);

        /** copy string ids for already known strings to their result position resp. add those which are new */
        for (size_t i = 0; i < num_strings; i++) {

                if (found_mask[i]) {
                        ids_out[i] = values[i];
                } else {
                        /** This path is taken only for strings that are not already contained in the dictionary. However,
                         * since this insertion batch may contain duplicate string, querying for already inserted strings
                         * must be done anyway for each string in the insertion batch that is inserted. */

                        jak_archive_field_sid_t string_id = 0;
                        const char *key = (const char *) (strings[i]);

                        bool found = false;
                        jak_archive_field_sid_t value;

                        /** Query the struct jak_bitmap if the keys was already seend. If the filter returns "yes", a lookup
                         * is requried since the filter maybe made a mistake. Of the filter returns "no", the
                         * keys is new for sure. In this case, one can skip the lookup into the buckets. */
                        size_t key_length = strlen(key);
                        hash32_t bloom_key = key_length > 0 ? JAK_HASH_FNV(strlen(key), key)
                                                            : 0; /** using a hash of a keys instead of the string keys itself avoids reading the entire string for computing k hashes inside the struct jak_bitmap */
                        if (JAK_BLOOM_TEST_AND_SET(&bitmap, &bloom_key, sizeof(hash32_t))) {
                                /** ensure that the string really was seen (due to collisions in the bloom filter the keys might not
                                 * been actually seen) */

                                /** query index for strings to get a boolean mask which strings are new and which must be added */
                                /** This is for the case that the string was not already contained in the string dictionary but may have
                                 * duplicates in this insertion batch that are already inserted */
                                strhash_get_bulk_safe_exact(&value,
                                                            &found,
                                                            &extra->index,
                                                            key);  /** OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
                        }

                        if (found) {
                                ids_out[i] = value;
                        } else {

                                /** register in contents list */
                                bool pop_result = freelist_pop(&string_id, self);
                                error_print_and_die_if(!pop_result, JAK_ERR_SLOTBROKEN)
                                struct entry *entries = (struct entry *) vec_data(&extra->contents);
                                struct entry *entry = entries + string_id;
                                JAK_ASSERT (!entry->in_use);
                                entry->in_use = true;
                                entry->str = strdup(strings[i]);
                                ids_out[i] = string_id;

                                /** add for not yet registered pairs to buffer for fast import */
                                strhash_put_exact_fast(&extra->index, entry->str, string_id);
                        }
                }
        }

        /** set potential non-null out parameters */
        JAK_optional_set_or_else(out, ids_out, jak_alloc_free(&self->alloc, ids_out));

        /** cleanup */
        jak_alloc_free(&hashtable_alloc, found_mask);
        jak_alloc_free(&hashtable_alloc, values);
        jak_bloom_drop(&bitmap);

        unlock(self);

        timestamp_t end = time_now_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_info(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin) / 1000.0f)

        return true;

}

static bool this_remove(struct jak_string_dict *self, jak_archive_field_sid_t *strings, size_t num_strings)
{
        JAK_ERROR_IF_NULL(self);
        JAK_ERROR_IF_NULL(strings);
        JAK_ERROR_IF_NULL(num_strings);
        JAK_check_tag(self->tag, SYNC)
        lock(self);

        struct sync_extra *extra = this_extra(self);

        size_t num_strings_to_delete = 0;
        char **string_to_delete = jak_alloc_malloc(&self->alloc, num_strings * sizeof(char *));
        jak_archive_field_sid_t *string_ids_to_delete = jak_alloc_malloc(&self->alloc, num_strings * sizeof(jak_archive_field_sid_t));

        /** remove strings from contents JAK_vector, and skip duplicates */
        for (size_t i = 0; i < num_strings; i++) {
                jak_archive_field_sid_t jak_archive_field_sid_t = strings[i];
                struct entry *entry = (struct entry *) vec_data(&extra->contents) + jak_archive_field_sid_t;
                if (JAK_LIKELY(entry->in_use)) {
                        string_to_delete[num_strings_to_delete] = entry->str;
                        string_ids_to_delete[num_strings_to_delete] = strings[i];
                        entry->str = NULL;
                        entry->in_use = false;
                        num_strings_to_delete++;
                        JAK_check_success(freelist_push(self, jak_archive_field_sid_t));
                }
        }

        /** remove from index */
        JAK_check_success(strhash_remove(&extra->index, string_to_delete, num_strings_to_delete));

        /** free up resources for strings that should be removed */
        for (size_t i = 0; i < num_strings_to_delete; i++) {
                free(string_to_delete[i]);
        }

        /** cleanup */
        jak_alloc_free(&self->alloc, string_to_delete);
        jak_alloc_free(&self->alloc, string_ids_to_delete);

        unlock(self);
        return true;
}

static bool this_locate_safe(struct jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                             char *const *keys, size_t num_keys)
{
        timestamp_t begin = time_now_wallclock();
        JAK_trace(STRING_DIC_SYNC_TAG, "'locate_safe' function invoked for %zu strings", num_keys)

        JAK_ERROR_IF_NULL(self);
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found_mask);
        JAK_ERROR_IF_NULL(num_not_found);
        JAK_ERROR_IF_NULL(keys);
        JAK_ERROR_IF_NULL(num_keys);
        JAK_check_tag(self->tag, SYNC)

        lock(self);
        struct sync_extra *extra = this_extra(self);
        int status = strhash_get_bulk_safe(out, found_mask, num_not_found, &extra->index, keys, num_keys);
        unlock(self);

        timestamp_t end = time_now_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_trace(STRING_DIC_SYNC_TAG, "'locate_safe' function done: %f seconds spent here", (end - begin) / 1000.0f)

        return status;
}

static bool this_locate_fast(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys)
{
        JAK_check_tag(self->tag, SYNC)

        bool *found_mask;
        size_t num_not_found;

        /** use safer but in principle more slower implementation */
        int result = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

        /** cleanup */
        this_free(self, found_mask);

        return result;
}

static char **this_extract(struct jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids)
{
        if (JAK_UNLIKELY(!self || !ids || num_ids == 0 || self->tag != SYNC)) {
                return NULL;
        }

        lock(self);

        struct jak_allocator hashtable_alloc;
#if defined(JAK_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
        allocatorTrace(&hashtable_alloc);
#else
        jak_alloc_this_or_std(&hashtable_alloc, &self->alloc);
#endif

        struct sync_extra *extra = this_extra(self);
        char **result = jak_alloc_malloc(&hashtable_alloc, num_ids * sizeof(char *));
        struct entry *entries = (struct entry *) vec_data(&extra->contents);

        /** Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
        vec_memadvice(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

        for (size_t i = 0; i < num_ids; i++) {
                jak_archive_field_sid_t jak_archive_field_sid_t = ids[i];
                JAK_ASSERT(jak_archive_field_sid_t < vec_length(&extra->contents));
                JAK_ASSERT(jak_archive_field_sid_t == JAK_NULL_ENCODED_STRING || entries[jak_archive_field_sid_t].in_use);
                result[i] = jak_archive_field_sid_t != JAK_NULL_ENCODED_STRING ? entries[jak_archive_field_sid_t].str : JAK_NULL_TEXT;
        }

        unlock(self);
        return result;
}

static bool this_free(struct jak_string_dict *self, void *ptr)
{
        JAK_UNUSED(self);

        struct jak_allocator hashtable_alloc;
#if defined(JAK_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
        CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
        JAK_check_success(jak_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif

        return jak_alloc_free(&hashtable_alloc, ptr);
}

static bool this_reset_counters(struct jak_string_dict *self)
{
        JAK_check_tag(self->tag, SYNC)
        struct sync_extra *extra = this_extra(self);
        JAK_check_success(strhash_reset_counters(&extra->index));
        return true;
}

static bool this_counters(struct jak_string_dict *self, struct jak_str_hash_counters *counters)
{
        JAK_check_tag(self->tag, SYNC)
        struct sync_extra *extra = this_extra(self);
        JAK_check_success(strhash_get_counters(counters, &extra->index));
        return true;
}

static bool this_num_distinct(struct jak_string_dict *self, size_t *num)
{
        JAK_check_tag(self->tag, SYNC)
        struct sync_extra *extra = this_extra(self);
        *num = vec_length(&extra->contents);
        return true;
}

static bool this_get_contents(struct jak_string_dict *self, struct jak_vector ofType (char *) *strings,
                              struct jak_vector ofType(jak_archive_field_sid_t) *string_ids)
{
        JAK_check_tag(self->tag, SYNC);
        struct sync_extra *extra = this_extra(self);

        for (jak_archive_field_sid_t i = 0; i < extra->contents.num_elems; i++) {
                const struct entry *e = vec_get(&extra->contents, i, struct entry);
                if (e->in_use) {
                        vec_push(strings, &e->str, 1);
                        vec_push(string_ids, &i, 1);
                }
        }
        return true;
}