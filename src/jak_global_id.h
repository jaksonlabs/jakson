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

#ifndef JAK_GLOBAL_ID_H
#define JAK_GLOBAL_ID_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

typedef jak_u64 global_id_t;

bool global_id_create(global_id_t *out);

bool global_id_get_global_wallclocktime(uint_fast8_t *out, global_id_t id);

bool global_id_get_global_build_path_bit(uint_fast8_t *out, global_id_t id);

bool global_id_get_global_build_time_bit(uint_fast8_t *out, global_id_t id);

bool global_id_get_process_id(uint_fast8_t *out, global_id_t id);

bool global_id_get_process_magic(uint_fast8_t *out, global_id_t id);

bool global_id_get_process_counter(uint_fast16_t *out, global_id_t id);

bool global_id_get_thread_id(uint_fast8_t *out, global_id_t id);

bool global_id_get_thread_magic(uint_fast8_t *out, global_id_t id);

bool global_id_get_thread_counter(uint_fast32_t *out, global_id_t id);

bool global_id_get_call_random(uint_fast8_t *out, global_id_t id);

JAK_END_DECL

#endif