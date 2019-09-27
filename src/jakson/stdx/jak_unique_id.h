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

#ifndef JAK_UNIQUE_ID_H
#define JAK_UNIQUE_ID_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include <jakson/jak_types.h>

JAK_BEGIN_DECL

typedef jak_u64 jak_uid_t;

bool jak_unique_id_create(jak_uid_t *out);

bool jak_unique_id_get_global_wallclocktime(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_global_build_path_bit(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_global_build_time_bit(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_id(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_magic(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_process_counter(uint_fast16_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_id(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_magic(uint_fast8_t *out, jak_uid_t id);
bool jak_unique_id_get_thread_counter(uint_fast32_t *out, jak_uid_t id);
bool jak_unique_id_get_call_random(uint_fast8_t *out, jak_uid_t id);

JAK_END_DECL

#endif