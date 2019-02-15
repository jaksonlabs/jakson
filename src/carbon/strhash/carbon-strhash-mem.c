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

#include "carbon/strhash/carbon-strhash-mem.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-sort.h"
#include "carbon/alloc/carbon-alloc_tracer.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-bloom.h"
#include "carbon/carbon-slicelist.h"

#define HASHCODE_OF(key)      CARBON_HASH_BERNSTEIN(strlen(key), key)

#define SMART_MAP_TAG "strhash-mem"

typedef struct bucket
{
    carbon_slice_list_t slice_list;
} bucket_t;

typedef struct mem_extra
{
    carbon_vec_t ofType(bucket) buckets;
} mem_extra_t;

static int this_drop(carbon_strhash_t *self);
static int this_put_safe_bulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t num_pairs);
static int this_put_fast_bulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t num_pairs);
static int this_put_safe_exact(carbon_strhash_t *self, const char *key, carbon_string_id_t value);
static int this_put_fast_exact(carbon_strhash_t *self, const char *key, carbon_string_id_t value);
static int this_get_safe(carbon_strhash_t *self, carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys);
static int this_get_safe_exact(carbon_strhash_t *self, carbon_string_id_t *out, bool *found_mask, const char *key);
static int this_get_fast(carbon_strhash_t *self, carbon_string_id_t **out, char *const *keys, size_t num_keys);
static int this_update_key_fast(carbon_strhash_t *self, const carbon_string_id_t *values, char *const *keys,
                             size_t num_keys);
static int this_remove(carbon_strhash_t *self, char *const *keys, size_t num_keys);
static int this_free(carbon_strhash_t *self, void *ptr);

static int this_insert_bulk(carbon_vec_t ofType(bucket) *buckets,
                          char *const *restrict keys,
                          const carbon_string_id_t *restrict values,
                          size_t *restrict bucket_idxs,
                          size_t num_pairs,
                          carbon_alloc_t *alloc,
                          carbon_string_hash_counters_t *counter);

static int this_insert_exact(carbon_vec_t ofType(bucket) *buckets, const char *restrict key,
                           carbon_string_id_t value, size_t bucket_idx, carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);
static int this_fetch_bulk(carbon_vec_t ofType(bucket) *buckets, carbon_string_id_t *values_out,
                         bool *key_found_mask,
                         size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                         carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);
static int this_fetch_single(carbon_vec_t ofType(bucket) *buckets,
                           carbon_string_id_t *value_out,
                           bool *key_found,
                           const size_t bucket_idx,
                           const char *key,
                           carbon_string_hash_counters_t *counter);

static int this_create_extra(carbon_strhash_t *self, size_t num_buckets, size_t cap_buckets);
static mem_extra_t *this_get_exta(carbon_strhash_t *self);
static int bucket_create(bucket_t *buckets, size_t num_buckets, size_t bucket_cap,
                        carbon_alloc_t *alloc);
static int bucket_drop(bucket_t *buckets, size_t num_buckets, carbon_alloc_t *alloc);
static int bucket_insert(bucket_t *bucket, const char *restrict key, carbon_string_id_t value,
                        carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);

bool carbon_strhash_create_inmemory(carbon_strhash_t *carbon_parallel_map_exec, const carbon_alloc_t *alloc, size_t num_buckets,
                                    size_t cap_buckets)
{
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&carbon_parallel_map_exec->allocator, alloc));

    num_buckets = num_buckets < 1 ? 1 : num_buckets;
    cap_buckets = cap_buckets < 1 ? 1 : cap_buckets;

    carbon_parallel_map_exec->tag = CARBON_STRHASH_INMEMORY;
    carbon_parallel_map_exec->drop = this_drop;
    carbon_parallel_map_exec->put_bulk_safe = this_put_safe_bulk;
    carbon_parallel_map_exec->put_bulk_fast = this_put_fast_bulk;
    carbon_parallel_map_exec->put_exact_safe = this_put_safe_exact;
    carbon_parallel_map_exec->put_exact_fast = this_put_fast_exact;
    carbon_parallel_map_exec->get_bulk_safe = this_get_safe;
    carbon_parallel_map_exec->get_fast = this_get_fast;
    carbon_parallel_map_exec->update_key_fast = this_update_key_fast;
    carbon_parallel_map_exec->remove = this_remove;
    carbon_parallel_map_exec->free = this_free;
    carbon_parallel_map_exec->get_exact_safe = this_get_safe_exact;
    carbon_error_init(&carbon_parallel_map_exec->err);

    carbon_strhash_reset_counters(carbon_parallel_map_exec);
    CARBON_CHECK_SUCCESS(this_create_extra(carbon_parallel_map_exec, num_buckets, cap_buckets));
    return true;
}

static int this_drop(carbon_strhash_t *self)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    mem_extra_t *extra = this_get_exta(self);
    bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);
    CARBON_CHECK_SUCCESS(bucket_drop(data, extra->buckets.cap_elems, &self->allocator));
    carbon_vec_drop(&extra->buckets);
    carbon_free(&self->allocator, self->extra);
    return true;
}

static int this_put_safe_bulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t num_pairs)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_pairs * sizeof(size_t));

    CARBON_PREFETCH_WRITE(bucket_idxs);

    for (size_t i = 0; i < num_pairs; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = HASHCODE_OF(key);
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
    }

    CARBON_PREFETCH_READ(bucket_idxs);
    CARBON_PREFETCH_READ(keys);
    CARBON_PREFETCH_READ(values);

    CARBON_CHECK_SUCCESS(this_insert_bulk(&extra->buckets, keys, values, bucket_idxs, num_pairs, &self->allocator,
                                 &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    return true;
}

static int this_put_safe_exact(carbon_strhash_t *self, const char *key, carbon_string_id_t value)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    mem_extra_t *extra = this_get_exta(self);

    carbon_hash_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucket_idx = hash % extra->buckets.cap_elems;

    CARBON_PREFETCH_READ(key);

    CARBON_CHECK_SUCCESS(this_insert_exact(&extra->buckets, key, value, bucket_idx, &self->allocator,
                                  &self->counters));

    return true;
}

static int this_put_fast_exact(carbon_strhash_t *self, const char *key, carbon_string_id_t value)
{
    return this_put_safe_exact(self, key, value);
}

static int this_put_fast_bulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t num_pairs)
{
    return this_put_safe_bulk(self, keys, values, num_pairs);
}

static int this_fetch_bulk(carbon_vec_t ofType(bucket) *buckets, carbon_string_id_t *values_out,
                         bool *key_found_mask,
                         size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                         carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    slice_handle_t result_handle;
    size_t num_not_found = 0;
    bucket_t *data = (bucket_t *) carbon_vec_data(buckets);

    CARBON_PREFETCH_WRITE(values_out);

    for (size_t i = 0; i < num_keys; i++) {
        bucket_t *bucket = data + bucket_idxs[i];
        const char *key = keys[i];
        if (CARBON_LIKELY(key != NULL)) {
            carbon_slice_list_lookup(&result_handle, &bucket->slice_list, key);
        } else {
            result_handle.is_contained = true;
            result_handle.value = CARBON_NULL_ENCODED_STRING;
        }



        num_not_found += result_handle.is_contained ? 0 : 1;
        key_found_mask[i] = result_handle.is_contained;
        values_out[i] = result_handle.is_contained ? result_handle.value : ((carbon_string_id_t) -1);
    }

    *num_keys_not_found = num_not_found;
    return true;
}

static int this_fetch_single(carbon_vec_t ofType(bucket) *buckets,
                           carbon_string_id_t *value_out,
                           bool *key_found,
                           const size_t bucket_idx,
                           const char *key,
                           carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);

    slice_handle_t handle;
    bucket_t *data = (bucket_t *) carbon_vec_data(buckets);

    CARBON_PREFETCH_WRITE(value_out);
    CARBON_PREFETCH_WRITE(key_found);

    bucket_t *bucket = data + bucket_idx;

    /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    carbon_slice_list_lookup(&handle, &bucket->slice_list, key);
    *key_found = !SliceListIsEmpty(&bucket->slice_list) && handle.is_contained;
    *value_out = (*key_found) ? handle.value : ((carbon_string_id_t) -1);

    return true;
}

static int this_get_safe(carbon_strhash_t *self, carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invoked for %zu strings", num_keys)

    carbon_alloc_t hashtable_alloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    carbon_string_id_t *values_out = carbon_malloc(&self->allocator, num_keys * sizeof(carbon_string_id_t));
    bool *found_mask_out = carbon_malloc(&self->allocator, num_keys * sizeof(bool));

    assert(bucket_idxs != NULL);
    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = key && strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
        CARBON_PREFETCH_READ((bucket_t *) carbon_vec_data(&extra->buckets) + bucket_idxs[i]);
    }

    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invoke fetch...for %zu strings", num_keys)
    CARBON_CHECK_SUCCESS(this_fetch_bulk(&extra->buckets, values_out, found_mask_out, num_not_found, bucket_idxs,
                                keys, num_keys, &self->allocator, &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invok fetch: done for %zu strings", num_keys)

    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    *out = values_out;
    *found_mask = found_mask_out;

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static int this_get_safe_exact(carbon_strhash_t *self, carbon_string_id_t *out, bool *found_mask, const char *key)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    carbon_alloc_t hashtable_alloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    mem_extra_t *extra = this_get_exta(self);

    carbon_hash_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucket_idx = hash % extra->buckets.cap_elems;
    CARBON_PREFETCH_READ((bucket_t *) carbon_vec_data(&extra->buckets) + bucket_idx);

    CARBON_CHECK_SUCCESS(this_fetch_single(&extra->buckets, out, found_mask, bucket_idx, key, &self->counters));

    return true;
}

static int this_get_fast(carbon_strhash_t *self, carbon_string_id_t **out, char *const *keys, size_t num_keys)
{
    bool *found_mask;
    size_t num_not_found;
    int status = this_get_safe(self, out, &found_mask, &num_not_found, keys, num_keys);
    this_free(self, found_mask);
    return status;
}

static int this_update_key_fast(carbon_strhash_t *self, const carbon_string_id_t *values, char *const *keys,
                             size_t num_keys)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(values);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_keys);
    CARBON_ERROR(&self->err, CARBON_ERR_NOTIMPL);
    carbon_error_print(&self->err);
    return false;
}

static int simple_map_remove(mem_extra_t *extra, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                             carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    slice_handle_t handle;
    bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);

    for (register size_t i = 0; i < num_keys; i++) {
        bucket_t *bucket = data + bucket_idxs[i];
        const char *key = keys[i];

        /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        carbon_slice_list_lookup(&handle, &bucket->slice_list, key);
        if (CARBON_LIKELY(handle.is_contained)) {
            SliceListRemove(&bucket->slice_list, &handle);
        }
    }
    return true;
}

static int this_remove(carbon_strhash_t *self, char *const *keys, size_t num_keys)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = HASHCODE_OF(key);
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
    }

    CARBON_CHECK_SUCCESS(simple_map_remove(extra, bucket_idxs, keys, num_keys, &self->allocator, &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    return true;
}

static int this_free(carbon_strhash_t *self, void *ptr)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, ptr));
    return true;
}

CARBON_FUNC_UNUSED
static int this_create_extra(carbon_strhash_t *self, size_t num_buckets, size_t cap_buckets)
{
    if ((self->extra = carbon_malloc(&self->allocator, sizeof(mem_extra_t))) != NULL) {
        mem_extra_t *extra = this_get_exta(self);
        carbon_vec_create(&extra->buckets, &self->allocator, sizeof(bucket_t), num_buckets);

        /** Optimization: notify the kernel that the list of buckets are accessed randomly (since hash based access)*/
        carbon_vec_memadvice(&extra->buckets, MADV_RANDOM | MADV_WILLNEED);


        bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);
        CARBON_CHECK_SUCCESS(bucket_create(data, num_buckets, cap_buckets, &self->allocator));
        return true;
    }
    else {
        CARBON_ERROR(&self->err, CARBON_ERR_MALLOCERR);
        return false;
    }
}

CARBON_FUNC_UNUSED
static mem_extra_t *this_get_exta(carbon_strhash_t *self)
{
    assert (self->tag == CARBON_STRHASH_INMEMORY);
    return (mem_extra_t *) (self->extra);
}

CARBON_FUNC_UNUSED
static int bucket_create(bucket_t *buckets, size_t num_buckets, size_t bucket_cap,
                        carbon_alloc_t *alloc)
{
    CARBON_NON_NULL_OR_ERROR(buckets);

    // TODO: parallize this!
    while (num_buckets--) {
        bucket_t *bucket = buckets++;
        carbon_slice_list_create(&bucket->slice_list, alloc, bucket_cap);
    }

    return true;
}

static int bucket_drop(bucket_t *buckets, size_t num_buckets, carbon_alloc_t *alloc)
{
    CARBON_UNUSED(alloc);
    CARBON_NON_NULL_OR_ERROR(buckets);

    while (num_buckets--) {
        bucket_t *bucket = buckets++;
        SliceListDrop(&bucket->slice_list);
    }

    return true;
}

static int bucket_insert(bucket_t *bucket, const char *restrict key, carbon_string_id_t value,
                        carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    CARBON_NON_NULL_OR_ERROR(bucket);
    CARBON_NON_NULL_OR_ERROR(key);

    slice_handle_t handle;

    /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    carbon_slice_list_lookup(&handle, &bucket->slice_list, key);

    if (handle.is_contained) {
        /** entry found by keys */
        assert(value == handle.value);
        //debug(SMART_MAP_TAG, "debug(SMART_MAP_TAG, \"*** put *** '%s' into bucket [new]\", keys);*** put *** '%s' into bucket [already contained]", keys);
    }
    else {
        /** no entry found */
        //debug(SMART_MAP_TAG, "*** put *** '%s' into bucket [new]", keys);
        carbon_slice_list_insert(&bucket->slice_list, (char **) &key, &value, 1);
    }

    return true;
}

static int this_insert_bulk(carbon_vec_t ofType(bucket) *buckets,
                          char *const *restrict keys,
                          const carbon_string_id_t *restrict values,
                          size_t *restrict bucket_idxs,
                          size_t num_pairs,
                          carbon_alloc_t *alloc,
                          carbon_string_hash_counters_t *counter)
{
    CARBON_NON_NULL_OR_ERROR(buckets)
    CARBON_NON_NULL_OR_ERROR(keys)
    CARBON_NON_NULL_OR_ERROR(values)
    CARBON_NON_NULL_OR_ERROR(bucket_idxs)

    bucket_t *buckets_data = (bucket_t *) carbon_vec_data(buckets);
    int status = true;
    for (register size_t i = 0; status == true && i < num_pairs; i++) {
        size_t bucket_idx = bucket_idxs[i];
        const char *key = keys[i];
        carbon_string_id_t value = values[i];

        bucket_t *bucket = buckets_data + bucket_idx;
        status = bucket_insert(bucket, key, value, alloc, counter);
    }

    return status;
}

static int this_insert_exact(carbon_vec_t ofType(bucket) *buckets, const char *restrict key,
                           carbon_string_id_t value, size_t bucket_idx, carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_NON_NULL_OR_ERROR(buckets)
    CARBON_NON_NULL_OR_ERROR(key)

    bucket_t *buckets_data = (bucket_t *) carbon_vec_data(buckets);
    bucket_t *bucket = buckets_data + bucket_idx;
    return bucket_insert(bucket, key, value, alloc, counter);
}
