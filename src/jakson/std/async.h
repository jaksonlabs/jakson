/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef ASYNC_H
#define ASYNC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdlib.h>

#include <jakson/stdinc.h>
#include <jakson/error.h>

BEGIN_DECL

#define ASYNC_MSG_UNKNOWN_HINT "Unknown threading hint"

typedef uint_fast16_t thread_id_t;

typedef void (*for_body_func_t)(const void *start, size_t width, size_t len, void *args, thread_id_t tid);
typedef void (*map_body_func_t)(void *dst, const void *src, size_t src_width, size_t dst_width, size_t len, void *args);
typedef void(*pred_func_t)(size_t *matching_positions, size_t *num_matching_positions, const void *src, size_t width, size_t len, void *args, size_t position_offset_to_add);

typedef enum threading_hint {
        THREADING_HINT_SINGLE, THREADING_HINT_MULTI
} threading_hint;

typedef struct async_func_proxy {
        for_body_func_t function;
        const void *start;
        size_t width;
        size_t len;
        thread_id_t tid;
        void *args;
} async_func_proxy;

typedef struct filter_arg {
        size_t num_positions;
        size_t *src_positions;
        const void *start;
        size_t len;
        size_t width;
        void *args;
        pred_func_t pred;
        size_t position_offset_to_add;
} filter_arg;

typedef struct map_args {
        map_body_func_t map_func;
        void *dst;
        const void *src;
        size_t dst_width;
        void *args;
} map_args;

typedef struct gather_scatter_args {
        const size_t *idx;
        const void *src;
        void *dst;
} gather_scatter_args;

void *async_for_proxy_function(void *args);

#define PARALLEL_ERROR(msg, retval)                                                                             \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define ASYNC_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (LIKELY(hint == THREADING_HINT_MULTI)) {                                             \
        return (forMulti);                                                                                             \
    } else if (hint == THREADING_HINT_SINGLE) {                                                           \
        return (forSingle);                                                                                            \
    } else PARALLEL_ERROR(ASYNC_MSG_UNKNOWN_HINT, false);                                                    \
}

bool foreach(const void *base, size_t width, size_t len, for_body_func_t f, void *args, threading_hint hint,
             uint_fast16_t num_threads);
bool map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);
bool gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, threading_hint hint, uint_fast16_t num_threads);
bool gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, threading_hint hint, uint_fast16_t num_threads);
bool scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num, threading_hint hint, uint_fast16_t num_threads);
bool shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idxLen, threading_hint hint);
bool filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args, threading_hint hint, uint_fast16_t num_threads);
bool filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, pred_func_t pred, void *args, threading_hint hint, size_t num_threads);

bool sync_for(const void *base, size_t width, size_t len, for_body_func_t f, void *args);
bool async_for(const void *base, size_t width, size_t len, for_body_func_t f, void *args, uint_fast16_t num_threads);
bool async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);
bool sync_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len);
bool async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, uint_fast16_t num_threads);
bool sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num);
bool int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool sync_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num);
bool sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args);
bool int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args, uint_fast16_t num_threads);
bool int_sync_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, pred_func_t pred, void *args);
bool async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, pred_func_t pred, void *args, size_t num_threads);

bool foreach(const void *base, size_t width, size_t len, for_body_func_t f, void *args, threading_hint hint,
             uint_fast16_t num_threads);
bool map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);
bool gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, threading_hint hint, uint_fast16_t num_threads);
bool gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, threading_hint hint, uint_fast16_t num_threads);
bool scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num, threading_hint hint, uint_fast16_t num_threads);
bool shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len, threading_hint hint);
bool filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args, threading_hint hint, uint_fast16_t num_threads);
bool filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, pred_func_t pred, void *args, threading_hint hint, size_t num_threads);
bool sync_for(const void *base, size_t width, size_t len, for_body_func_t f, void *args);
bool async_for(const void *base, size_t width, size_t len, for_body_func_t f, void *args, uint_fast16_t num_threads);

void map_proxy(const void *src, size_t src_width, size_t len, void *args, thread_id_t tid);
bool async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width, map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);
void int_async_gather(const void *start, size_t width, size_t len, void *args, thread_id_t tid);

bool sync_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len);
bool async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len, uint_fast16_t num_threads);
bool sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num);
void async_gather_adr_func(const void *start, size_t width, size_t len, void *args, thread_id_t tid);
bool int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num, uint_fast16_t num_threads);
void async_scatter(const void *start, size_t width, size_t len, void *args, thread_id_t tid);
bool sync_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num);
bool sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num, uint_fast16_t num_threads);
bool sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);
bool async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx, const size_t *src_idx, size_t idx_len);

bool int_sync_filter_late(size_t *positions, size_t *num_positions, const void *source, size_t width, size_t length, pred_func_t predicate, void *arguments);
void *int_sync_filter_procy_func(void *args);

bool async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len, pred_func_t pred, void *args, size_t num_threads);
bool async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args);
bool int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len, pred_func_t pred, void *args, uint_fast16_t num_threads);

END_DECL

#endif
