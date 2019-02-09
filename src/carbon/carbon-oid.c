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

#include <carbon/carbon-hash.h>
#include <carbon/carbon-oid.h>
#include <carbon/carbon-time.h>
#include <carbon/carbon-error.h>

_Thread_local bool     thread_local_init;
_Thread_local uint64_t thread_local_id;
_Thread_local uint8_t  thread_local_magic;
_Thread_local uint32_t thread_local_counter;

/**
 * Object identifier that acts as the primary key for any object stored in a CARBON record.
 * Below is the best effort to create a world-unique, fast and scalable to compute identifier.
 *
 * An 64bit object identifier is constructed using process-local, thread-local and
 * call-local state-dependent and state-independent components. Additionally, a global time
 * component is added to minimize the risk of producing two equal identifier across multiple
 * machines.
 */
typedef union
{
    struct {
        /* global */
        uint64_t global_wallclock  : 5;   /* increasing wall clock time (ms) */
        uint64_t global_build_date : 1;   /* fix bit dependent on compilation time */
        uint64_t global_build_path : 1;   /* fix bit dependent on compilation path */

        /* per-process */
        uint64_t process_id        : 7;   /* fix id */
        uint64_t process_magic     : 2;   /* random fix value */
        uint64_t process_counter   : 8;   /* increasing counter */

        /* per-thread  */
        uint64_t thread_id         : 7;   /* fix id */
        uint64_t thread_magic      : 2;   /* random fix value */
        uint64_t thread_counter    : 29;  /* increasing counter (< 536mio ids per thread) */

        /* per-call */
        uint64_t call_random       :  2;  /* random value */
    };

    uint64_t value;
} internal_object_id_t;

CARBON_EXPORT(carbon_object_id_t)
carbon_object_id_create(void)
{
    static bool     process_init;
    static uint64_t process_local_id;
    static uint8_t  process_magic;
    static uint64_t process_counter;

    static uint8_t  global_build_date_bit;
    static uint8_t  global_build_path_bit;

    if (!process_init) {
        srand(time(NULL));
        process_magic = rand();
        process_init = true;
        process_counter = rand();

        const char *file = __FILE__;
        const char *time = __TIME__;

        global_build_path_bit = CARBON_HASH_BERNSTEIN(strlen(file), file) % 2;
        global_build_date_bit = CARBON_HASH_BERNSTEIN(strlen(time), time) % 2;
    }

    if (!thread_local_init) {
        thread_local_counter = rand();
        thread_local_id = (uint64_t) pthread_self();
        process_local_id = getpid();
        thread_local_magic = rand();
        thread_local_init = true;
    }

    internal_object_id_t internal = {
        .global_wallclock  = carbon_time_now_wallclock(),
        .global_build_date = global_build_date_bit,
        .global_build_path = global_build_path_bit,
        .process_id        = process_local_id,
        .process_magic     = process_magic,
        .process_counter   = process_counter++,
        .thread_id         = (uint64_t) thread_local_id,
        .thread_magic      = thread_local_magic,
        .thread_counter    = thread_local_counter++,
        .call_random       = rand()
    };

    return internal.value;
}

CARBON_EXPORT(bool)
carbon_object_id_get_global_wallclocktime(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->global_wallclock;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_global_build_path_bit(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->global_build_path;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_global_build_time_bit(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->global_build_date;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_process_id(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->process_id;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_process_magic(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->process_magic;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_process_counter(uint_fast16_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->process_counter;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_thread_id(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->thread_id;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_thread_magic(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->thread_magic;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_thread_counter(uint_fast32_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->thread_counter;
    return true;
}

CARBON_EXPORT(bool)
carbon_object_id_get_call_random(uint_fast8_t *out, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(out);
    *out = ((internal_object_id_t *) &id)->call_random;
    return true;
}


