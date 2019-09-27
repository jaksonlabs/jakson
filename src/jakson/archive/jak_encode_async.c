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

#include <jakson/stdx/jak_alloc.h>
#include <jakson/std/jak_vector.h>
#include <jakson/archive/jak_encode_sync.h>
#include <jakson/archive/jak_encode_async.h>
#include <jakson/std/jak_spinlock.h>
#include <jakson/std/jak_str_hash.h>
#include <jakson/utils/jak_time.h>
#include <jakson/std/jak_async.h>
#include <jakson/stdx/jak_slicelist.h>
#include <jakson/std/jak_hash.h>

#define STRING_DIC_ASYNC_TAG "jak_string_dict_async"

#define HASH_FUNCTION                  JAK_HASH_SAX

struct carrier {
        jak_string_dict local_dictionary;
        pthread_t thread;
        size_t id;
};

struct async_extra {
        jak_vector ofType(carrier) carriers;
        jak_vector ofType(struct carrier *) carrier_mapping;
        jak_spinlock lock;
};

struct parallel_insert_arg {
        jak_vector ofType(char *) strings;
        jak_archive_field_sid_t *out;
        struct carrier *carrier;
        bool enable_write_out;
        bool did_work;
        uint_fast16_t insert_num_threads;
};

struct parallel_remove_arg {
        jak_vector ofType(jak_archive_field_sid_t) *local_ids;
        struct carrier *carrier;
        int result;
        bool did_work;
};

struct parallel_locate_arg {
        struct carrier *carrier;
        jak_archive_field_sid_t *ids_out;
        bool *found_mask_out;
        size_t num_not_found_out;
        jak_vector ofType(char *) keys_in;
        int result;
        bool did_work;
};

struct parallel_extract_arg {
        jak_vector ofType(jak_archive_field_sid_t) local_ids_in;
        char **strings_out;
        struct carrier *carrier;
        bool did_work;
};

#define _JAK_ENCODE_ASYNC_HASHCODE_OF(string)                                                                                            \
    HASH_FUNCTION(strlen(string), string)

#define MAKE_GLOBAL(thread_id, localjak_string_id_t)                                                                \
    ((thread_id << 54) | localjak_string_id_t)

#define GET_OWNER(globalId)                                                                                            \
    (globalId >> 54)

#define GET_jak_string_id_t(globalId)                                                                               \
    ((~((jak_archive_field_sid_t) 0)) >> 10 & global_jak_string_id);

static bool _jak_encode_async_drop(jak_string_dict *self);

static bool
_jak_encode_async_insert(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t num_strings,
            size_t __num_threads);

static bool _jak_encode_async_remove(jak_string_dict *self, jak_archive_field_sid_t *strings, size_t num_strings);

static bool
_jak_encode_async_locate_safe(jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                 char *const *keys, size_t num_keys);

static bool
_jak_encode_async_locate_fast(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys);

static char **_jak_encode_async_locate_extract(jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids);

static bool _jak_encode_async_free(jak_string_dict *self, void *ptr);

static bool _jak_encode_async_num_distinct(jak_string_dict *self, size_t *num);

static bool _jak_encode_async_get_contents(jak_string_dict *self, jak_vector ofType (char *) *strings,
                              jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids);

static bool _jak_encode_async_reset_counters(jak_string_dict *self);

static bool _jak_encode_async_counters(jak_string_dict *self, jak_str_hash_counters *counters);

static bool this_lock(jak_string_dict *self);

static bool this_unlock(jak_string_dict *self);

static bool _jak_encode_async_create_extra(jak_string_dict *self, size_t capacity, size_t num_index_buckets,
                              size_t approx_num_unique_str, size_t num_threads);

static bool this_setup_carriers(jak_string_dict *self, size_t capacity, size_t num_index_buckets,
                                size_t approx_num_unique_str, size_t num_threads);

#define THIS_EXTRAS(self)                                                                                              \
({                                                                                                                     \
    JAK_CHECK_TAG(self->tag, JAK_ASYNC);                                                             \
    (struct async_extra *) self->extra;                                                                                       \
})

int jak_encode_async_create(jak_string_dict *dic, size_t capacity, size_t num_index_buckets,
                        size_t approx_num_unique_strs, size_t num_threads, const jak_allocator *alloc)
{
        JAK_CHECK_SUCCESS(jak_alloc_this_or_std(&dic->alloc, alloc));

        dic->tag = JAK_ASYNC;
        dic->drop = _jak_encode_async_drop;
        dic->insert = _jak_encode_async_insert;
        dic->remove = _jak_encode_async_remove;
        dic->locate_safe = _jak_encode_async_locate_safe;
        dic->locate_fast = _jak_encode_async_locate_fast;
        dic->extract = _jak_encode_async_locate_extract;
        dic->free = _jak_encode_async_free;
        dic->resetCounters = _jak_encode_async_reset_counters;
        dic->counters = _jak_encode_async_counters;
        dic->num_distinct = _jak_encode_async_num_distinct;
        dic->get_contents = _jak_encode_async_get_contents;

        JAK_CHECK_SUCCESS(_jak_encode_async_create_extra(dic, capacity, num_index_buckets, approx_num_unique_strs, num_threads));
        return true;
}

static bool _jak_encode_async_create_extra(jak_string_dict *self, size_t capacity, size_t num_index_buckets,
                              size_t approx_num_unique_str, size_t num_threads)
{
        JAK_ASSERT(self);

        self->extra = jak_alloc_malloc(&self->alloc, sizeof(struct async_extra));
        struct async_extra *extra = THIS_EXTRAS(self);
        jak_spinlock_init(&extra->lock);
        jak_vector_create(&extra->carriers, &self->alloc, sizeof(struct carrier), num_threads);
        this_setup_carriers(self, capacity, num_index_buckets, approx_num_unique_str, num_threads);
        jak_vector_create(&extra->carrier_mapping, &self->alloc, sizeof(struct carrier *), capacity);

        return true;
}

static bool _jak_encode_async_drop(jak_string_dict *self)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);
        struct async_extra *extra = THIS_EXTRAS(self);
        for (size_t i = 0; i < extra->carriers.num_elems; i++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, i, struct carrier);
                jak_string_dict_drop(&carrier->local_dictionary);
        }
        JAK_CHECK_SUCCESS(jak_vector_drop(&extra->carriers));
        JAK_CHECK_SUCCESS(jak_vector_drop(&extra->carrier_mapping));
        JAK_CHECK_SUCCESS(jak_alloc_free(&self->alloc, extra));
        return true;
}

void *parallel_remove_function(void *args)
{
        struct parallel_remove_arg *carrier_arg = (struct parallel_remove_arg *) args;
        jak_archive_field_sid_t len = jak_vector_length(carrier_arg->local_ids);
        carrier_arg->did_work = len > 0;

        JAK_DEBUG(STRING_DIC_ASYNC_TAG,
                  "thread %zu spawned for remove task (%zu elements)",
                  carrier_arg->carrier->id,
                  jak_vector_length(carrier_arg->local_ids));
        if (len > 0) {
                jak_string_dict *dic = &carrier_arg->carrier->local_dictionary;
                jak_archive_field_sid_t *ids = JAK_VECTOR_ALL(carrier_arg->local_ids, jak_archive_field_sid_t);
                carrier_arg->result = jak_string_dict_remove(dic, ids, len);
                JAK_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu task done", carrier_arg->carrier->id);
        } else {
                carrier_arg->result = true;
                JAK_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", carrier_arg->carrier->id);
        }

        return NULL;
}

void *parallel_insert_function(void *args)
{
        struct parallel_insert_arg *restrict this_args = (struct parallel_insert_arg *restrict) args;
        this_args->did_work = this_args->strings.num_elems > 0;

        JAK_TRACE(STRING_DIC_ASYNC_TAG, "thread-local insert function started (thread %zu)", this_args->carrier->id);
        JAK_DEBUG(STRING_DIC_ASYNC_TAG,
                  "thread %zu spawned for insert task (%zu elements)",
                  this_args->carrier->id,
                  jak_vector_length(&this_args->strings));

        if (this_args->did_work) {
                JAK_TRACE(STRING_DIC_ASYNC_TAG,
                          "thread %zu starts insertion of %zu strings",
                          this_args->carrier->id,
                          jak_vector_length(&this_args->strings));
                char **data = (char **) jak_vector_data(&this_args->strings);

                int status = jak_string_dict_insert(&this_args->carrier->local_dictionary,
                                           this_args->enable_write_out ? &this_args->out : NULL,
                                           data,
                                           jak_vector_length(&this_args->strings),
                                           this_args->insert_num_threads);

                /** internal JAK_ERROR during thread-local string dictionary building process */
                JAK_ERROR_PRINT_AND_DIE_IF(status != true, JAK_ERR_INTERNALERR);
                JAK_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
        } else {
                JAK_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
        }

        return NULL;
}

void *parallel_locate_safe_function(void *args)
{
        struct parallel_locate_arg *restrict this_args = (struct parallel_locate_arg *restrict) args;
        this_args->did_work = jak_vector_length(&this_args->keys_in) > 0;

        JAK_TRACE(STRING_DIC_ASYNC_TAG,
                  "thread-local 'locate' function invoked for thread %zu...",
                  this_args->carrier->id)

        JAK_DEBUG(STRING_DIC_ASYNC_TAG,
                  "thread %zu spawned for locate (safe) task (%zu elements)",
                  this_args->carrier->id,
                  jak_vector_length(&this_args->keys_in));

        if (this_args->did_work) {
                this_args->result = jak_string_dict_locate_safe(&this_args->ids_out,
                                                       &this_args->found_mask_out,
                                                       &this_args->num_not_found_out,
                                                       &this_args->carrier->local_dictionary,
                                                       JAK_VECTOR_ALL(&this_args->keys_in, char *),
                                                       jak_vector_length(&this_args->keys_in));

                JAK_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
        } else {
                JAK_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
        }

        return NULL;
}

void *parallel_extract_function(void *args)
{
        struct parallel_extract_arg *restrict this_args = (struct parallel_extract_arg *restrict) args;
        this_args->did_work = jak_vector_length(&this_args->local_ids_in) > 0;

        JAK_DEBUG(STRING_DIC_ASYNC_TAG,
                  "thread %zu spawned for extract task (%zu elements)",
                  this_args->carrier->id,
                  jak_vector_length(&this_args->local_ids_in));

        if (this_args->did_work) {
                this_args->strings_out = jak_string_dict_extract(&this_args->carrier->local_dictionary,
                                                        JAK_VECTOR_ALL(&this_args->local_ids_in, jak_archive_field_sid_t),
                                                        jak_vector_length(&this_args->local_ids_in));
                JAK_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
        } else {
                JAK_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
        }

        return NULL;
}

static void synchronize(jak_vector ofType(carrier) *carriers, size_t num_threads)
{
        JAK_DEBUG(STRING_DIC_ASYNC_TAG, "barrier installed for %d threads", num_threads);

        jak_timestamp begin = jak_wallclock();
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                volatile struct carrier *carrier = JAK_VECTOR_GET(carriers, thread_id, struct carrier);
                pthread_join(carrier->thread, NULL);
                JAK_DEBUG(STRING_DIC_ASYNC_TAG, "thread %d joined", carrier->id);
        }
        jak_timestamp end = jak_wallclock();
        jak_timestamp duration = (end - begin);
        JAK_UNUSED(duration);

        JAK_DEBUG(STRING_DIC_ASYNC_TAG,
                  "barrier passed for %d threads after %f seconds",
                  num_threads,
                  duration / 1000.0f);
}

static void create_thread_assignment(atomic_uint_fast16_t **str_carrier_mapping, atomic_size_t **carrier_num_strings,
                                     size_t **str_carrier_idx_mapping, jak_allocator *alloc, size_t num_strings,
                                     size_t num_threads)
{
        /** jak_async_map_exec string depending on hash values to a particular carrier */
        *str_carrier_mapping = jak_alloc_malloc(alloc, num_strings * sizeof(atomic_uint_fast16_t));
        memset(*str_carrier_mapping, 0, num_strings * sizeof(atomic_uint_fast16_t));

        /** counters to compute how many strings go to a particular carrier */
        *carrier_num_strings = jak_alloc_malloc(alloc, num_threads * sizeof(atomic_size_t));
        memset(*carrier_num_strings, 0, num_threads * sizeof(atomic_size_t));

        /** an inverted index that contains the i-th position for string k that was assigned to carrier m.
         * With this, given a (global) string and and its carrier, one can have directly the position of the
         * string in the carriers "thread-local locate" args */
        *str_carrier_idx_mapping = jak_alloc_malloc(alloc, num_strings * sizeof(size_t));
}

static void drop_thread_assignment(jak_allocator *alloc, atomic_uint_fast16_t *str_carrier_mapping,
                                   atomic_size_t *carrier_num_strings, size_t *str_carrier_idx_mapping)
{
        jak_alloc_free(alloc, carrier_num_strings);
        jak_alloc_free(alloc, str_carrier_mapping);
        jak_alloc_free(alloc, str_carrier_idx_mapping);
}

struct thread_assign_arg {
        atomic_uint_fast16_t *str_carrier_mapping;
        size_t num_threads;
        atomic_size_t *carrier_num_strings;
        char *const *base_strings;
};

static void parallel_compute_thread_assignment_function(const void *restrict start, size_t width, size_t len,
                                                        void *restrict args, jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_UNUSED(width);

        char *const *strings = (char *const *) start;

        struct thread_assign_arg *func_args = (struct thread_assign_arg *) args;

        while (len--) {
                size_t i = strings - func_args->base_strings;
                const char *key = *strings;
                /** re-using this hashcode for the thread-local dictionary is more costly than to compute it fresh
                 * (due to more I/O with the RAM) */
                size_t thread_id = _JAK_ENCODE_ASYNC_HASHCODE_OF(key) % func_args->num_threads;
                atomic_fetch_add(&func_args->str_carrier_mapping[i], thread_id);
                atomic_fetch_add(&func_args->carrier_num_strings[thread_id], 1);
                strings++;
        }
}

static void compute_thread_assignment(atomic_uint_fast16_t *str_carrier_mapping, atomic_size_t *carrier_num_strings,
                                      char *const *strings, size_t num_strings, size_t num_threads)
{
        struct thread_assign_arg args =
                {.base_strings = strings, .carrier_num_strings = carrier_num_strings, .num_threads = num_threads, .str_carrier_mapping = str_carrier_mapping};
        jak_for(strings,
                sizeof(char *const *),
                num_strings,
                parallel_compute_thread_assignment_function,
                &args,
                JAK_THREADING_HINT_MULTI,
                num_threads);

}

static bool
_jak_encode_async_insert(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t num_strings,
            size_t __num_threads)
{
        jak_timestamp begin = jak_wallclock();
        JAK_INFO(STRING_DIC_ASYNC_TAG, "insert operation invoked: %zu strings in total", num_strings)

        JAK_CHECK_TAG(self->tag, JAK_ASYNC);
        /** parameter 'num_threads' must be set to 0 for async dictionary */
        JAK_ERROR_PRINT_AND_DIE_IF(__num_threads != 0, JAK_ERR_INTERNALERR);

        this_lock(self);

        struct async_extra *extra = THIS_EXTRAS(self);
        uint_fast16_t num_threads = jak_vector_length(&extra->carriers);

        atomic_uint_fast16_t *str_carrier_mapping;
        size_t *str_carrier_idx_mapping;
        atomic_size_t *carrier_num_strings;

        create_thread_assignment(&str_carrier_mapping,
                                 &carrier_num_strings,
                                 &str_carrier_idx_mapping,
                                 &self->alloc,
                                 num_strings,
                                 num_threads);

        jak_vector ofType(struct parallel_insert_arg *) carrier_args;
        jak_vector_create(&carrier_args, &self->alloc, sizeof(struct parallel_insert_arg *), num_threads);

        /** compute which carrier is responsible for which string */
        compute_thread_assignment(str_carrier_mapping, carrier_num_strings, strings, num_strings, num_threads);

        /** prepare to move string subsets to carriers */
        for (uint_fast16_t i = 0; i < num_threads; i++) {
                struct parallel_insert_arg *entry = jak_alloc_malloc(&self->alloc, sizeof(struct parallel_insert_arg));
                entry->carrier = JAK_VECTOR_GET(&extra->carriers, i, struct carrier);
                entry->insert_num_threads = num_threads;

                jak_vector_create(&entry->strings, &self->alloc, sizeof(char *), JAK_MAX(1, carrier_num_strings[i]));
                jak_vector_push(&carrier_args, &entry, 1);
                JAK_ASSERT (entry->strings.base != NULL);

                struct parallel_insert_arg *carrier_arg = *JAK_VECTOR_GET(&carrier_args, i, struct parallel_insert_arg *);
                carrier_arg->out = NULL;
        }

        /** create per-carrier string subset */
        /** parallizing this makes no sense but waste of resources and energy */
        for (size_t i = 0; i < num_strings; i++) {
                uint_fast16_t thread_id = str_carrier_mapping[i];
                struct parallel_insert_arg
                        *carrier_arg = *JAK_VECTOR_GET(&carrier_args, thread_id, struct parallel_insert_arg *);
                carrier_arg->enable_write_out = out != NULL;

                /** store local index of string i inside the thread */
                str_carrier_idx_mapping[i] = jak_vector_length(&carrier_arg->strings);

                jak_vector_push(&carrier_arg->strings, &strings[i], 1);
        }


        /** schedule insert operation per carrier */
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "schedule insert operation to %zu threads", num_threads)
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_insert_arg
                        *carrier_arg = *JAK_VECTOR_GET(&carrier_args, thread_id, struct parallel_insert_arg *);
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                JAK_TRACE(STRING_DIC_ASYNC_TAG, "create thread %zu...", thread_id)
                pthread_create(&carrier->thread, NULL, parallel_insert_function, carrier_arg);
                JAK_TRACE(STRING_DIC_ASYNC_TAG, "thread %zu created", thread_id)
        }
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "scheduling done for %zu threads", num_threads)

        /** synchronize */
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "start synchronizing %zu threads", num_threads)
        synchronize(&extra->carriers, num_threads);
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync", num_threads)

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
        if (JAK_LIKELY(out != NULL)) {
                jak_archive_field_sid_t *total_out = jak_alloc_malloc(&self->alloc,
                                                                      num_strings * sizeof(jak_archive_field_sid_t));
                size_t currentOut = 0;

                for (size_t jak_string_idx = 0; jak_string_idx < num_strings; jak_string_idx++) {
                        uint_fast16_t thread_id = str_carrier_mapping[jak_string_idx];
                        size_t localIdx = str_carrier_idx_mapping[jak_string_idx];
                        struct parallel_insert_arg
                                *carrier_arg = *JAK_VECTOR_GET(&carrier_args, thread_id, struct parallel_insert_arg *);
                        jak_archive_field_sid_t global_jak_string_owner_id = thread_id;
                        jak_archive_field_sid_t global_jak_string_local_id = carrier_arg->out[localIdx];
                        jak_archive_field_sid_t global_jak_string_id = MAKE_GLOBAL(global_jak_string_owner_id,
                                                                               global_jak_string_local_id);
                        total_out[currentOut++] = global_jak_string_id;
                }

                *out = total_out;
        }

        /** cleanup */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_insert_arg
                        *carrier_arg = *JAK_VECTOR_GET(&carrier_args, thread_id, struct parallel_insert_arg *);
                if (carrier_arg->did_work) {
                        jak_string_dict_free(&carrier_arg->carrier->local_dictionary, carrier_arg->out);
                }
                jak_vector_drop(&carrier_arg->strings);
                jak_alloc_free(&self->alloc, carrier_arg);
        }

        /** cleanup */
        drop_thread_assignment(&self->alloc, str_carrier_mapping, carrier_num_strings, str_carrier_idx_mapping);
        jak_vector_drop(&carrier_args);

        this_unlock(self);

        jak_timestamp end = jak_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_INFO(STRING_DIC_ASYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin) / 1000.0f)

        return true;
}

static bool _jak_encode_async_remove(jak_string_dict *self, jak_archive_field_sid_t *strings, size_t num_strings)
{
        jak_timestamp begin = jak_wallclock();
        JAK_INFO(STRING_DIC_ASYNC_TAG, "remove operation started: %zu strings to remove", num_strings);

        JAK_CHECK_TAG(self->tag, JAK_ASYNC);

        this_lock(self);

        struct parallel_remove_arg empty;
        struct async_extra *extra = THIS_EXTRAS(self);
        uint_fast16_t num_threads = jak_vector_length(&extra->carriers);
        size_t approx_num_strings_per_thread = JAK_MAX(1, num_strings / num_threads);
        jak_vector ofType(jak_archive_field_sid_t) *jak_string_map = jak_alloc_malloc(&self->alloc, num_threads *
                                                                                                       sizeof(jak_vector));

        jak_vector ofType(struct parallel_remove_arg) carrier_args;
        jak_vector_create(&carrier_args, &self->alloc, sizeof(struct parallel_remove_arg), num_threads);

        /** prepare thread-local subset of string ids */
        jak_vector_repeated_push(&carrier_args, &empty, num_threads);
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                jak_vector_create(jak_string_map + thread_id, &self->alloc, sizeof(jak_archive_field_sid_t),
                           approx_num_strings_per_thread);
        }

        /** compute subset of string ids per thread  */
        for (size_t i = 0; i < num_strings; i++) {
                jak_archive_field_sid_t global_jak_string_id = strings[i];
                uint_fast16_t owning_thread_id = GET_OWNER(global_jak_string_id);
                jak_archive_field_sid_t localjak_string_id_t = GET_jak_string_id_t(global_jak_string_id);
                JAK_ASSERT(owning_thread_id < num_threads);

                jak_vector_push(jak_string_map + owning_thread_id, &localjak_string_id_t, 1);
        }

        /** schedule remove operation per carrier */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                struct parallel_remove_arg *carrier_arg = JAK_VECTOR_GET(&carrier_args, thread_id, struct parallel_remove_arg);
                carrier_arg->carrier = carrier;
                carrier_arg->local_ids = jak_string_map + thread_id;

                pthread_create(&carrier->thread, NULL, parallel_remove_function, carrier_arg);
        }

        /** synchronize */
        synchronize(&extra->carriers, num_threads);

        /** cleanup */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                jak_vector_drop(jak_string_map + thread_id);
        }

        jak_alloc_free(&self->alloc, jak_string_map);
        jak_vector_data(&carrier_args);

        this_unlock(self);

        jak_timestamp end = jak_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_INFO(STRING_DIC_ASYNC_TAG, "remove operation done: %f seconds spent here", (end - begin) / 1000.0f)

        return true;
}

static bool
_jak_encode_async_locate_safe(jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                 char *const *keys, size_t num_keys)
{
        jak_timestamp begin = jak_wallclock();
        JAK_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation started: %zu strings to locate", num_keys)

        JAK_CHECK_TAG(self->tag, JAK_ASYNC);

        this_lock(self);

        struct async_extra *extra = THIS_EXTRAS(self);
        uint_fast16_t num_threads = jak_vector_length(&extra->carriers);

        /** global result output */
        JAK_ALLOC_MALLOC(jak_archive_field_sid_t, global_out, num_keys, &self->alloc);
        JAK_ALLOC_MALLOC(bool, global_found_mask, num_keys, &self->alloc);

        size_t global_num_not_found = 0;

        atomic_uint_fast16_t *str_carrier_mapping;
        size_t *str_carrier_idx_mapping;
        atomic_size_t *carrier_num_strings;

        struct parallel_locate_arg carrier_args[num_threads];

        create_thread_assignment(&str_carrier_mapping,
                                 &carrier_num_strings,
                                 &str_carrier_idx_mapping,
                                 &self->alloc,
                                 num_keys,
                                 num_threads);

        /** compute which carrier is responsible for which string */
        compute_thread_assignment(str_carrier_mapping, carrier_num_strings, keys, num_keys, num_threads);

        /** prepare to move string subsets to carriers */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_locate_arg *arg = carrier_args + thread_id;
                jak_vector_create(&arg->keys_in, &self->alloc, sizeof(char *), carrier_num_strings[thread_id]);
                JAK_ASSERT (&arg->keys_in.base != NULL);
        }

        JAK_TRACE(STRING_DIC_ASYNC_TAG, "computing per-thread string subset for %zu strings", num_keys)
        /** create per-carrier string subset */
        for (size_t i = 0; i < num_keys; i++) {
                /** get thread responsible for this particular string */
                uint_fast16_t thread_id = str_carrier_mapping[i];

                /** get the thread-local argument list for the thread that is responsible for this particular string */
                struct parallel_locate_arg *arg = carrier_args + thread_id;

                /** store local index of string i inside the thread */
                str_carrier_idx_mapping[i] = jak_vector_length(&arg->keys_in);

                /** push that string into the thread-local vector */
                jak_vector_push(&arg->keys_in, &keys[i], 1);
        }

        JAK_TRACE(STRING_DIC_ASYNC_TAG, "schedule operation to threads to %zu threads...", num_threads)
        /** schedule operation to threads */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                struct parallel_locate_arg *arg = carrier_args + thread_id;
                carrier_args[thread_id].carrier = carrier;
                pthread_create(&carrier->thread, NULL, parallel_locate_safe_function, arg);
        }

        /** synchronize */
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "start syncing %zu threads...", num_threads)
        synchronize(&extra->carriers, num_threads);
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync.", num_threads)

        /** collect and merge results */
        JAK_TRACE(STRING_DIC_ASYNC_TAG, "merging results of %zu threads", num_threads)
        for (size_t i = 0; i < num_keys; i++) {
                /** get thread responsible for this particular string, and local position of that string inside the
                 * thread storage */
                uint_fast16_t thread_id = str_carrier_mapping[i];
                size_t local_thread_idx = str_carrier_idx_mapping[i];

                /** get the thread-local argument list for the thread that is responsible for this particular string */
                struct parallel_locate_arg *arg = carrier_args + thread_id;

                /** merge into global result */
                jak_archive_field_sid_t jak_string_id_owner = thread_id;
                jak_archive_field_sid_t jak_string_id_local_idx = arg->ids_out[local_thread_idx];
                jak_archive_field_sid_t global_jak_string_id = MAKE_GLOBAL(jak_string_id_owner, jak_string_id_local_idx);
                global_out[i] = global_jak_string_id;
                global_found_mask[i] = arg->found_mask_out[local_thread_idx];
        }
        for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
                /** compute total number of not-found elements */
                struct parallel_locate_arg *arg = carrier_args + thread_id;
                global_num_not_found += arg->num_not_found_out;

                /** cleanup */
                if (JAK_LIKELY(arg->did_work)) {
                        jak_string_dict_free(&arg->carrier->local_dictionary, arg->found_mask_out);
                        jak_string_dict_free(&arg->carrier->local_dictionary, arg->ids_out);
                }
        }

        JAK_TRACE(STRING_DIC_ASYNC_TAG, "cleanup%s", "...")

        /** cleanup */
        drop_thread_assignment(&self->alloc, str_carrier_mapping, carrier_num_strings, str_carrier_idx_mapping);

        for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_locate_arg *arg = carrier_args + thread_id;
                jak_vector_drop(&arg->keys_in);
        }

        /** return results */
        *out = global_out;
        *found_mask = global_found_mask;
        *num_not_found = global_num_not_found;

        this_unlock(self);

        jak_timestamp end = jak_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

        return true;
}

static bool
_jak_encode_async_locate_fast(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);

        this_lock(self);

        bool *found_mask;
        size_t num_not_found;
        int result;

        /** use safer but in principle more slower implementation */
        result = _jak_encode_async_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

        /** cleanup */
        _jak_encode_async_free(self, found_mask);

        this_unlock(self);

        return result;
}

static char **_jak_encode_async_locate_extract(jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids)
{
        jak_timestamp begin = jak_wallclock();
        JAK_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation started: %zu strings to extract", num_ids)

        if (self->tag != JAK_ASYNC) {
                return NULL;
        }

        this_lock(self);

        JAK_ALLOC_MALLOC(char *, globalResult, num_ids, &self->alloc);

        struct async_extra *extra = (struct async_extra *) self->extra;
        uint_fast16_t num_threads = jak_vector_length(&extra->carriers);
        size_t approx_num_strings_per_thread = JAK_MAX(1, num_ids / num_threads);

        JAK_ALLOC_MALLOC(size_t, local_thread_idx, num_ids, &self->alloc);
        JAK_ALLOC_MALLOC(uint_fast16_t, owning_thread_ids, num_ids, &self->alloc);
        JAK_ALLOC_MALLOC(struct parallel_extract_arg, thread_args, num_threads, &self->alloc);

        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_extract_arg *arg = thread_args + thread_id;
                jak_vector_create(&arg->local_ids_in, &self->alloc, sizeof(jak_archive_field_sid_t),
                           approx_num_strings_per_thread);
        }

        /** compute subset of string ids per thread  */
        for (size_t i = 0; i < num_ids; i++) {
                jak_archive_field_sid_t global_jak_string_id = ids[i];
                owning_thread_ids[i] = GET_OWNER(global_jak_string_id);
                jak_archive_field_sid_t localjak_string_id_t = GET_jak_string_id_t(global_jak_string_id);
                JAK_ASSERT(owning_thread_ids[i] < num_threads);

                struct parallel_extract_arg *arg = thread_args + owning_thread_ids[i];
                local_thread_idx[i] = jak_vector_length(&arg->local_ids_in);
                jak_vector_push(&arg->local_ids_in, &localjak_string_id_t, 1);
        }

        /** schedule remove operation per carrier */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                struct parallel_extract_arg *carrier_arg = thread_args + thread_id;
                carrier_arg->carrier = carrier;
                pthread_create(&carrier->thread, NULL, parallel_extract_function, carrier_arg);
        }

        /** synchronize */
        synchronize(&extra->carriers, num_threads);

        for (size_t i = 0; i < num_ids; i++) {
                uint_fast16_t owning_thread_id = owning_thread_ids[i];
                size_t localIdx = local_thread_idx[i];
                struct parallel_extract_arg *carrier_arg = thread_args + owning_thread_id;
                char *extractedString = carrier_arg->strings_out[localIdx];
                globalResult[i] = extractedString;
        }

        /** cleanup */
        for (uint_fast16_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct parallel_extract_arg *carrier_arg = thread_args + thread_id;
                jak_vector_drop(&carrier_arg->local_ids_in);
                if (JAK_LIKELY(carrier_arg->did_work)) {
                        jak_string_dict_free(&carrier_arg->carrier->local_dictionary, carrier_arg->strings_out);
                }
        }

        JAK_ALLOC_FREE(local_thread_idx, &self->alloc);
        JAK_ALLOC_FREE(owning_thread_ids, &self->alloc);
        JAK_ALLOC_FREE(thread_args, &self->alloc);

        this_unlock(self);

        jak_timestamp end = jak_wallclock();
        JAK_UNUSED(begin);
        JAK_UNUSED(end);
        JAK_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

        return globalResult;
}

static bool _jak_encode_async_free(jak_string_dict *self, void *ptr)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);
        jak_alloc_free(&self->alloc, ptr);
        return true;
}

static bool _jak_encode_async_num_distinct(jak_string_dict *self, size_t *num)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);
        this_lock(self);

        struct async_extra *extra = THIS_EXTRAS(self);
        size_t num_carriers = jak_vector_length(&extra->carriers);
        struct carrier *carriers = JAK_VECTOR_ALL(&extra->carriers, struct carrier);
        size_t num_distinct = 0;
        while (num_carriers--) {
                size_t local_distinct;
                jak_string_dict_num_distinct(&local_distinct, &carriers->local_dictionary);
                num_distinct += local_distinct;
                carriers++;
        }
        *num = num_distinct;
        this_unlock(self);
        return true;
}

static bool _jak_encode_async_get_contents(jak_string_dict *self, jak_vector ofType (char *) *strings,
                              jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);
        this_lock(self);
        struct async_extra *extra = THIS_EXTRAS(self);
        size_t num_carriers = jak_vector_length(&extra->carriers);
        jak_vector ofType (char *) local_jak_string_results;
        jak_vector ofType (jak_archive_field_sid_t) local_jak_string_id_results;
        size_t approx_num_distinct_local_values;
        _jak_encode_async_num_distinct(self, &approx_num_distinct_local_values);
        approx_num_distinct_local_values = JAK_MAX(1, approx_num_distinct_local_values / extra->carriers.num_elems);
        approx_num_distinct_local_values *= 1.2f;

        jak_vector_create(&local_jak_string_results, NULL, sizeof(char *), approx_num_distinct_local_values);
        jak_vector_create(&local_jak_string_id_results, NULL, sizeof(jak_archive_field_sid_t), approx_num_distinct_local_values);

        for (size_t thread_id = 0; thread_id < num_carriers; thread_id++) {
                jak_vector_clear(&local_jak_string_results);
                jak_vector_clear(&local_jak_string_id_results);

                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);

                jak_string_dict_get_contents(&local_jak_string_results, &local_jak_string_id_results, &carrier->local_dictionary);

                JAK_ASSERT(local_jak_string_id_results.num_elems == local_jak_string_results.num_elems);
                for (size_t k = 0; k < local_jak_string_results.num_elems; k++) {
                        char *string = *JAK_VECTOR_GET(&local_jak_string_results, k, char *);
                        jak_archive_field_sid_t localjak_string_id_t = *JAK_VECTOR_GET(&local_jak_string_id_results, k,
                                                                            jak_archive_field_sid_t);
                        jak_archive_field_sid_t global_jak_string_id = MAKE_GLOBAL(thread_id, localjak_string_id_t);
                        jak_vector_push(strings, &string, 1);
                        jak_vector_push(jak_string_ids, &global_jak_string_id, 1);
                }
        }

        jak_vector_drop(&local_jak_string_results);
        jak_vector_drop(&local_jak_string_id_results);
        this_unlock(self);
        return true;
}

static bool _jak_encode_async_reset_counters(jak_string_dict *self)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);

        this_lock(self);

        struct async_extra *extra = THIS_EXTRAS(self);
        size_t num_threads = jak_vector_length(&extra->carriers);

        for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                jak_string_dict_reset_counters(&carrier->local_dictionary);
        }

        this_unlock(self);

        return true;
}

static bool _jak_encode_async_counters(jak_string_dict *self, jak_str_hash_counters *counters)
{
        JAK_CHECK_TAG(self->tag, JAK_ASYNC);

        this_lock(self);

        struct async_extra *extra = THIS_EXTRAS(self);
        size_t num_threads = jak_vector_length(&extra->carriers);

        JAK_CHECK_SUCCESS(jak_str_hash_counters_init(counters));

        for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
                struct carrier *carrier = JAK_VECTOR_GET(&extra->carriers, thread_id, struct carrier);
                jak_str_hash_counters local_counters;
                jak_string_dict_get_counters(&local_counters, &carrier->local_dictionary);
                jak_str_hash_counters_add(counters, &local_counters);
        }

        this_unlock(self);

        return true;
}

struct create_carrier_arg {
        size_t local_capacity;
        size_t local_bucket_num;
        size_t local_bucket_cap;
        const jak_allocator *alloc;
};

static void parallel_create_carrier(const void *restrict start, size_t width, size_t len, void *restrict args,
                                    jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_UNUSED(width);

        struct carrier *carrier = (struct carrier *) start;
        const struct create_carrier_arg *createArgs = (const struct create_carrier_arg *) args;
        while (len--) {
                jak_encode_sync_create(&carrier->local_dictionary,
                                   createArgs->local_capacity,
                                   createArgs->local_bucket_num,
                                   createArgs->local_bucket_cap,
                                   0,
                                   createArgs->alloc);
                memset(&carrier->thread, 0, sizeof(pthread_t));
                carrier++;
        }
}

static bool this_setup_carriers(jak_string_dict *self, size_t capacity, size_t num_index_buckets,
                                size_t approx_num_unique_str, size_t num_threads)
{
        struct async_extra *extra = THIS_EXTRAS(self);
        size_t local_bucket_num = JAK_MAX(1, num_index_buckets / num_threads);
        struct carrier new_carrier;

        struct create_carrier_arg createArgs = {.local_capacity = JAK_MAX(1,
                                                                          capacity /
                                                                          num_threads), .local_bucket_num = local_bucket_num, .local_bucket_cap = JAK_MAX(
                1,
                approx_num_unique_str / num_threads / local_bucket_num / SLICE_KEY_COLUMN_MAX_ELEMS), .alloc = &self
                ->alloc};

        for (size_t thread_id = 0; thread_id < num_threads; thread_id++) {
                new_carrier.id = thread_id;
                jak_vector_push(&extra->carriers, &new_carrier, 1);
        }

        jak_for(JAK_VECTOR_ALL(&extra->carriers, struct carrier),
                sizeof(struct carrier),
                num_threads,
                parallel_create_carrier,
                &createArgs,
                JAK_THREADING_HINT_MULTI,
                num_threads);

        return true;
}

static bool this_lock(jak_string_dict *self)
{
        struct async_extra *extra = THIS_EXTRAS(self);
        JAK_CHECK_SUCCESS(jak_spinlock_acquire(&extra->lock));
        return true;
}

static bool this_unlock(jak_string_dict *self)
{
        struct async_extra *extra = THIS_EXTRAS(self);
        JAK_CHECK_SUCCESS(jak_spinlock_release(&extra->lock));
        return true;
}
