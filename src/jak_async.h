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

#ifndef JAK_ASYNC_H
#define JAK_ASYNC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdlib.h>

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

#define JAK_ASYNC_MSG_UNKNOWN_HINT "Unknown threading hint"

typedef uint_fast16_t jak_thread_id_t;

typedef void (*jak_for_body_func_t)(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);

typedef void
(*jak_map_body_func_t)(void *dst, const void *src, size_t src_width, size_t dst_width, size_t len, void *args);

typedef void(*jak_pred_func_t)
        (size_t *matching_positions, size_t *num_matching_positions, const void *src, size_t width, size_t len,
         void *args, size_t position_offset_to_add);

enum jak_threading_hint {
    JAK_THREADING_HINT_SINGLE, JAK_THREADING_HINT_MULTI
};

struct jak_async_func_proxy {
    jak_for_body_func_t function;
    const void *start;
    size_t width;
    size_t len;
    jak_thread_id_t tid;
    void *args;
};

struct jak_filter_arg  {
    size_t num_positions;
    size_t *src_positions;
    const void *start;
    size_t len;
    size_t width;
    void *args;
    jak_pred_func_t pred;
    size_t position_offset_to_add;
};

void *jak_async_for_proxy_function(void *args);

#define JAK_PARALLEL_ERROR(msg, retval)                                                                             \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define JAK_ASYNC_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (JAK_LIKELY(hint == JAK_THREADING_HINT_MULTI)) {                                             \
        return (forMulti);                                                                                             \
    } else if (hint == JAK_THREADING_HINT_SINGLE) {                                                           \
        return (forSingle);                                                                                            \
    } else JAK_PARALLEL_ERROR(JAK_ASYNC_MSG_UNKNOWN_HINT, false);                                                    \
}

bool jak_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args,
             enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             jak_map_body_func_t f, void *args, enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                      enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                      const size_t *src_idx, size_t idxLen, enum jak_threading_hint hint);

bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                           jak_pred_func_t pred, void *args, enum jak_threading_hint hint,
                           uint_fast16_t num_threads);

bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                          jak_pred_func_t pred, void *args, enum jak_threading_hint hint, size_t num_threads);

bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                             void *args);

bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                           void *args, uint_fast16_t num_threads);

bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                       jak_map_body_func_t f, void *args, enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_sync_gather(void *dst, const void *src, size_t width, const size_t *idx,
                                size_t dst_src_len);

bool jak_async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                              uint_fast16_t num_threads);

bool jak_sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx,
                                    size_t num);

bool jak_int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx,
                                       size_t num, uint_fast16_t num_threads);

bool jak_sync_scatter(void *dst, const void *src, size_t width, const size_t *idx,
                                      size_t num);

bool jak_sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                                    uint_fast16_t num_threads);

bool jak_sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                                 const size_t *src_idx, size_t idx_len);

bool jak_async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                               const size_t *src_idx, size_t idx_len);

bool jak_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                      size_t len, jak_pred_func_t pred, void *args);

bool jak_int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                    size_t len, jak_pred_func_t pred, void *args, uint_fast16_t num_threads);

bool jak_int_sync_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width,
                                     size_t len, jak_pred_func_t pred, void *args);

bool jak_async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                                   jak_pred_func_t pred, void *args, size_t num_threads);

bool jak_for(const void *base, size_t width, size_t len, jak_for_body_func_t f, void *args,
             enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             jak_map_body_func_t f, void *args, enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                      enum jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                      const size_t *src_idx, size_t idx_len, enum jak_threading_hint hint);

bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                           jak_pred_func_t pred, void *args, enum jak_threading_hint hint,
                           uint_fast16_t num_threads);

bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                          jak_pred_func_t pred, void *args, enum jak_threading_hint hint, size_t num_threads);

bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                             void *args);

bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                           void *args, uint_fast16_t num_threads);

struct map_args {
    jak_map_body_func_t map_func;
    void *dst;
    const void *src;
    size_t dst_width;
    void *args;
};

void jak_map_proxy(const void *src, size_t src_width, size_t len, void *args, jak_thread_id_t tid);

bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                       jak_map_body_func_t f, void *args, enum jak_threading_hint hint, uint_fast16_t num_threads);

struct gather_scatter_args {
    const size_t *idx;
    const void *src;
    void *dst;
};

void jak_int_async_gather(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);

bool jak_sync_gather(void *dst, const void *src, size_t width, const size_t *idx,
                                size_t dst_src_len);

bool jak_async_gather(void *dst, const void *src, size_t width, const size_t *idx,
                              size_t dst_src_len, uint_fast16_t num_threads);

bool jak_sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx,
                                    size_t num);

void jak_async_gather_adr_func(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);

bool jak_int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx,
                                       size_t num, uint_fast16_t num_threads);

void jak_async_scatter(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid);

bool jak_sync_scatter(void *dst, const void *src, size_t width, const size_t *idx,
                                      size_t num);

bool jak_sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                                    uint_fast16_t num_threads);

bool jak_sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                                 const size_t *src_idx, size_t idx_len);

bool jak_async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                               const size_t *src_idx, size_t idx_len);

bool jak_int_sync_filter_late(size_t *positions, size_t *num_positions, const void *source,
                                     size_t width, size_t length, jak_pred_func_t predicate, void *arguments);

void *jak_int_sync_filter_procy_func(void *args);

bool jak_async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                                   jak_pred_func_t pred, void *args, size_t num_threads);

bool jak_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                      size_t len, jak_pred_func_t pred, void *args);

bool jak_int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                    size_t len, jak_pred_func_t pred, void *args, uint_fast16_t num_threads);

JAK_END_DECL

#endif
