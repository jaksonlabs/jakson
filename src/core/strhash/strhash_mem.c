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

#include "core/strhash/strhash_mem.h"
#include "core/async/spin.h"
#include "std/sort.h"
#include "core/alloc/trace.h"
#include "utils/time.h"
#include "std/bloom.h"
#include "stdx/slicelist.h"
#include "hash/bern.h"

#define HASHCODE_OF(key)      NG5_HASH_BERNSTEIN(strlen(key), key)

#define SMART_MAP_TAG "strhash-mem"

typedef struct bucket
{
    carbon_slice_list_t slice_list;
} bucket_t;

typedef struct mem_extra
{
    struct vector ofType(bucket) buckets;
} mem_extra_t;

static int this_drop(struct strhash *self);
static int this_put_safe_bulk(struct strhash *self,
                           char *const *keys,
                           const field_sid_t *values,
                           size_t num_pairs);
static int this_put_fast_bulk(struct strhash *self,
                           char *const *keys,
                           const field_sid_t *values,
                           size_t num_pairs);
static int this_put_safe_exact(struct strhash *self, const char *key, field_sid_t value);
static int this_put_fast_exact(struct strhash *self, const char *key, field_sid_t value);
static int this_get_safe(struct strhash *self, field_sid_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys);
static int this_get_safe_exact(struct strhash *self, field_sid_t *out, bool *found_mask, const char *key);
static int this_get_fast(struct strhash *self, field_sid_t **out, char *const *keys, size_t num_keys);
static int this_update_key_fast(struct strhash *self, const field_sid_t *values, char *const *keys,
                             size_t num_keys);
static int this_remove(struct strhash *self, char *const *keys, size_t num_keys);
static int this_free(struct strhash *self, void *ptr);

static int this_insert_bulk(struct vector ofType(bucket) *buckets,
                          char *const *restrict keys,
                          const field_sid_t *restrict values,
                          size_t *restrict bucket_idxs,
                          size_t num_pairs,
                          struct allocator *alloc,
                          struct strhash_counters *counter);

static int this_insert_exact(struct vector ofType(bucket) *buckets, const char *restrict key,
                           field_sid_t value, size_t bucket_idx, struct allocator *alloc, struct strhash_counters *counter);
static int this_fetch_bulk(struct vector ofType(bucket) *buckets, field_sid_t *values_out,
                         bool *key_found_mask,
                         size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                         struct allocator *alloc, struct strhash_counters *counter);
static int this_fetch_single(struct vector ofType(bucket) *buckets,
                           field_sid_t *value_out,
                           bool *key_found,
                           const size_t bucket_idx,
                           const char *key,
                           struct strhash_counters *counter);

static int this_create_extra(struct strhash *self, size_t num_buckets, size_t cap_buckets);
static mem_extra_t *this_get_exta(struct strhash *self);
static int bucket_create(bucket_t *buckets, size_t num_buckets, size_t bucket_cap,
                        struct allocator *alloc);
static int bucket_drop(bucket_t *buckets, size_t num_buckets, struct allocator *alloc);
static int bucket_insert(bucket_t *bucket, const char *restrict key, field_sid_t value,
                        struct allocator *alloc, struct strhash_counters *counter);

bool carbon_strhash_create_inmemory(struct strhash *carbon_parallel_map_exec, const struct allocator *alloc, size_t num_buckets,
                                    size_t cap_buckets)
{
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&carbon_parallel_map_exec->allocator, alloc));

    num_buckets = num_buckets < 1 ? 1 : num_buckets;
    cap_buckets = cap_buckets < 1 ? 1 : cap_buckets;

    carbon_parallel_map_exec->tag = MEMORY_RESIDENT;
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
    NG5_CHECK_SUCCESS(this_create_extra(carbon_parallel_map_exec, num_buckets, cap_buckets));
    return true;
}

static int this_drop(struct strhash *self)
{
    assert(self->tag == MEMORY_RESIDENT);
    mem_extra_t *extra = this_get_exta(self);
    bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);
    NG5_CHECK_SUCCESS(bucket_drop(data, extra->buckets.cap_elems, &self->allocator));
    carbon_vec_drop(&extra->buckets);
    carbon_free(&self->allocator, self->extra);
    return true;
}

static int this_put_safe_bulk(struct strhash *self,
                           char *const *keys,
                           const field_sid_t *values,
                           size_t num_pairs)
{
    assert(self->tag == MEMORY_RESIDENT);
    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_pairs * sizeof(size_t));

    NG5_PREFETCH_WRITE(bucket_idxs);

    for (size_t i = 0; i < num_pairs; i++) {
        const char *key = keys[i];
        hash32_t hash = HASHCODE_OF(key);
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
    }

    NG5_PREFETCH_READ(bucket_idxs);
    NG5_PREFETCH_READ(keys);
    NG5_PREFETCH_READ(values);

    NG5_CHECK_SUCCESS(this_insert_bulk(&extra->buckets, keys, values, bucket_idxs, num_pairs, &self->allocator,
                                 &self->counters));
    NG5_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    return true;
}

static int this_put_safe_exact(struct strhash *self, const char *key, field_sid_t value)
{
    assert(self->tag == MEMORY_RESIDENT);
    mem_extra_t *extra = this_get_exta(self);

    hash32_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucket_idx = hash % extra->buckets.cap_elems;

    NG5_PREFETCH_READ(key);

    NG5_CHECK_SUCCESS(this_insert_exact(&extra->buckets, key, value, bucket_idx, &self->allocator,
                                  &self->counters));

    return true;
}

static int this_put_fast_exact(struct strhash *self, const char *key, field_sid_t value)
{
    return this_put_safe_exact(self, key, value);
}

static int this_put_fast_bulk(struct strhash *self,
                           char *const *keys,
                           const field_sid_t *values,
                           size_t num_pairs)
{
    return this_put_safe_bulk(self, keys, values, num_pairs);
}

static int this_fetch_bulk(struct vector ofType(bucket) *buckets, field_sid_t *values_out,
                         bool *key_found_mask,
                         size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                         struct allocator *alloc, struct strhash_counters *counter)
{
    NG5_UNUSED(counter);
    NG5_UNUSED(alloc);

    slice_handle_t result_handle;
    size_t num_not_found = 0;
    bucket_t *data = (bucket_t *) carbon_vec_data(buckets);

    NG5_PREFETCH_WRITE(values_out);

    for (size_t i = 0; i < num_keys; i++) {
        bucket_t *bucket = data + bucket_idxs[i];
        const char *key = keys[i];
        if (NG5_LIKELY(key != NULL)) {
            carbon_slice_list_lookup(&result_handle, &bucket->slice_list, key);
        } else {
            result_handle.is_contained = true;
            result_handle.value = NG5_NULL_ENCODED_STRING;
        }



        num_not_found += result_handle.is_contained ? 0 : 1;
        key_found_mask[i] = result_handle.is_contained;
        values_out[i] = result_handle.is_contained ? result_handle.value : ((field_sid_t) -1);
    }

    *num_keys_not_found = num_not_found;
    return true;
}

static int this_fetch_single(struct vector ofType(bucket) *buckets,
                           field_sid_t *value_out,
                           bool *key_found,
                           const size_t bucket_idx,
                           const char *key,
                           struct strhash_counters *counter)
{
    NG5_UNUSED(counter);

    slice_handle_t handle;
    bucket_t *data = (bucket_t *) carbon_vec_data(buckets);

    NG5_PREFETCH_WRITE(value_out);
    NG5_PREFETCH_WRITE(key_found);

    bucket_t *bucket = data + bucket_idx;

    /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    carbon_slice_list_lookup(&handle, &bucket->slice_list, key);
    *key_found = !SliceListIsEmpty(&bucket->slice_list) && handle.is_contained;
    *value_out = (*key_found) ? handle.value : ((field_sid_t) -1);

    return true;
}

static int this_get_safe(struct strhash *self, field_sid_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys)
{
    assert(self->tag == MEMORY_RESIDENT);

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    NG5_TRACE(SMART_MAP_TAG, "'get_safe' function invoked for %zu strings", num_keys)

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    field_sid_t *values_out = carbon_malloc(&self->allocator, num_keys * sizeof(field_sid_t));
    bool *found_mask_out = carbon_malloc(&self->allocator, num_keys * sizeof(bool));

    assert(bucket_idxs != NULL);
    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        hash32_t hash = key && strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
        NG5_PREFETCH_READ((bucket_t *) carbon_vec_data(&extra->buckets) + bucket_idxs[i]);
    }

    NG5_TRACE(SMART_MAP_TAG, "'get_safe' function invoke fetch...for %zu strings", num_keys)
    NG5_CHECK_SUCCESS(this_fetch_bulk(&extra->buckets, values_out, found_mask_out, num_not_found, bucket_idxs,
                                keys, num_keys, &self->allocator, &self->counters));
    NG5_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    NG5_TRACE(SMART_MAP_TAG, "'get_safe' function invok fetch: done for %zu strings", num_keys)

    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    *out = values_out;
    *found_mask = found_mask_out;

    carbon_timestamp_t end = carbon_time_now_wallclock();
    NG5_UNUSED(begin);
    NG5_UNUSED(end);
    NG5_TRACE(SMART_MAP_TAG, "'get_safe' function done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static int this_get_safe_exact(struct strhash *self, field_sid_t *out, bool *found_mask, const char *key)
{
    assert(self->tag == MEMORY_RESIDENT);

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    mem_extra_t *extra = this_get_exta(self);

    hash32_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucket_idx = hash % extra->buckets.cap_elems;
    NG5_PREFETCH_READ((bucket_t *) carbon_vec_data(&extra->buckets) + bucket_idx);

    NG5_CHECK_SUCCESS(this_fetch_single(&extra->buckets, out, found_mask, bucket_idx, key, &self->counters));

    return true;
}

static int this_get_fast(struct strhash *self, field_sid_t **out, char *const *keys, size_t num_keys)
{
    bool *found_mask;
    size_t num_not_found;
    int status = this_get_safe(self, out, &found_mask, &num_not_found, keys, num_keys);
    this_free(self, found_mask);
    return status;
}

static int this_update_key_fast(struct strhash *self, const field_sid_t *values, char *const *keys,
                             size_t num_keys)
{
    NG5_UNUSED(self);
    NG5_UNUSED(values);
    NG5_UNUSED(keys);
    NG5_UNUSED(num_keys);
    error(&self->err, NG5_ERR_NOTIMPL);
    carbon_error_print(&self->err);
    return false;
}

static int simple_map_remove(mem_extra_t *extra, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                             struct allocator *alloc, struct strhash_counters *counter)
{
    NG5_UNUSED(counter);
    NG5_UNUSED(alloc);

    slice_handle_t handle;
    bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);

    for (register size_t i = 0; i < num_keys; i++) {
        bucket_t *bucket = data + bucket_idxs[i];
        const char *key = keys[i];

        /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        carbon_slice_list_lookup(&handle, &bucket->slice_list, key);
        if (NG5_LIKELY(handle.is_contained)) {
            SliceListRemove(&bucket->slice_list, &handle);
        }
    }
    return true;
}

static int this_remove(struct strhash *self, char *const *keys, size_t num_keys)
{
    assert(self->tag == MEMORY_RESIDENT);

    mem_extra_t *extra = this_get_exta(self);
    size_t *bucket_idxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        hash32_t hash = HASHCODE_OF(key);
        bucket_idxs[i] = hash % extra->buckets.cap_elems;
    }

    NG5_CHECK_SUCCESS(simple_map_remove(extra, bucket_idxs, keys, num_keys, &self->allocator, &self->counters));
    NG5_CHECK_SUCCESS(carbon_free(&self->allocator, bucket_idxs));
    return true;
}

static int this_free(struct strhash *self, void *ptr)
{
    assert(self->tag == MEMORY_RESIDENT);
    NG5_CHECK_SUCCESS(carbon_free(&self->allocator, ptr));
    return true;
}

NG5_FUNC_UNUSED
static int this_create_extra(struct strhash *self, size_t num_buckets, size_t cap_buckets)
{
    if ((self->extra = carbon_malloc(&self->allocator, sizeof(mem_extra_t))) != NULL) {
        mem_extra_t *extra = this_get_exta(self);
        carbon_vec_create(&extra->buckets, &self->allocator, sizeof(bucket_t), num_buckets);

        /** Optimization: notify the kernel that the list of buckets are accessed randomly (since hash based access)*/
        carbon_vec_memadvice(&extra->buckets, MADV_RANDOM | MADV_WILLNEED);


        bucket_t *data = (bucket_t *) carbon_vec_data(&extra->buckets);
        NG5_CHECK_SUCCESS(bucket_create(data, num_buckets, cap_buckets, &self->allocator));
        return true;
    }
    else {
        error(&self->err, NG5_ERR_MALLOCERR);
        return false;
    }
}

NG5_FUNC_UNUSED
static mem_extra_t *this_get_exta(struct strhash *self)
{
    assert (self->tag == MEMORY_RESIDENT);
    return (mem_extra_t *) (self->extra);
}

NG5_FUNC_UNUSED
static int bucket_create(bucket_t *buckets, size_t num_buckets, size_t bucket_cap,
                        struct allocator *alloc)
{
    NG5_NON_NULL_OR_ERROR(buckets);

    // TODO: parallize this!
    while (num_buckets--) {
        bucket_t *bucket = buckets++;
        carbon_slice_list_create(&bucket->slice_list, alloc, bucket_cap);
    }

    return true;
}

static int bucket_drop(bucket_t *buckets, size_t num_buckets, struct allocator *alloc)
{
    NG5_UNUSED(alloc);
    NG5_NON_NULL_OR_ERROR(buckets);

    while (num_buckets--) {
        bucket_t *bucket = buckets++;
        SliceListDrop(&bucket->slice_list);
    }

    return true;
}

static int bucket_insert(bucket_t *bucket, const char *restrict key, field_sid_t value,
                        struct allocator *alloc, struct strhash_counters *counter)
{
    NG5_UNUSED(counter);
    NG5_UNUSED(alloc);

    NG5_NON_NULL_OR_ERROR(bucket);
    NG5_NON_NULL_OR_ERROR(key);

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

static int this_insert_bulk(struct vector ofType(bucket) *buckets,
                          char *const *restrict keys,
                          const field_sid_t *restrict values,
                          size_t *restrict bucket_idxs,
                          size_t num_pairs,
                          struct allocator *alloc,
                          struct strhash_counters *counter)
{
    NG5_NON_NULL_OR_ERROR(buckets)
    NG5_NON_NULL_OR_ERROR(keys)
    NG5_NON_NULL_OR_ERROR(values)
    NG5_NON_NULL_OR_ERROR(bucket_idxs)

    bucket_t *buckets_data = (bucket_t *) carbon_vec_data(buckets);
    int status = true;
    for (register size_t i = 0; status == true && i < num_pairs; i++) {
        size_t bucket_idx = bucket_idxs[i];
        const char *key = keys[i];
        field_sid_t value = values[i];

        bucket_t *bucket = buckets_data + bucket_idx;
        status = bucket_insert(bucket, key, value, alloc, counter);
    }

    return status;
}

static int this_insert_exact(struct vector ofType(bucket) *buckets, const char *restrict key,
                           field_sid_t value, size_t bucket_idx, struct allocator *alloc, struct strhash_counters *counter)
{
    NG5_NON_NULL_OR_ERROR(buckets)
    NG5_NON_NULL_OR_ERROR(key)

    bucket_t *buckets_data = (bucket_t *) carbon_vec_data(buckets);
    bucket_t *bucket = buckets_data + bucket_idx;
    return bucket_insert(bucket, key, value, alloc, counter);
}
