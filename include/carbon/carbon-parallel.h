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

#ifndef CARBON_PARALLEL_H
#define CARBON_PARALLEL_H

#include "carbon-common.h"
#include "stdlib.h"
#include "carbon-error.h"

CARBON_BEGIN_DECL

#define PARALLEL_MSG_UNKNOWN_HINT "Unknown threading hint"

typedef uint_fast16_t thread_id_t;

typedef void (*carbon_parallel_for_body)(const void * start, size_t width, size_t len, void * args, thread_id_t tid);

typedef void (*carbon_parallel_map_body)(void * dst, const void * src, size_t src_width, size_t dst_width,
                                size_t len, void * args);

typedef void (*carbon_parallel_predicate)(size_t *matching_positions, size_t *num_matching_positions, const void * src,
                            size_t width, size_t len, void * args, size_t position_offset_to_add);

typedef enum carbon_parallel_threading_hint
{
    CARBON_PARALLEL_THREAD_HINT_SINGLE,
    CARBON_PARALLEL_THREAD_HINT_MULTI
} carbon_parallel_threading_hint_t;

typedef struct carbon_parallel_func_proxy
{
    carbon_parallel_for_body function;
    const void * start;
    size_t width;
    size_t len;
    thread_id_t tid;
    void *args;
} carbon_parallel_func_proxy_t;

typedef struct carbon_filter_arg
{
    size_t num_positions;
    size_t * src_positions;
    const void * start;
    size_t len;
    size_t width;
    void * args;
    carbon_parallel_predicate pred;
    size_t position_offset_to_add;
} carbon_filter_arg_t;


CARBON_EXPORT(void *)
carbon_parallel_for_proxy_function(void * args);

#define CARBON_PARALLEL_ERROR(msg, retval)                                                                             \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define PARALLEL_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (CARBON_BRANCH_LIKELY(hint == CARBON_PARALLEL_THREAD_HINT_MULTI)) {                                             \
        return (forMulti);                                                                                             \
    } else if (hint == CARBON_PARALLEL_THREAD_HINT_SINGLE) {                                                           \
        return (forSingle);                                                                                            \
    } else CARBON_PARALLEL_ERROR(PARALLEL_MSG_UNKNOWN_HINT, false);                                                    \
}

CARBON_EXPORT(bool)
carbon_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                              void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_map(void * dst,
                              const void * src,
                              size_t src_width,
                              size_t len,
                              size_t dst_width,
                              carbon_parallel_map_body f,
                              void * args,
                              carbon_parallel_threading_hint_t hint,
                              uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_gather(void * dst,
                                 const void * src,
                                 size_t width,
                                 const size_t * idx,
                                 size_t dstSrcLen,
                                 carbon_parallel_threading_hint_t hint,
                                 uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_gather_adr(void * dst,
                                     const void * src,
                                     size_t src_width,
                                     const size_t * idx,
                                     size_t num,
                                     carbon_parallel_threading_hint_t hint,
                                     uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_scatter(void * dst,
                                  const void * src,
                                  size_t width,
                                  const size_t * idx,
                                  size_t num,
                                  carbon_parallel_threading_hint_t hint,
                                  uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_shuffle(void * dst, const void * src, size_t width,
                                  const size_t * dst_idx, const size_t * src_idx,
                                  size_t idxLen, carbon_parallel_threading_hint_t hint);

CARBON_EXPORT(bool)
carbon_parallel_filter_early(void * result, size_t * result_size,
                                       const void * src,
                                       size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                                       carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_filter_late(size_t * pos, size_t * num_pos, const void * src,
                                      size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                                      carbon_parallel_threading_hint_t hint, size_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                                   void * args);
CARBON_EXPORT(bool)
carbon_parallel_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                                 void * args, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_map_exec(void * dst,
                        const void * src,
                        size_t src_width,
                        size_t len,
                        size_t dst_width,
                        carbon_parallel_map_body f,
                        void * args,
                        carbon_parallel_threading_hint_t hint,
                        uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_gather(void * dst, const void * src, size_t width,
                                      const size_t * idx, size_t dstSrcLen);
CARBON_EXPORT(bool)
carbon_parallel_parallel_gather(void * dst, const void * src, size_t width,
                                    const size_t * idx, size_t dstSrcLen, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_gather_adr(void * dst, const void * src, size_t src_width,
                                          const size_t * idx, size_t num);
CARBON_EXPORT(bool)
carbon_parallel_parallel_gather_adr_func(void * dst, const void * src, size_t src_width,
                                        const size_t * idx, size_t num, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_scatter_func(void * dst, const void * src, size_t width,
                                       const size_t * idx, size_t num);
CARBON_EXPORT(bool)
carbon_parallel_parallel_scatter_func(void * dst, const void * src, size_t width,
                                     const size_t * idx, size_t num, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_shuffle(void * dst, const void * src, size_t width,
                                       const size_t * dst_idx, const size_t * src_idx,
                                       size_t idx_len);

CARBON_EXPORT(bool)
carbon_parallel_parallel_shuffle(void * dst, const void * src, size_t width,
                                     const size_t * dst_idx, const size_t * src_idx,
                                     size_t idx_len);

CARBON_EXPORT(bool)
carbon_parallel_sequential_filter_early(void * result, size_t * result_size,
                                            const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                            void * args);

CARBON_EXPORT(bool)
carbon_parallel_parallel_filter_early(void * result, size_t * result_size,
                                          const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                          void * args, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_filter_late(size_t * pos, size_t * num_pos,
                                           const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                           void * args);

CARBON_EXPORT(bool)
carbon_parallel_parallel_filter_late(size_t * pos, size_t * num_pos,
                                         const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                         void * args, size_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                              void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_map(void * dst,
                              const void * src,
                              size_t src_width,
                              size_t len,
                              size_t dst_width,
                              carbon_parallel_map_body f,
                              void * args,
                              carbon_parallel_threading_hint_t hint,
                              uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_gather(void * dst,
                                 const void * src,
                                 size_t width,
                                 const size_t * idx,
                                 size_t dst_src_len,
                                 carbon_parallel_threading_hint_t hint,
                                 uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_gather_adr(void * dst,
                                     const void * src,
                                     size_t src_width,
                                     const size_t * idx,
                                     size_t num,
                                     carbon_parallel_threading_hint_t hint,
                                     uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_scatter(void * dst, const void * src, size_t width,
                                  const size_t * idx, size_t num, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_shuffle(void * dst, const void * src, size_t width,
                                  const size_t * dst_idx, const size_t * src_idx,
                                  size_t idx_len, carbon_parallel_threading_hint_t hint);

CARBON_EXPORT(bool)
carbon_parallel_filter_early(void * result, size_t * result_size,
                                       const void * src,
                                       size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                                       carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_filter_late(size_t * pos, size_t * num_pos, const void * src,
                                      size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                                      carbon_parallel_threading_hint_t hint, size_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                                   void * args);

CARBON_EXPORT(bool)
carbon_parallel_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                                 void * args, uint_fast16_t num_threads);

typedef struct MapArgs
{
    carbon_parallel_map_body mapFunction;
    void * dst;
    const void * src;
    size_t dst_width;
    void *args;
} MapArgs;

inline static void
mapProxy(const void * src, size_t src_width, size_t len, void * args,
                               thread_id_t tid);

CARBON_EXPORT(bool)
carbon_parallel_map_exec(void * dst, const void * src, size_t src_width, size_t len, size_t
                      dst_width, carbon_parallel_map_body f, void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

typedef struct carbon_gather_scatter_args
{
    const size_t * idx;
    const void * src;
    void * dst;
} carbon_gather_scatter_args_t;

inline static void
carbon_gather_function(const void * start, size_t width, size_t len, void * args,
                                 thread_id_t tid);

CARBON_EXPORT(bool)
carbon_parallel_sequential_gather(void * dst, const void * src, size_t width,
                                      const size_t * idx, size_t dst_src_len);

CARBON_EXPORT(bool)
carbon_parallel_parallel_gather(void * dst, const void * src, size_t width,
                                    const size_t * idx, size_t dst_src_len, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_gather_adr(void * dst, const void * src, size_t src_width,
                                          const size_t * idx, size_t num);

inline static void
carbon_parallel_gather_adr_func(const void * start, size_t width, size_t len, void * args,
                                     thread_id_t tid);

CARBON_EXPORT(bool)
carbon_parallel_parallel_gather_adr_func(void * dst, const void * src, size_t src_width,
                                     const size_t * idx, size_t num, uint_fast16_t num_threads);

inline static void
carbon_parallel_scatter_func(const void * start, size_t width, size_t len, void * args,
                                   thread_id_t tid);

CARBON_EXPORT(bool)
carbon_parallel_sequential_scatter_func(void * dst, const void * src, size_t width,
                                    const size_t * idx, size_t num);

CARBON_EXPORT(bool)
carbon_parallel_parallel_scatter_func(void * dst, const void * src, size_t width,
                                  const size_t * idx, size_t num, uint_fast16_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_shuffle(void * dst, const void * src, size_t width,
                                    const size_t * dst_idx, const size_t * src_idx,
                                    size_t idx_len);

CARBON_EXPORT(bool)
carbon_parallel_parallel_shuffle(void * dst, const void * src, size_t width,
                                  const size_t * dst_idx, const size_t * src_idx,
                                  size_t idx_len);

CARBON_EXPORT(bool)
carbon_parallel_sequential_filter_late(size_t * positions, size_t * num_positions,
                                       const void * source, size_t width, size_t length, carbon_parallel_predicate predicate,
                                       void * arguments);

inline static void *
carbon_parallel_filter_proxy_func(void * args);

CARBON_EXPORT(bool)
carbon_parallel_parallel_filter_late(size_t * pos, size_t * num_pos,
                                         const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                         void * args, size_t num_threads);

CARBON_EXPORT(bool)
carbon_parallel_sequential_filter_early(void * result, size_t * result_size,
                                            const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                            void * args);

CARBON_EXPORT(bool)
carbon_parallel_parallel_filter_early(void * result, size_t * result_size,
                                          const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                          void * args, uint_fast16_t num_threads);

CARBON_END_DECL

#endif
