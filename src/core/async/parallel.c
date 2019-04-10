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

#include "core/async/parallel.h"


NG5_EXPORT(void *)
carbon_parallel_for_proxy_function(void * args)
{
    NG5_CAST(carbon_parallel_func_proxy_t *, proxy_arg, args);
    proxy_arg->function(proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args, proxy_arg->tid);
    return NULL;
}

#define NG5_PARALLEL_ERROR(msg, retval)                                                                             \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define PARALLEL_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (NG5_LIKELY(hint == NG5_PARALLEL_THREAD_HINT_MULTI)) {                                             \
        return (forMulti);                                                                                             \
    } else if (hint == NG5_PARALLEL_THREAD_HINT_SINGLE) {                                                           \
        return (forSingle);                                                                                            \
    } else NG5_PARALLEL_ERROR(PARALLEL_MSG_UNKNOWN_HINT, false);                                                    \
}

NG5_EXPORT(bool)
carbon_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                    void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_map(void * dst,
                    const void * src,
                    size_t src_width,
                    size_t len,
                    size_t dst_width,
                    carbon_parallel_map_body f,
                    void * args,
                    carbon_parallel_threading_hint_t hint,
                    uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_gather(void * dst,
                       const void * src,
                       size_t width,
                       const size_t * idx,
                       size_t dstSrcLen,
                       carbon_parallel_threading_hint_t hint,
                       uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_gather_adr(void * dst,
                           const void * src,
                           size_t src_width,
                           const size_t * idx,
                           size_t num,
                           carbon_parallel_threading_hint_t hint,
                           uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_scatter(void * dst,
                        const void * src,
                        size_t width,
                        const size_t * idx,
                        size_t num,
                        carbon_parallel_threading_hint_t hint,
                        uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_shuffle(void * dst, const void * src, size_t width,
                        const size_t * dst_idx, const size_t * src_idx,
                        size_t idxLen, carbon_parallel_threading_hint_t hint);

NG5_EXPORT(bool)
carbon_parallel_filter_early(void * result, size_t * result_size,
                             const void * src,
                             size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                             carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_filter_late(size_t * pos, size_t * num_pos, const void * src,
                            size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                            carbon_parallel_threading_hint_t hint, size_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                               void * args);
NG5_EXPORT(bool)
carbon_parallel_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                             void * args, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_map_exec(void * dst,
                         const void * src,
                         size_t src_width,
                         size_t len,
                         size_t dst_width,
                         carbon_parallel_map_body f,
                         void * args,
                         carbon_parallel_threading_hint_t hint,
                         uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_gather(void * dst, const void * src, size_t width,
                                  const size_t * idx, size_t dstSrcLen);
NG5_EXPORT(bool)
carbon_parallel_parallel_gather(void * dst, const void * src, size_t width,
                                const size_t * idx, size_t dstSrcLen, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_gather_adr(void * dst, const void * src, size_t src_width,
                                      const size_t * idx, size_t num);
NG5_EXPORT(bool)
carbon_parallel_parallel_gather_adr_func(void * dst, const void * src, size_t src_width,
                                         const size_t * idx, size_t num, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_scatter_func(void * dst, const void * src, size_t width,
                                        const size_t * idx, size_t num);
NG5_EXPORT(bool)
carbon_parallel_parallel_scatter_func(void * dst, const void * src, size_t width,
                                      const size_t * idx, size_t num, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_shuffle(void * dst, const void * src, size_t width,
                                   const size_t * dst_idx, const size_t * src_idx,
                                   size_t idx_len);

NG5_EXPORT(bool)
carbon_parallel_parallel_shuffle(void * dst, const void * src, size_t width,
                                 const size_t * dst_idx, const size_t * src_idx,
                                 size_t idx_len);

NG5_EXPORT(bool)
carbon_parallel_sequential_filter_early(void * result, size_t * result_size,
                                        const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                        void * args);

NG5_EXPORT(bool)
carbon_parallel_parallel_filter_early(void * result, size_t * result_size,
                                      const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                      void * args, uint_fast16_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_sequential_filter_late(size_t * pos, size_t * num_pos,
                                       const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                       void * args);

NG5_EXPORT(bool)
carbon_parallel_parallel_filter_late(size_t * pos, size_t * num_pos,
                                     const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                     void * args, size_t num_threads);

NG5_EXPORT(bool)
carbon_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                    void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_for(base, width, len, f, args),
                   carbon_parallel_parallel_for(base, width, len, f, args, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_map(void * dst,
                    const void * src,
                    size_t src_width,
                    size_t len,
                    size_t dst_width,
                    carbon_parallel_map_body f,
                    void * args,
                    carbon_parallel_threading_hint_t hint,
                    uint_fast16_t num_threads)
{
    return carbon_parallel_map_exec(dst, src, src_width, len, dst_width, f, args, hint, num_threads);
}

NG5_EXPORT(bool)
carbon_parallel_gather(void * dst,
                       const void * src,
                       size_t width,
                       const size_t * idx,
                       size_t dst_src_len,
                       carbon_parallel_threading_hint_t hint,
                       uint_fast16_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_gather(dst, src, width, idx, dst_src_len),
                   carbon_parallel_parallel_gather(dst, src, width, idx, dst_src_len, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_gather_adr(void * dst,
                           const void * src,
                           size_t src_width,
                           const size_t * idx,
                           size_t num,
                           carbon_parallel_threading_hint_t hint,
                           uint_fast16_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_gather_adr(dst, src, src_width, idx, num),
                   carbon_parallel_parallel_gather_adr_func(dst, src, src_width, idx, num, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_scatter(void * dst, const void * src, size_t width,
                        const size_t * idx, size_t num, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_scatter_func(dst, src, width, idx, num),
                   carbon_parallel_parallel_scatter_func(dst, src, width, idx, num, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_shuffle(void * dst, const void * src, size_t width,
                        const size_t * dst_idx, const size_t * src_idx,
                        size_t idx_len, carbon_parallel_threading_hint_t hint)
{
    PARALLEL_MATCH(carbon_parallel_sequential_shuffle(dst, src, width, dst_idx, src_idx, idx_len),
                   carbon_parallel_parallel_shuffle(dst, src, width, dst_idx, src_idx, idx_len))
}

NG5_EXPORT(bool)
carbon_parallel_filter_early(void * result, size_t * result_size,
                             const void * src,
                             size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                             carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_filter_early(result, result_size, src, width, len, pred, args),
                   carbon_parallel_parallel_filter_early(result, result_size, src, width, len, pred, args, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_filter_late(size_t * pos, size_t * num_pos, const void * src,
                            size_t width, size_t len, carbon_parallel_predicate pred, void * args,
                            carbon_parallel_threading_hint_t hint, size_t num_threads)
{
    PARALLEL_MATCH(carbon_parallel_sequential_filter_late(pos, num_pos, src, width, len, pred, args),
                   carbon_parallel_parallel_filter_late(pos, num_pos, src, width, len, pred, args, num_threads))
}

NG5_EXPORT(bool)
carbon_parallel_sequential_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                               void * args)
{
    NG5_NON_NULL_OR_ERROR(base)
    NG5_NON_NULL_OR_ERROR(width)
    NG5_NON_NULL_OR_ERROR(len)
    f(base, width, len, args, 0);
    return true;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_for(const void * base, size_t width, size_t len, carbon_parallel_for_body f,
                             void * args, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(base)
    NG5_NON_NULL_OR_ERROR(width)

    if (len > 0) {
        uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */
        pthread_t threads[num_threads];
        carbon_parallel_func_proxy_t proxyArgs[num_thread];
        size_t chunk_len = len / num_thread;
        size_t chunk_len_remain = len % num_thread;
        const void * main_thread_base = base + num_threads * chunk_len * width;

        NG5_PREFETCH_READ(f);
        NG5_PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            carbon_parallel_func_proxy_t *proxy_arg = proxyArgs + tid;
            proxy_arg->start = base + tid * chunk_len * width;
            proxy_arg->len = chunk_len;
            proxy_arg->tid = (tid + 1);
            proxy_arg->width = width;
            proxy_arg->args = args;
            proxy_arg->function = f;

            NG5_PREFETCH_READ(proxy_arg->start);
            pthread_create(threads + tid, NULL, carbon_parallel_for_proxy_function, proxyArgs + tid);
        }
        /** run f on this thread */
        NG5_PREFETCH_READ(main_thread_base);
        f(main_thread_base, width, chunk_len + chunk_len_remain, args, 0);

        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            pthread_join(threads[tid], NULL);
        }
    }
    return true;
}

void
mapProxy(const void * src, size_t src_width, size_t len, void * args,
         thread_id_t tid)
{
    NG5_UNUSED(tid);
    NG5_CAST(MapArgs *, mapArgs, args);
    size_t globalStart = (src - mapArgs->src) / src_width;

    NG5_PREFETCH_READ(mapArgs->src);
    NG5_PREFETCH_READ(mapArgs->args);
    NG5_PREFETCH_WRITE(mapArgs->dst);
    mapArgs->mapFunction(mapArgs->dst + globalStart * mapArgs->dst_width,
                         src, src_width, mapArgs->dst_width, len, mapArgs->args);
}

NG5_EXPORT(bool)
carbon_parallel_map_exec(void * dst, const void * src, size_t src_width, size_t len, size_t
dst_width, carbon_parallel_map_body f, void * args, carbon_parallel_threading_hint_t hint, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(src_width)
    NG5_NON_NULL_OR_ERROR(dst_width)
    NG5_NON_NULL_OR_ERROR(f)

    NG5_PREFETCH_READ(f);
    NG5_PREFETCH_WRITE(dst);

    MapArgs mapArgs = {
        .args = args,
        .mapFunction = f,
        .dst = dst,
        .dst_width = dst_width,
        .src = src
    };

    return carbon_parallel_for((void *) src, src_width, len, &mapProxy, &mapArgs, hint, num_threads);
}

void
carbon_gather_function(const void * start, size_t width, size_t len, void * args,
                       thread_id_t tid)
{
    NG5_UNUSED(tid);
    NG5_CAST(carbon_gather_scatter_args_t *, gather_args, args);
    size_t global_index_start = (start - gather_args->dst) / width;

    NG5_PREFETCH_WRITE(gather_args->dst);
    NG5_PREFETCH_WRITE(gather_args->idx);
    NG5_PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;
        memcpy(gather_args->dst + global_index_cur * width,
               gather_args->src + gather_args->idx[global_index_cur] * width,
               width);

        bool has_next = (next_i < len);
        NG5_PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
        NG5_PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * width : NULL);
    }
}

NG5_EXPORT(bool)
carbon_parallel_sequential_gather(void * dst, const void * src, size_t width,
                                  const size_t * idx, size_t dst_src_len)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(width)

    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);

    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);
    NG5_PREFETCH_READ((dst_src_len > 0) ? src + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < dst_src_len; next_i = ++i + 1) {
        memcpy(dst + i * width, src + idx[i] * width, width);

        bool has_next = (next_i < dst_src_len);
        NG5_PREFETCH_READ(has_next ? idx + next_i : NULL);
        NG5_PREFETCH_READ(has_next ? src + idx[next_i] * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? dst + next_i * width : NULL);
    }

    return true;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_gather(void * dst, const void * src, size_t width,
                                const size_t * idx, size_t dst_src_len, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(width)

    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);

    carbon_gather_scatter_args_t args = {
        .idx           = idx,
        .src           = src,
        .dst           = dst,
    };
    return carbon_parallel_parallel_for(dst, width, dst_src_len, carbon_gather_function, &args, num_threads);
}

NG5_EXPORT(bool)
carbon_parallel_sequential_gather_adr(void * dst, const void * src, size_t src_width,
                                      const size_t * idx, size_t num)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(src_width)

    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);

    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);
    NG5_PREFETCH_READ(num > 0 ? src + idx[0] * src_width : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
        const void *ptr = src + idx[i] * src_width;
        size_t adr = (size_t) ptr;
        memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < num);
        NG5_PREFETCH_READ(has_next ? idx + next_i : NULL);
        NG5_PREFETCH_READ(has_next ? src + idx[next_i] * src_width : NULL);
        NG5_PREFETCH_WRITE(has_next ? dst + next_i * sizeof(void *) : NULL);
    }
    return true;
}

 void
carbon_parallel_gather_adr_func(const void * start, size_t width, size_t len, void * args,
                                thread_id_t tid)
{
    NG5_UNUSED(tid);
    NG5_CAST(carbon_gather_scatter_args_t *, gather_args, args);

    NG5_PREFETCH_READ(gather_args->idx);
    NG5_PREFETCH_WRITE(gather_args->dst);
    NG5_PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

    size_t global_index_start = (start - gather_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;
        const void *ptr = gather_args->src + gather_args->idx[global_index_cur] * width;
        size_t adr = (size_t) ptr;
        memcpy(gather_args->dst + global_index_cur * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < len);
        NG5_PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
        NG5_PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * sizeof(void *) : NULL);
    }
}

NG5_EXPORT(bool)
carbon_parallel_parallel_gather_adr_func(void * dst, const void * src, size_t src_width,
                                         const size_t * idx, size_t num, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(src_width)

    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);

    carbon_gather_scatter_args_t args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return carbon_parallel_parallel_for(dst, src_width, num, carbon_parallel_gather_adr_func, &args, num_threads);
}

 void
carbon_parallel_scatter_func(const void * start, size_t width, size_t len, void * args,
                             thread_id_t tid)
{
    NG5_UNUSED(tid);
    NG5_CAST(carbon_gather_scatter_args_t *, scatter_args, args);

    NG5_PREFETCH_READ(scatter_args->idx);
    NG5_PREFETCH_READ(scatter_args->src);
    NG5_PREFETCH_WRITE((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

    size_t global_index_start = (start - scatter_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;

        memcpy(scatter_args->dst + scatter_args->idx[global_index_cur] * width,
               scatter_args->src + global_index_cur * width,
               width);

        bool has_next = (next_i < len);
        NG5_PREFETCH_READ(has_next ? scatter_args->idx + global_index_next : NULL);
        NG5_PREFETCH_READ(has_next ? scatter_args->src + global_index_next * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? scatter_args->dst + scatter_args->idx[global_index_next] * width : NULL);
    }
}

NG5_EXPORT(bool)
carbon_parallel_sequential_scatter_func(void * dst, const void * src, size_t width,
                                        const size_t * idx, size_t num)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(width)

    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_WRITE((num > 0) ? dst + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
        memcpy(dst + idx[i] * width, src + i * width, width);

        bool has_next = (next_i < num);
        NG5_PREFETCH_READ(has_next ? idx + next_i : NULL);
        NG5_PREFETCH_READ(has_next ? src + next_i * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? dst + idx[next_i] * width : NULL);
    }
    return true;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_scatter_func(void * dst, const void * src, size_t width,
                                      const size_t * idx, size_t num, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(idx)
    NG5_NON_NULL_OR_ERROR(width)

    NG5_PREFETCH_READ(src);
    NG5_PREFETCH_READ(idx);
    NG5_PREFETCH_WRITE(dst);

    carbon_gather_scatter_args_t args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return carbon_parallel_parallel_for(dst, width, num, carbon_parallel_scatter_func, &args, num_threads);
}

NG5_EXPORT(bool)
carbon_parallel_sequential_shuffle(void * dst, const void * src, size_t width,
                                   const size_t * dst_idx, const size_t * src_idx,
                                   size_t idx_len)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_NON_NULL_OR_ERROR(dst_idx)
    NG5_NON_NULL_OR_ERROR(src_idx)
    NG5_NON_NULL_OR_ERROR(width)

    bool has_first = (idx_len > 0);
    NG5_PREFETCH_READ(src_idx);
    NG5_PREFETCH_READ(dst_idx);
    NG5_PREFETCH_READ(has_first ? src + src_idx[0] * width : NULL);
    NG5_PREFETCH_WRITE(has_first ? dst + dst_idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1) {
        memcpy(dst + dst_idx[i] * width, src + src_idx[i] * width, width);

        bool has_next = (next_i < idx_len);
        NG5_PREFETCH_READ(has_next ? src_idx + next_i : NULL);
        NG5_PREFETCH_READ(has_next ? dst_idx + next_i : NULL);
        NG5_PREFETCH_READ(has_next ? src + src_idx[next_i] * width : NULL);
        NG5_PREFETCH_WRITE(has_next ? dst + dst_idx[next_i] * width : NULL);
    }

    return true;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_shuffle(void * dst, const void * src, size_t width,
                                 const size_t * dst_idx, const size_t * src_idx,
                                 size_t idx_len)
{
    NG5_UNUSED(dst);
    NG5_UNUSED(src);
    NG5_UNUSED(width);
    NG5_UNUSED(dst_idx);
    NG5_UNUSED(src_idx);
    NG5_UNUSED(idx_len);
    NG5_NOT_IMPLEMENTED
}

NG5_EXPORT(bool)
carbon_parallel_sequential_filter_late(size_t * positions, size_t * num_positions,
                                       const void * source, size_t width, size_t length, carbon_parallel_predicate predicate,
                                       void * arguments)
{
    NG5_NON_NULL_OR_ERROR(positions);
    NG5_NON_NULL_OR_ERROR(num_positions);
    NG5_NON_NULL_OR_ERROR(source);
    NG5_NON_NULL_OR_ERROR(width);
    NG5_NON_NULL_OR_ERROR(length);
    NG5_NON_NULL_OR_ERROR(predicate);

    predicate(positions, num_positions, source, width, length, arguments, 0);

    return true;
}

 void *
carbon_parallel_filter_proxy_func(void * args)
{
    NG5_CAST(carbon_filter_arg_t *, proxy_arg, args);
    proxy_arg->pred(proxy_arg->src_positions, &proxy_arg->num_positions,
                    proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args,
                    proxy_arg->position_offset_to_add);
    return NULL;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_filter_late(size_t * pos, size_t * num_pos,
                                     const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                     void * args, size_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(pos);
    NG5_NON_NULL_OR_ERROR(num_pos);
    NG5_NON_NULL_OR_ERROR(src);
    NG5_NON_NULL_OR_ERROR(width);
    NG5_NON_NULL_OR_ERROR(pred);

    if (NG5_UNLIKELY(len == 0)) {
        *num_pos = 0;
        return true;
    }

    uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

    pthread_t threads[num_threads];
    carbon_filter_arg_t thread_args[num_thread];

    register size_t chunk_len = len / num_thread;
    size_t chunk_len_remain = len % num_thread;
    size_t main_position_offset_to_add = num_threads * chunk_len;
    const void * main_thread_base = src + main_position_offset_to_add * width;

    NG5_PREFETCH_READ(pred);
    NG5_PREFETCH_READ(args);

    /** run f on NTHREADS_FOR additional threads */
    if (NG5_LIKELY(chunk_len > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            carbon_filter_arg_t *arg = thread_args + tid;
            arg->num_positions = 0;
            arg->src_positions = malloc(chunk_len * sizeof(size_t));
            arg->position_offset_to_add = tid * chunk_len;
            arg->start = src + arg->position_offset_to_add * width;
            arg->len = chunk_len;
            arg->width = width;
            arg->args = args;
            arg->pred = pred;

            NG5_PREFETCH_READ(arg->start);
            pthread_create(threads + tid, NULL, carbon_parallel_filter_proxy_func, arg);
        }
    }
    /** run f on this thread */
    NG5_PREFETCH_READ(main_thread_base);
    size_t main_chunk_len = chunk_len + chunk_len_remain;
    size_t *main_src_positions = malloc(main_chunk_len * sizeof(size_t));
    size_t main_num_positions = 0;

    pred(main_src_positions,
         &main_num_positions,
         main_thread_base,
         width,
         main_chunk_len,
         args,
         main_position_offset_to_add);

    size_t total_num_matching_positions = 0;

    if (NG5_LIKELY(chunk_len > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            pthread_join(threads[tid], NULL);
            const carbon_filter_arg_t * thread_arg = (thread_args + tid);
            if (thread_arg->num_positions > 0) {
                memcpy(pos + total_num_matching_positions, thread_arg->src_positions,
                       thread_arg->num_positions * sizeof(size_t));
                total_num_matching_positions += thread_arg->num_positions;
            }
            free(thread_args[tid].src_positions);
        }
    }

    if (NG5_LIKELY(main_num_positions > 0)) {
        memcpy(pos + total_num_matching_positions, main_src_positions,
               main_num_positions * sizeof(size_t));
        total_num_matching_positions += main_num_positions;
    }
    free(main_src_positions);

    *num_pos = total_num_matching_positions;

    return true;
}

NG5_EXPORT(bool)
carbon_parallel_sequential_filter_early(void * result, size_t * result_size,
                                        const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                        void * args)
{
    NG5_NON_NULL_OR_ERROR(result);
    NG5_NON_NULL_OR_ERROR(result_size);
    NG5_NON_NULL_OR_ERROR(src);
    NG5_NON_NULL_OR_ERROR(width);
    NG5_NON_NULL_OR_ERROR(len);
    NG5_NON_NULL_OR_ERROR(pred);

    size_t num_matching_positions;
    size_t * matching_positions = malloc(len * sizeof(size_t));

    pred(matching_positions, &num_matching_positions, src, width, len, args, 0);

    carbon_parallel_gather(result, src, width, matching_positions, num_matching_positions, NG5_PARALLEL_THREAD_HINT_SINGLE, 0);
    *result_size = num_matching_positions;

    free(matching_positions);

    return true;
}

NG5_EXPORT(bool)
carbon_parallel_parallel_filter_early(void * result, size_t * result_size,
                                      const void * src, size_t width, size_t len, carbon_parallel_predicate pred,
                                      void * args, uint_fast16_t num_threads)
{
    NG5_NON_NULL_OR_ERROR(result);
    NG5_NON_NULL_OR_ERROR(result_size);
    NG5_NON_NULL_OR_ERROR(src);
    NG5_NON_NULL_OR_ERROR(width);
    NG5_NON_NULL_OR_ERROR(len);
    NG5_NON_NULL_OR_ERROR(pred);

    uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

    pthread_t threads[num_threads];
    carbon_filter_arg_t thread_args[num_thread];

    register size_t chunk_len = len / num_thread;
    size_t chunk_len_remain = len % num_thread;
    size_t main_position_offset_to_add = num_threads * chunk_len;
    const void * main_thread_base = src + main_position_offset_to_add * width;

    NG5_PREFETCH_READ(pred);
    NG5_PREFETCH_READ(args);

    /** run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        carbon_filter_arg_t *arg = thread_args + tid;
        arg->num_positions = 0;
        arg->src_positions = malloc(chunk_len * sizeof(size_t));
        arg->position_offset_to_add = tid * chunk_len;
        arg->start = src + arg->position_offset_to_add * width;
        arg->len = chunk_len;
        arg->width = width;
        arg->args = args;
        arg->pred = pred;

        NG5_PREFETCH_READ(arg->start);
        pthread_create(threads + tid, NULL, carbon_parallel_filter_proxy_func, arg);
    }
    /** run f on this thread */
    NG5_PREFETCH_READ(main_thread_base);
    size_t main_chunk_len = chunk_len + chunk_len_remain;
    size_t *main_src_positions = malloc(main_chunk_len * sizeof(size_t));
    size_t main_num_positions = 0;

    pred(main_src_positions, &main_num_positions, main_thread_base, width, main_chunk_len, args,
         main_position_offset_to_add);


    size_t total_num_matching_positions = main_num_positions;
    size_t partial_num_matching_positions = 0;

    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        pthread_join(threads[tid], NULL);
        const carbon_filter_arg_t * thread_arg = (thread_args + tid);
        total_num_matching_positions += thread_arg->num_positions;
        NG5_PREFETCH_READ(thread_arg->src_positions);
    }

    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        const carbon_filter_arg_t * thread_arg = (thread_args + tid);

        if (NG5_LIKELY(thread_arg->num_positions > 0)) {
            carbon_parallel_gather(result + partial_num_matching_positions * width, src, width, thread_arg->src_positions,
                                   thread_arg->num_positions,
                                   NG5_PARALLEL_THREAD_HINT_MULTI, num_threads);
        }

        partial_num_matching_positions += thread_arg->num_positions;
        free(thread_arg->src_positions);
    }

    if (NG5_LIKELY(main_num_positions > 0)) {
        carbon_parallel_gather(result + partial_num_matching_positions * width, src, width, main_src_positions,
                               main_num_positions, NG5_PARALLEL_THREAD_HINT_MULTI, num_threads);
    }
    free(main_src_positions);

    *result_size = total_num_matching_positions;

    return true;
}