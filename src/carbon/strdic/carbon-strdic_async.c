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

#include "carbon/carbon-alloc.h"
#include "carbon/carbon-vector.h"
#include "carbon/strdic/carbon-strdic-sync.h"
#include "carbon/strdic/carbon-strdic-async.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-strhash.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-parallel.h"
#include "carbon/carbon-slicelist.h"

#define STRING_DIC_ASYNC_TAG "strdic_async"

#define HASH_FUNCTION                  CARBON_HASH_SAX

typedef struct async_extra async_extra;

typedef struct carrier
{
    carbon_strdic_t local_dictionary;
    pthread_t thread;
    size_t id;
} carrier_t;

typedef struct async_extra
{
    carbon_vec_t ofType(carrier) carriers;
    carbon_vec_t ofType(carrier_t *) carrier_mapping;
    carbon_spinlock_t lock;
} async_extra;

typedef struct parallel_insert_arg
{
    carbon_vec_t ofType(char *) strings;
    carbon_string_id_t *out;
    carbon_string_id_t grouping_key;
    carrier_t *carrier;
    bool enable_write_out;
    bool did_work;
    uint_fast16_t insert_num_threads;
} parallel_insert_arg;

typedef struct parallel_remove_arg
{
    carbon_vec_t ofType(carbon_string_id_t) *local_ids;
    carrier_t *carrier;
    int result;
    bool did_work;
} parallel_remove_arg;

typedef struct parallel_locate_arg
{
    carrier_t *carrier;
    carbon_string_id_t *ids_out;
    bool *found_mask_out;
    size_t num_not_found_out;
    carbon_vec_t ofType(char *) keys_in;
    int result;
    bool did_work;
} parallel_locate_arg;

typedef struct parallel_extract_arg
{
    carbon_vec_t ofType(carbon_string_id_t) local_ids_in;
    carbon_strdic_entry_t *entries_out;
    carrier_t *carrier;
    bool did_work;
} parallel_extract_arg;

#define HASHCODE_OF(string)                                                                                            \
    HASH_FUNCTION(strlen(string), string)

#define MAKE_GLOBAL(thread_id, localcarbon_string_id_t)                                                                \
    ((thread_id << 54) | localcarbon_string_id_t)

#define GET_OWNER(globalId)                                                                                            \
    (globalId >> 54)

#define GET_carbon_string_id_t(globalId)                                                                               \
    ((~((carbon_string_id_t) 0)) >> 10 & global_string_id);

static bool this_drop(carbon_strdic_t *self);
static bool this_insert(carbon_strdic_t *self,
                      carbon_string_id_t **out,
                      char *const *strings,
                      carbon_string_id_t grouping_key,
                      size_t num_strings,
                      size_t __num_threads);
static bool this_remove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t num_strings);
static bool this_locate_safe(carbon_strdic_t *self, carbon_string_id_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys);
static bool this_locate_fast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t num_keys);
static carbon_strdic_entry_t *this_extract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t num_ids);
static bool this_free(carbon_strdic_t *self, void *ptr);

static bool this_num_distinct(carbon_strdic_t *self, size_t *num);
static bool this_get_contents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                              carbon_vec_t ofType(carbon_string_id_t) * string_ids,
                              carbon_vec_t ofType(carbon_string_id_t) * grouping_keys);

static bool this_reset_counters(carbon_strdic_t *self);
static bool this_counters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters);

static bool this_lock(carbon_strdic_t *self);
static bool this_unlock(carbon_strdic_t *self);

static bool this_create_extra(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                           size_t approx_num_unique_str, size_t num_threads);

static bool this_setup_carriers(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                             size_t approx_num_unique_str, size_t num_threads);

#define THIS_EXTRAS(self)                                                                                              \
({                                                                                                                     \
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);                                                             \
    (async_extra *) self->extra;                                                                                       \
})

CARBON_EXPORT (int)
carbon_strdic_create_async(carbon_strdic_t *dic, size_t capacity, size_t num_index_buckets,
                               size_t approx_num_unique_strs, size_t num_threads, const carbon_alloc_t *alloc)
{
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&dic->alloc, alloc));

    dic->tag = CARBON_STRDIC_TYPE_ASYNC;
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

    CARBON_CHECK_SUCCESS(this_create_extra(dic, capacity, num_index_buckets, approx_num_unique_strs, num_threads));
    return true;
}

static bool this_create_extra(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                           size_t approx_num_unique_str, size_t num_threads)
{
    assert(self);

    self->extra = carbon_malloc(&self->alloc, sizeof(async_extra));
    async_extra *extra = THIS_EXTRAS(self);
    carbon_spinlock_init(&extra->lock);
    carbon_vec_create(&extra->carriers, &self->alloc, sizeof(carrier_t), num_threads);
    this_setup_carriers(self, capacity, num_index_buckets, approx_num_unique_str, num_threads);
    carbon_vec_create(&extra->carrier_mapping, &self->alloc, sizeof(carrier_t *), capacity);

    return true;
}

static bool this_drop(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    async_extra *extra = THIS_EXTRAS(self);
    for (size_t i = 0; i < extra->carriers.num_elems; i++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, i, carrier_t);
        carbon_strdic_drop(&carrier->local_dictionary);
    }
    CARBON_CHECK_SUCCESS(carbon_vec_drop(&extra->carriers));
    CARBON_CHECK_SUCCESS(carbon_vec_drop(&extra->carrier_mapping));
    CARBON_CHECK_SUCCESS(carbon_free(&self->alloc, extra));
    return true;
}

void *parallel_remove_function(void *args)
{
    parallel_remove_arg *carrier_arg = (parallel_remove_arg *) args;
    carbon_string_id_t len = carbon_vec_length(carrier_arg->local_ids);
    carrier_arg->did_work = len > 0;

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for remove task (%zu elements)", carrier_arg->carrier->id,
          carbon_vec_length(carrier_arg->local_ids));
    if (len > 0) {
        carbon_strdic_t *dic = &carrier_arg->carrier->local_dictionary;
        carbon_string_id_t *ids = CARBON_VECTOR_ALL(carrier_arg->local_ids, carbon_string_id_t);
        carrier_arg->result = carbon_strdic_remove(dic, ids, len);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu task done", carrier_arg->carrier->id);
    }
    else {
        carrier_arg->result = true;
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", carrier_arg->carrier->id);
    }

    return NULL;
}

void *parallel_insert_function(void *args)
{
    parallel_insert_arg *restrict this_args = (parallel_insert_arg *restrict) args;
    this_args->did_work = this_args->strings.num_elems > 0;

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread-local insert function started (thread %zu)", this_args->carrier->id);
    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for insert task (%zu elements)", this_args->carrier->id,
          carbon_vec_length(&this_args->strings));

    if (this_args->did_work) {
        CARBON_TRACE(STRING_DIC_ASYNC_TAG,
              "thread %zu starts insertion of %zu strings",
              this_args->carrier->id,
              carbon_vec_length(&this_args->strings));
        char **data = (char **) carbon_vec_data(&this_args->strings);

        int status = carbon_strdic_insert(&this_args->carrier->local_dictionary,
                                          this_args->enable_write_out ? &this_args->out : NULL,
                                          data, this_args->grouping_key, carbon_vec_length(&this_args->strings), this_args->insert_num_threads);

        /** internal error during thread-local string dictionary building process */
        CARBON_PRINT_ERROR_AND_DIE_IF(status != true, CARBON_ERR_INTERNALERR);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

void *parallel_locate_safe_function(void *args)
{
    parallel_locate_arg *restrict this_args = (parallel_locate_arg *restrict) args;
    this_args->did_work = carbon_vec_length(&this_args->keys_in) > 0;

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread-local 'locate' function invoked for thread %zu...", this_args->carrier->id)

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for locate (safe) task (%zu elements)", this_args->carrier->id,
          carbon_vec_length(&this_args->keys_in));

    if (this_args->did_work) {
        this_args->result = carbon_strdic_locate_safe(&this_args->ids_out,
                                                     &this_args->found_mask_out,
                                                     &this_args->num_not_found_out,
                                                     &this_args->carrier->local_dictionary,
                                                     CARBON_VECTOR_ALL(&this_args->keys_in, char *),
                                                     carbon_vec_length(&this_args->keys_in));

        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

void *parallel_extract_function(void *args)
{
    parallel_extract_arg *restrict this_args = (parallel_extract_arg *restrict) args;
    this_args->did_work = carbon_vec_length(&this_args->local_ids_in) > 0;

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for extract task (%zu elements)", this_args->carrier->id,
          carbon_vec_length(&this_args->local_ids_in));

    if (this_args->did_work) {
        this_args->entries_out = carbon_strdic_extract(&this_args->carrier->local_dictionary,
                                                     CARBON_VECTOR_ALL(&this_args->local_ids_in, carbon_string_id_t),
                                                     carbon_vec_length(&this_args->local_ids_in));
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

static void synchronize(carbon_vec_t ofType(carrier) *carriers, size_t num_threads)
{
    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "barrier installed for %d threads", num_threads);

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        volatile carrier_t *carrier = CARBON_VECTOR_GET(carriers, thread_id, carrier_t);
        pthread_join(carrier->thread, NULL);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %d joined", carrier->id);
    }
    carbon_timestamp_t end = carbon_time_now_wallclock();
    carbon_timestamp_t duration = (end - begin);
    CARBON_UNUSED(duration);

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "barrier passed for %d threads after %f seconds", num_threads, duration / 1000.0f);
}

static void create_thread_assignment(atomic_uint_fast16_t **str_carrier_mapping, atomic_size_t **carrier_num_strings,
                                   size_t **str_carrier_idx_mapping,
                                   carbon_alloc_t *alloc, size_t num_strings, size_t num_threads)
{
    /** carbon_parallel_map_exec string depending on hash values to a particular carrier */
    *str_carrier_mapping = carbon_malloc(alloc, num_strings * sizeof(atomic_uint_fast16_t));
    memset(*str_carrier_mapping, 0, num_strings * sizeof(atomic_uint_fast16_t));

    /** counters to compute how many strings go to a particular carrier */
    *carrier_num_strings = carbon_malloc(alloc, num_threads * sizeof(atomic_size_t));
    memset(*carrier_num_strings, 0, num_threads * sizeof(atomic_size_t));

    /** an inverted index that contains the i-th position for string k that was assigned to carrier m.
     * With this, given a (global) string and and its carrier, one can have directly the position of the
     * string in the carriers "thread-local locate" args */
    *str_carrier_idx_mapping = carbon_malloc(alloc, num_strings * sizeof(size_t));
}

static void drop_thread_assignment(carbon_alloc_t *alloc, atomic_uint_fast16_t *str_carrier_mapping,
                                 atomic_size_t *carrier_num_strings, size_t *str_carrier_idx_mapping)
{
    carbon_free(alloc, carrier_num_strings);
    carbon_free(alloc, str_carrier_mapping);
    carbon_free(alloc, str_carrier_idx_mapping);
}

typedef struct parallel_compute_thread_assignment_arg
{
    atomic_uint_fast16_t *str_carrier_mapping;
    size_t num_threads;
    atomic_size_t *carrier_num_strings;
    char *const *base_strings;
} parallel_compute_thread_assignment_arg_t;

static void parallel_compute_thread_assignment_function(const void *restrict start, size_t width, size_t len,
                                                    void *restrict args, thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_UNUSED(width);

    char *const *strings = (char *const *) start;

    parallel_compute_thread_assignment_arg_t *func_args = (parallel_compute_thread_assignment_arg_t *) args;

    while (len--) {
        size_t i = strings - func_args->base_strings;
        const char *key = *strings;
        /** re-using this hashcode for the thread-local dictionary is more costly than to compute it fresh
         * (due to more I/O with the RAM) */
        size_t thread_id = HASHCODE_OF(key) % func_args->num_threads;
        atomic_fetch_add(&func_args->str_carrier_mapping[i], thread_id);
        atomic_fetch_add(&func_args->carrier_num_strings[thread_id], 1);
        strings++;
    }
}

static void compute_thread_assignment(atomic_uint_fast16_t *str_carrier_mapping, atomic_size_t *carrier_num_strings,
                                    char *const *strings, size_t num_strings, size_t num_threads)
{
    parallel_compute_thread_assignment_arg_t args = {
        .base_strings = strings,
        .carrier_num_strings = carrier_num_strings,
        .num_threads = num_threads,
        .str_carrier_mapping = str_carrier_mapping
    };
    carbon_parallel_for(strings,
                sizeof(char *const *),
                num_strings,
                parallel_compute_thread_assignment_function,
                &args,
                CARBON_PARALLEL_THREAD_HINT_MULTI,
                num_threads);

}

static bool this_insert(carbon_strdic_t *self,
                      carbon_string_id_t **out,
                      char *const *strings,
                      carbon_string_id_t grouping_key,
                      size_t num_strings,
                      size_t __num_threads)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "insert operation invoked: %zu strings in total", num_strings)


    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    /** parameter 'num_threads' must be set to 0 for async dictionary */
    CARBON_PRINT_ERROR_AND_DIE_IF(__num_threads != 0, CARBON_ERR_INTERNALERR);

    this_lock(self);

    async_extra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);

    atomic_uint_fast16_t *str_carrier_mapping;
    size_t *str_carrier_idx_mapping;
    atomic_size_t *carrier_num_strings;


    create_thread_assignment(&str_carrier_mapping, &carrier_num_strings, &str_carrier_idx_mapping,
                           &self->alloc, num_strings, num_threads);

    carbon_vec_t ofType(parallel_insert_arg *) carrier_args;
    carbon_vec_create(&carrier_args, &self->alloc, sizeof(parallel_insert_arg *), num_threads);

    /** compute which carrier is responsible for which string */
    compute_thread_assignment(str_carrier_mapping, carrier_num_strings, strings, num_strings, num_threads);

    /** prepare to move string subsets to carriers */
    for (uint_fast16_t i = 0; i < num_threads; i++) {
        parallel_insert_arg *entry = carbon_malloc(&self->alloc, sizeof(parallel_insert_arg));
        entry->carrier = CARBON_VECTOR_GET(&extra->carriers, i, carrier_t);
        entry->insert_num_threads = num_threads;
        entry->grouping_key = grouping_key;

        carbon_vec_create(&entry->strings, &self->alloc, sizeof(char *), CARBON_MAX(1, carrier_num_strings[i]));
        carbon_vec_push(&carrier_args, &entry, 1);
        assert (entry->strings.base != NULL);

        parallel_insert_arg *carrier_arg = *CARBON_VECTOR_GET(&carrier_args, i, parallel_insert_arg *);
        carrier_arg->out = NULL;
    }

    /** create per-carrier string subset */
    /** parallizing this makes no sense but waste of resources and energy */
    for (size_t i = 0; i < num_strings; i++) {
        uint_fast16_t thread_id = str_carrier_mapping[i];
        parallel_insert_arg *carrier_arg = *CARBON_VECTOR_GET(&carrier_args, thread_id, parallel_insert_arg *);
        carrier_arg->enable_write_out = out != NULL;

        /** store local index of string i inside the thread */
        str_carrier_idx_mapping[i] = carbon_vec_length(&carrier_arg->strings);

        carbon_vec_push(&carrier_arg->strings, &strings[i], 1);
    }


    /** schedule insert operation per carrier */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "schedule insert operation to %zu threads", num_threads)
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_insert_arg *carrier_arg = *CARBON_VECTOR_GET(&carrier_args, thread_id, parallel_insert_arg *);
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        CARBON_TRACE(STRING_DIC_ASYNC_TAG, "create thread %zu...", thread_id)
        pthread_create(&carrier->thread, NULL, parallel_insert_function, carrier_arg);
        CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread %zu created", thread_id)
    }
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "scheduling done for %zu threads", num_threads)

    /** synchronize */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "start synchronizing %zu threads", num_threads)
    synchronize(&extra->carriers, num_threads);
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync", num_threads)

    /** compute string ids; the string id produced by this implementation is a compound identifier encoding
     * both the owning thread id and the thread-local string id. For this, the returned (global) string identifier
     * is split into 10bits encoded the thread (given a maximum of 1024 threads that can be handled by this
     * implementation), and 54bits used to encode the thread-local string id
     *
     * TECHNICAL LIMIT: 1024 threads
     */

    /** optionally, return the created string ids. In case 'out' is NULL, nothing has to be done (especially
     * none of the carrier threads allocated thread-local 'out's which mean that no cleanup must be done */

    /** parallelizing the following block makes no sense but waste of compute power and energy */
    if (CARBON_LIKELY(out != NULL)) {
        carbon_string_id_t *total_out = carbon_malloc(&self->alloc, num_strings * sizeof(carbon_string_id_t));
        size_t currentOut = 0;

        for (size_t string_idx = 0; string_idx < num_strings; string_idx++) {
            uint_fast16_t thread_id = str_carrier_mapping[string_idx];
            size_t localIdx = str_carrier_idx_mapping[string_idx];
            parallel_insert_arg *carrier_arg = *CARBON_VECTOR_GET(&carrier_args, thread_id, parallel_insert_arg *);
            carbon_string_id_t global_string_owner_id = thread_id;
            carbon_string_id_t global_string_local_id = carrier_arg->out[localIdx];
            carbon_string_id_t global_string_id = MAKE_GLOBAL(global_string_owner_id, global_string_local_id);
            total_out[currentOut++] = global_string_id;
        }

        *out = total_out;
    }

    /** cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_insert_arg *carrier_arg = *CARBON_VECTOR_GET(&carrier_args, thread_id, parallel_insert_arg *);
        if (carrier_arg->did_work) {
            carbon_strdic_free(&carrier_arg->carrier->local_dictionary, carrier_arg->out);
        }
        carbon_vec_drop(&carrier_arg->strings);
        carbon_free(&self->alloc, carrier_arg);
    }

    /** cleanup */
    drop_thread_assignment(&self->alloc, str_carrier_mapping, carrier_num_strings, str_carrier_idx_mapping);
    carbon_vec_drop(&carrier_args);

    this_unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_remove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t num_strings)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "remove operation started: %zu strings to remove", num_strings);

    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    this_lock(self);

    parallel_remove_arg empty;
    struct async_extra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);
    size_t approx_num_strings_per_thread = CARBON_MAX(1, num_strings / num_threads);
    carbon_vec_t ofType(carbon_string_id_t) *string_map =
        carbon_malloc(&self->alloc, num_threads * sizeof(carbon_vec_t));

    carbon_vec_t ofType(parallel_remove_arg) carrier_args;
    carbon_vec_create(&carrier_args, &self->alloc, sizeof(parallel_remove_arg), num_threads);

    /** prepare thread-local subset of string ids */
    carbon_vec_repeated_push(&carrier_args, &empty, num_threads);
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carbon_vec_create(string_map + thread_id, &self->alloc, sizeof(carbon_string_id_t), approx_num_strings_per_thread);
    }

    /** compute subset of string ids per thread  */
    for (size_t i = 0; i < num_strings; i++) {
        carbon_string_id_t global_string_id = strings[i];
        uint_fast16_t owning_thread_id = GET_OWNER(global_string_id);
        carbon_string_id_t localcarbon_string_id_t = GET_carbon_string_id_t(global_string_id);
        assert(owning_thread_id < num_threads);

        carbon_vec_push(string_map + owning_thread_id, &localcarbon_string_id_t, 1);
    }

    /** schedule remove operation per carrier */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        parallel_remove_arg *carrier_arg = CARBON_VECTOR_GET(&carrier_args, thread_id, parallel_remove_arg);
        carrier_arg->carrier = carrier;
        carrier_arg->local_ids = string_map + thread_id;

        pthread_create(&carrier->thread, NULL, parallel_remove_function, carrier_arg);
    }

    /** synchronize */
    synchronize(&extra->carriers, num_threads);

    /** cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carbon_vec_drop(string_map + thread_id);
    }

    carbon_free(&self->alloc, string_map);
    carbon_vec_data(&carrier_args);

    this_unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "remove operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_locate_safe(carbon_strdic_t *self, carbon_string_id_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation started: %zu strings to locate", num_keys)

    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    this_lock(self);

    struct async_extra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);

    /** global result output */
    CARBON_MALLOC(carbon_string_id_t, global_out, num_keys, &self->alloc);
    CARBON_MALLOC(bool, global_found_mask, num_keys, &self->alloc);

    size_t global_num_not_found = 0;

    atomic_uint_fast16_t *str_carrier_mapping;
    size_t *str_carrier_idx_mapping;
    atomic_size_t *carrier_num_strings;

    parallel_locate_arg carrier_args[num_threads];

    create_thread_assignment(&str_carrier_mapping, &carrier_num_strings, &str_carrier_idx_mapping,
                           &self->alloc, num_keys, num_threads);

    /** compute which carrier is responsible for which string */
    compute_thread_assignment(str_carrier_mapping, carrier_num_strings, keys, num_keys, num_threads);

    /** prepare to move string subsets to carriers */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_locate_arg *arg = carrier_args + thread_id;
        carbon_vec_create(&arg->keys_in, &self->alloc, sizeof(char *), carrier_num_strings[thread_id]);
        assert (&arg->keys_in.base != NULL);
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "computing per-thread string subset for %zu strings", num_keys)
    /** create per-carrier string subset */
    for (size_t i = 0; i < num_keys; i++) {
        /** get thread responsible for this particular string */
        uint_fast16_t thread_id = str_carrier_mapping[i];

        /** get the thread-local argument list for the thread that is responsible for this particular string */
        parallel_locate_arg *arg = carrier_args + thread_id;

        /** store local index of string i inside the thread */
        str_carrier_idx_mapping[i] = carbon_vec_length(&arg->keys_in);

        /** push that string into the thread-local vector */
        carbon_vec_push(&arg->keys_in, &keys[i], 1);
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "schedule operation to threads to %zu threads...", num_threads)
    /** schedule operation to threads */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        parallel_locate_arg *arg = carrier_args + thread_id;
        carrier_args[thread_id].carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallel_locate_safe_function, arg);
    }

    /** synchronize */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "start syncing %zu threads...", num_threads)
    synchronize(&extra->carriers, num_threads);
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync.", num_threads)

    /** collect and merge results */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "merging results of %zu threads", num_threads)
    for (size_t i = 0; i < num_keys; i++) {
        /** get thread responsible for this particular string, and local position of that string inside the
         * thread storage */
        uint_fast16_t thread_id = str_carrier_mapping[i];
        size_t local_thread_idx = str_carrier_idx_mapping[i];

        /** get the thread-local argument list for the thread that is responsible for this particular string */
        parallel_locate_arg *arg = carrier_args + thread_id;

        /** merge into global result */
        carbon_string_id_t string_id_owner = thread_id;
        carbon_string_id_t string_id_local_idx = arg->ids_out[local_thread_idx];
        carbon_string_id_t global_string_id = MAKE_GLOBAL(string_id_owner, string_id_local_idx);
        global_out[i] = global_string_id;
        global_found_mask[i] = arg->found_mask_out[local_thread_idx];
    }
    for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        /** compute total number of not-found elements */
        parallel_locate_arg *arg = carrier_args + thread_id;
        global_num_not_found += arg->num_not_found_out;

        /** cleanup */
        if (CARBON_LIKELY(arg->did_work)) {
            carbon_strdic_free(&arg->carrier->local_dictionary, arg->found_mask_out);
            carbon_strdic_free(&arg->carrier->local_dictionary, arg->ids_out);
        }
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "cleanup%s", "...")

    /** cleanup */
    drop_thread_assignment(&self->alloc, str_carrier_mapping, carrier_num_strings, str_carrier_idx_mapping);

    for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_locate_arg *arg = carrier_args + thread_id;
        carbon_vec_drop(&arg->keys_in);
    }

    /** return results */
    *out = global_out;
    *found_mask = global_found_mask;
    *num_not_found = global_num_not_found;

    this_unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_locate_fast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t num_keys)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    this_lock(self);

    bool *found_mask;
    size_t num_not_found;
    int result;

    /** use safer but in principle more slower implementation */
    result = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /** cleanup */
    this_free(self, found_mask);

    this_unlock(self);

    return result;
}

static carbon_strdic_entry_t *this_extract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t num_ids)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation started: %zu strings to extract", num_ids)

    if (self->tag != CARBON_STRDIC_TYPE_ASYNC) {
        return NULL;
    }

    this_lock(self);

    CARBON_MALLOC(carbon_strdic_entry_t, globalResult, num_ids, &self->alloc);

    struct async_extra *extra = (struct async_extra *) self->extra;
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);
    size_t approx_num_strings_per_thread = CARBON_MAX(1, num_ids / num_threads);

    CARBON_MALLOC(size_t, local_thread_idx, num_ids, &self->alloc);
    CARBON_MALLOC(uint_fast16_t, owning_thread_ids, num_ids, &self->alloc);
    CARBON_MALLOC(parallel_extract_arg, thread_args, num_threads, &self->alloc);

    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_extract_arg *arg = thread_args + thread_id;
        carbon_vec_create(&arg->local_ids_in, &self->alloc, sizeof(carbon_string_id_t), approx_num_strings_per_thread);
    }

    /** compute subset of string ids per thread  */
    for (size_t i = 0; i < num_ids; i++) {
        carbon_string_id_t global_string_id = ids[i];
        owning_thread_ids[i] = GET_OWNER(global_string_id);
        carbon_string_id_t localcarbon_string_id_t = GET_carbon_string_id_t(global_string_id);
        assert(owning_thread_ids[i] < num_threads);

        parallel_extract_arg *arg = thread_args + owning_thread_ids[i];
        local_thread_idx[i] = carbon_vec_length(&arg->local_ids_in);
        carbon_vec_push(&arg->local_ids_in, &localcarbon_string_id_t, 1);
    }

    /** schedule remove operation per carrier */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        parallel_extract_arg *carrier_arg = thread_args + thread_id;
        carrier_arg->carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallel_extract_function, carrier_arg);
    }

    /** synchronize */
    synchronize(&extra->carriers, num_threads);

    for (size_t i = 0; i < num_ids; i++) {
        uint_fast16_t owning_thread_id = owning_thread_ids[i];
        size_t localIdx = local_thread_idx[i];
        parallel_extract_arg *carrier_arg = thread_args + owning_thread_id;
        carbon_strdic_entry_t extractedString = carrier_arg->entries_out[localIdx];
        globalResult[i] = extractedString;
    }

    /** cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
        parallel_extract_arg *carrier_arg = thread_args + thread_id;
        carbon_vec_drop(&carrier_arg->local_ids_in);
        if (CARBON_LIKELY(carrier_arg->did_work)) {
            carbon_strdic_free(&carrier_arg->carrier->local_dictionary, carrier_arg->entries_out);
        }
    }

    CARBON_FREE(local_thread_idx, &self->alloc);
    CARBON_FREE(owning_thread_ids, &self->alloc);
    CARBON_FREE(thread_args, &self->alloc);

    this_unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return globalResult;
}

static bool this_free(carbon_strdic_t *self, void *ptr)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    carbon_free(&self->alloc, ptr);
    return true;
}

static bool this_num_distinct(carbon_strdic_t *self, size_t *num)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    this_lock(self);

    struct async_extra *extra = THIS_EXTRAS(self);
    size_t num_carriers = carbon_vec_length(&extra->carriers);
    carrier_t *carriers = CARBON_VECTOR_ALL(&extra->carriers, carrier_t);
    size_t num_distinct = 0;
    while (num_carriers--) {
        size_t local_distinct;
        carbon_strdic_num_distinct(&local_distinct, &carriers->local_dictionary);
        num_distinct += local_distinct;
        carriers++;
    }
    *num = num_distinct;
    this_unlock(self);
    return true;
}

static bool this_get_contents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                              carbon_vec_t ofType(carbon_string_id_t) * string_ids,
                              carbon_vec_t ofType(carbon_string_id_t) * grouping_keys)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    this_lock(self);
    struct async_extra *extra = THIS_EXTRAS(self);
    size_t num_carriers = carbon_vec_length(&extra->carriers);
    carbon_vec_t ofType (char *) local_string_results;
    carbon_vec_t ofType (carbon_string_id_t) local_string_id_results;
    carbon_vec_t ofType (carbon_string_id_t) local_grouping_key_results;
    size_t approx_num_distinct_local_values;
    this_num_distinct(self, &approx_num_distinct_local_values);
    approx_num_distinct_local_values = CARBON_MAX(1, approx_num_distinct_local_values / extra->carriers.num_elems);
    approx_num_distinct_local_values *= 1.2f;

    carbon_vec_create(&local_string_results, NULL, sizeof(char *), approx_num_distinct_local_values);
    carbon_vec_create(&local_string_id_results, NULL, sizeof(carbon_string_id_t), approx_num_distinct_local_values);
    carbon_vec_create(&local_grouping_key_results, NULL, sizeof(carbon_string_id_t), approx_num_distinct_local_values);


    for (size_t thread_id = 0; thread_id < num_carriers; thread_id++)
    {
        carbon_vec_clear(&local_string_results);
        carbon_vec_clear(&local_string_id_results);
        carbon_vec_clear(&local_grouping_key_results);

        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);

        carbon_strdic_get_contents(&local_string_results, &local_string_id_results, &local_grouping_key_results, &carrier->local_dictionary);

        assert(local_string_id_results.num_elems == local_string_results.num_elems);
        for (size_t k = 0; k < local_string_results.num_elems; k++) {
            char *string = *CARBON_VECTOR_GET(&local_string_results, k, char *);
            carbon_string_id_t grouping_key = *CARBON_VECTOR_GET(&local_grouping_key_results, k, carbon_string_id_t);
            carbon_string_id_t localcarbon_string_id_t = *CARBON_VECTOR_GET(&local_string_id_results, k, carbon_string_id_t);
            carbon_string_id_t global_string_id = MAKE_GLOBAL(thread_id, localcarbon_string_id_t);
            carbon_vec_push(strings, &string, 1);
            carbon_vec_push(string_ids, &global_string_id, 1);
            carbon_vec_push(grouping_keys, &grouping_key, 1);
        }
    }

    carbon_vec_drop(&local_string_results);
    carbon_vec_drop(&local_string_id_results);
    carbon_vec_drop(&local_grouping_key_results);
    this_unlock(self);
    return true;
}

static bool this_reset_counters(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    this_lock(self);

    struct async_extra *extra = THIS_EXTRAS(self);
    size_t num_threads = carbon_vec_length(&extra->carriers);

    for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        carbon_strdic_reset_counters(&carrier->local_dictionary);
    }

    this_unlock(self);

    return true;
}

static bool this_counters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    this_lock(self);

    struct async_extra *extra = THIS_EXTRAS(self);
    size_t num_threads = carbon_vec_length(&extra->carriers);

    CARBON_CHECK_SUCCESS(carbon_strhash_counters_init(counters));

    for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        carrier_t *carrier = CARBON_VECTOR_GET(&extra->carriers, thread_id, carrier_t);
        carbon_string_hash_counters_t local_counters;
        carbon_strdic_get_counters(&local_counters, &carrier->local_dictionary);
        carbon_strhash_counters_add(counters, &local_counters);
    }

    this_unlock(self);

    return true;
}

typedef struct parallel_create_carrier_arg
{
    size_t local_capacity;
    size_t local_bucket_num;
    size_t local_bucket_cap;
    const carbon_alloc_t *alloc;
} parallel_create_carrier_arg_t;

static void parallel_create_carrier(const void *restrict start, size_t width, size_t len, void *restrict args,
                                  thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_UNUSED(width);

    carrier_t *carrier = (carrier_t *) start;
    const parallel_create_carrier_arg_t *createArgs = (const parallel_create_carrier_arg_t *) args;
    while (len--) {
        carbon_strdic_create_sync(&carrier->local_dictionary, createArgs->local_capacity, createArgs->local_bucket_num,
                                  createArgs->local_bucket_cap, 0, createArgs->alloc);
        memset(&carrier->thread, 0, sizeof(pthread_t));
        carrier++;
    }
}

static bool this_setup_carriers(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                             size_t approx_num_unique_str, size_t num_threads)
{
    async_extra *extra = THIS_EXTRAS(self);
    size_t local_bucket_num = CARBON_MAX(1, num_index_buckets / num_threads);
    carrier_t new_carrier;

    parallel_create_carrier_arg_t createArgs = {
        .local_capacity = CARBON_MAX(1, capacity / num_threads),
        .local_bucket_num = local_bucket_num,
        .local_bucket_cap = CARBON_MAX(1, approx_num_unique_str / num_threads / local_bucket_num / SLICE_KEY_COLUMN_MAX_ELEMS),
        .alloc = &self->alloc
    };

    for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        new_carrier.id = thread_id;
        carbon_vec_push(&extra->carriers, &new_carrier, 1);
    }

    carbon_parallel_for(CARBON_VECTOR_ALL(&extra->carriers, carrier_t), sizeof(carrier_t), num_threads, parallel_create_carrier,
                &createArgs, CARBON_PARALLEL_THREAD_HINT_MULTI, num_threads);

    return true;
}

static bool this_lock(carbon_strdic_t *self)
{
    async_extra *extra = THIS_EXTRAS(self);
    CARBON_CHECK_SUCCESS(carbon_spinlock_acquire(&extra->lock));
    return true;
}

static bool this_unlock(carbon_strdic_t *self)
{
    async_extra *extra = THIS_EXTRAS(self);
    CARBON_CHECK_SUCCESS(carbon_spinlock_release(&extra->lock));
    return true;
}
