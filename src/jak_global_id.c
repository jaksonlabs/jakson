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

#include <jak_global_id.h>
#include <jak_time.h>
#include <jak_error.h>
#include <jak_hash_bern.h>

_Thread_local bool thread_local_init;

_Thread_local jak_u64 thread_local_id;

_Thread_local jak_u8 thread_local_magic;

_Thread_local jak_u32 thread_local_counter;

_Thread_local jak_u32 thread_local_counter_limit;

/**
 * Below is the best effort to create a world-unique, fast and scalable to compute identifier.
 *
 * An 64bit global identifier is constructed using process-local, thread-local and
 * call-local state-dependent and state-independent components. Additionally, a global time
 * component is added to minimize the risk of producing two equal identifier across multiple
 * machines.
 */
union global_id {
    struct {
        /* global */
        jak_u64 global_wallclock
                : 5;   /* increasing wall clock time (ms) */
        jak_u64 global_build_date
                : 1;   /* fix bit dependent on compilation time */
        jak_u64 global_build_path
                : 1;   /* fix bit dependent on compilation path */

        /* per-process */
        jak_u64 process_id
                : 7;   /* fix id */
        jak_u64 process_magic
                : 2;   /* random fix value */
        jak_u64 process_counter
                : 8;   /* increasing counter */

        /* per-thread  */
        jak_u64 thread_id
                : 7;   /* fix id */
        jak_u64 thread_magic
                : 2;   /* random fix value */
        jak_u64 thread_counter
                : 29;  /* increasing counter (< 536mio ids per thread) */

        /* per-call */
        jak_u64 call_random
                :  2;  /* random value */
    };

    jak_u64 value;
};

bool global_id_create(jak_global_id_t *out)
{
        JAK_ASSERT(out);

        static bool process_init;
        static jak_u64 process_local_id;
        static jak_u8 process_magic;
        static jak_u64 process_counter;

        static jak_u8 global_build_date_bit;
        static jak_u8 global_build_path_bit;

        if (!process_init) {
                srand(time(NULL));
                process_magic = rand();
                process_init = true;
                process_counter = rand();

                const char *file = __FILE__;
                const char *time = __TIME__;

                global_build_path_bit = JAK_HASH_BERNSTEIN(strlen(file), file) % 2;
                global_build_date_bit = JAK_HASH_BERNSTEIN(strlen(time), time) % 2;
        }

        if (!thread_local_init) {
                thread_local_counter = rand();
                thread_local_counter_limit = thread_local_counter++;
                thread_local_id = (jak_u64) pthread_self();
                process_local_id = getpid();
                thread_local_magic = rand();
                thread_local_init = true;
        }

        bool capacity_left = (thread_local_counter != thread_local_counter_limit);
        error_print_if(!capacity_left, JAK_ERR_THREADOOOBJIDS)
        if (JAK_LIKELY(capacity_left)) {
                union global_id internal =
                        {.global_wallclock  = time_now_wallclock(),
                         .global_build_date = global_build_date_bit,
                         .global_build_path = global_build_path_bit,
                         .process_id        = process_local_id,
                         .process_magic     = process_magic,
                         .process_counter   = process_counter++,
                         .thread_id         = (jak_u64) thread_local_id,
                         .thread_magic      = thread_local_magic,
                         .thread_counter    = thread_local_counter++,
                         .call_random       = rand()};
                *out = internal.value;
        } else {
                *out = 0;
        }
        return capacity_left;
}

bool global_id_get_global_wallclocktime(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->global_wallclock;
        return true;
}

bool global_id_get_global_build_path_bit(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->global_build_path;
        return true;
}

bool global_id_get_global_build_time_bit(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->global_build_date;
        return true;
}

bool global_id_get_process_id(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->process_id;
        return true;
}

bool global_id_get_process_magic(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->process_magic;
        return true;
}

bool global_id_get_process_counter(uint_fast16_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->process_counter;
        return true;
}

bool global_id_get_thread_id(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->thread_id;
        return true;
}

bool global_id_get_thread_magic(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->thread_magic;
        return true;
}

bool global_id_get_thread_counter(uint_fast32_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->thread_counter;
        return true;
}

bool global_id_get_call_random(uint_fast8_t *out, jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(out);
        *out = ((union global_id *) &id)->call_random;
        return true;
}


