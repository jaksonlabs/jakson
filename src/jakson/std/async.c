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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/std/async.h>

void *async_for_proxy_function(void *args)
{
        cast(async_func_proxy *, proxy_arg, args);
        proxy_arg->function(proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args, proxy_arg->tid);
        return NULL;
}

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

bool foreach(const void *base, size_t width, size_t len, for_body_func_t f, void *args,
             threading_hint hint, uint_fast16_t num_threads);

bool map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);

bool gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                threading_hint hint, uint_fast16_t num_threads);

bool gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    threading_hint hint, uint_fast16_t num_threads);

bool scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                 threading_hint hint, uint_fast16_t num_threads);

bool shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                 const size_t *src_idx, size_t idxLen, threading_hint hint);

bool filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                      pred_func_t pred, void *args, threading_hint hint,
                      uint_fast16_t num_threads);

bool filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                     pred_func_t pred, void *args, threading_hint hint, size_t num_threads);

bool sync_for(const void *base, size_t width, size_t len, for_body_func_t f,
                  void *args);

bool async_for(const void *base, size_t width, size_t len, for_body_func_t f,
                   void *args, uint_fast16_t num_threads);

bool async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                        map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads);

bool sync_gather(void *dst, const void *src, size_t width, const size_t *idx,
                     size_t dst_src_len);

bool async_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                      uint_fast16_t num_threads);

bool sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx,
                         size_t num);

bool int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx,
                                   size_t num, uint_fast16_t num_threads);

bool sync_scatter(void *dst, const void *src, size_t width, const size_t *idx,
                      size_t num);

bool sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                           uint_fast16_t num_threads);

bool sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                      const size_t *src_idx, size_t idx_len);

bool async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                       const size_t *src_idx, size_t idx_len);

bool async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                            size_t len, pred_func_t pred, void *args);

bool int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                size_t len, pred_func_t pred, void *args, uint_fast16_t num_threads);

bool int_sync_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width,
                              size_t len, pred_func_t pred, void *args);

bool async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                           pred_func_t pred, void *args, size_t num_threads);

bool foreach(const void *base, size_t width, size_t len, for_body_func_t f, void *args,
             threading_hint hint, uint_fast16_t num_threads)
{
        ASYNC_MATCH(sync_for(base, width, len, f, args),
                        async_for(base, width, len, f, args, num_threads))
}

bool map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads)
{
        return async_map_exec(dst, src, src_width, len, dst_width, f, args, hint, num_threads);
}

bool gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                threading_hint hint, uint_fast16_t num_threads)
{
        ASYNC_MATCH(sync_gather(dst, src, width, idx, dst_src_len),
                        async_gather(dst, src, width, idx, dst_src_len, num_threads))
}

bool gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    threading_hint hint, uint_fast16_t num_threads)
{
        ASYNC_MATCH(sync_gather_adr(dst, src, src_width, idx, num),
                        int_async_gather_adr_func(dst, src, src_width, idx, num, num_threads))
}

bool scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                 threading_hint hint, uint_fast16_t num_threads)
{
        ASYNC_MATCH(sync_scatter(dst, src, width, idx, num),
                        sync_scatter_func(dst, src, width, idx, num, num_threads))
}

bool shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                 const size_t *src_idx, size_t idx_len, threading_hint hint)
{
        ASYNC_MATCH(sync_shuffle(dst, src, width, dst_idx, src_idx, idx_len),
                        async_shuffle(dst, src, width, dst_idx, src_idx, idx_len))
}

bool filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                      pred_func_t pred, void *args, threading_hint hint,
                      uint_fast16_t num_threads)
{
        ASYNC_MATCH(async_filter_early(result, result_size, src, width, len, pred, args),
                        int_async_filter_early(result, result_size, src, width, len, pred, args, num_threads))
}

bool filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                     pred_func_t pred, void *args, threading_hint hint, size_t num_threads)
{
        ASYNC_MATCH(int_sync_filter_late(pos, num_pos, src, width, len, pred, args),
                        async_filter_late(pos, num_pos, src, width, len, pred, args, num_threads))
}

bool sync_for(const void *base, size_t width, size_t len, for_body_func_t f,
                  void *args)
{
        ERROR_IF_NULL(base)
        ERROR_IF_NULL(width)
        ERROR_IF_NULL(len)
        f(base, width, len, args, 0);
        return true;
}

bool async_for(const void *base, size_t width, size_t len, for_body_func_t f,
                   void *args, uint_fast16_t num_threads)
{
        ERROR_IF_NULL(base)
        ERROR_IF_NULL(width)

        if (len > 0) {
                uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */
                pthread_t threads[num_threads];
                async_func_proxy proxyArgs[num_thread];
                size_t chunk_len = len / num_thread;
                size_t chunk_len_remain = len % num_thread;
                const void *main_thread_base = base + num_threads * chunk_len * width;

                PREFETCH_READ(f);
                PREFETCH_READ(args);

                /** run f on NTHREADS_FOR additional threads */
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        async_func_proxy *proxy_arg = proxyArgs + tid;
                        proxy_arg->start = base + tid * chunk_len * width;
                        proxy_arg->len = chunk_len;
                        proxy_arg->tid = (tid + 1);
                        proxy_arg->width = width;
                        proxy_arg->args = args;
                        proxy_arg->function = f;

                        PREFETCH_READ(proxy_arg->start);
                        pthread_create(threads + tid, NULL, async_for_proxy_function, proxyArgs + tid);
                }
                /** run f on this thread */
                PREFETCH_READ(main_thread_base);
                f(main_thread_base, width, chunk_len + chunk_len_remain, args, 0);

                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        pthread_join(threads[tid], NULL);
                }
        }
        return true;
}

void map_proxy(const void *src, size_t src_width, size_t len, void *args, thread_id_t tid)
{
        UNUSED(tid);
        cast(map_args *, mapArgs, args);
        size_t globalStart = (src - mapArgs->src) / src_width;

        PREFETCH_READ(mapArgs->src);
        PREFETCH_READ(mapArgs->args);
        PREFETCH_WRITE(mapArgs->dst);
        mapArgs->map_func(mapArgs->dst + globalStart * mapArgs->dst_width,
                          src,
                          src_width,
                          mapArgs->dst_width,
                          len,
                          mapArgs->args);
}

bool async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                        map_body_func_t f, void *args, threading_hint hint, uint_fast16_t num_threads)
{
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(src_width)
        ERROR_IF_NULL(dst_width)
        ERROR_IF_NULL(f)

        PREFETCH_READ(f);
        PREFETCH_WRITE(dst);

        map_args mapArgs = {.args = args, .map_func = f, .dst = dst, .dst_width = dst_width, .src = src};

        return foreach((void *) src, src_width, len, &map_proxy, &mapArgs, hint, num_threads);
}

void int_async_gather(const void *start, size_t width, size_t len, void *args, thread_id_t tid)
{
        UNUSED(tid);
        cast(gather_scatter_args *, gather_args, args);
        size_t global_index_start = (start - gather_args->dst) / width;

        PREFETCH_WRITE(gather_args->dst);
        PREFETCH_WRITE(gather_args->idx);
        PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;
                memcpy(gather_args->dst + global_index_cur * width,
                       gather_args->src + gather_args->idx[global_index_cur] * width,
                       width);

                bool has_next = (next_i < len);
                PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
                PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
                PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * width : NULL);
        }
}

bool sync_gather(void *dst, const void *src, size_t width, const size_t *idx,
                     size_t dst_src_len)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(width)

        PREFETCH_READ(src);
        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);

        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);
        PREFETCH_READ((dst_src_len > 0) ? src + idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < dst_src_len; next_i = ++i + 1) {
                memcpy(dst + i * width, src + idx[i] * width, width);

                bool has_next = (next_i < dst_src_len);
                PREFETCH_READ(has_next ? idx + next_i : NULL);
                PREFETCH_READ(has_next ? src + idx[next_i] * width : NULL);
                PREFETCH_WRITE(has_next ? dst + next_i * width : NULL);
        }

        return true;
}

bool async_gather(void *dst, const void *src, size_t width, const size_t *idx,
                      size_t dst_src_len, uint_fast16_t num_threads)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(width)

        PREFETCH_READ(src);
        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);

        gather_scatter_args args = {.idx           = idx, .src           = src, .dst           = dst,};
        return async_for(dst, width, dst_src_len, int_async_gather, &args, num_threads);
}

bool sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx,
                         size_t num)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(src_width)

        PREFETCH_READ(src);
        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);

        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);
        PREFETCH_READ(num > 0 ? src + idx[0] * src_width : NULL);

        for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
                const void *ptr = src + idx[i] * src_width;
                size_t adr = (size_t) ptr;
                memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

                bool has_next = (next_i < num);
                PREFETCH_READ(has_next ? idx + next_i : NULL);
                PREFETCH_READ(has_next ? src + idx[next_i] * src_width : NULL);
                PREFETCH_WRITE(has_next ? dst + next_i * sizeof(void *) : NULL);
        }
        return true;
}

void async_gather_adr_func(const void *start, size_t width, size_t len, void *args, thread_id_t tid)
{
        UNUSED(tid);
        cast(gather_scatter_args *, gather_args, args);

        PREFETCH_READ(gather_args->idx);
        PREFETCH_WRITE(gather_args->dst);
        PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

        size_t global_index_start = (start - gather_args->dst) / width;
        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;
                const void *ptr = gather_args->src + gather_args->idx[global_index_cur] * width;
                size_t adr = (size_t) ptr;
                memcpy(gather_args->dst + global_index_cur * sizeof(void *), &adr, sizeof(size_t));

                bool has_next = (next_i < len);
                PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
                PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
                PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * sizeof(void *) : NULL);
        }
}

bool int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx,
                                   size_t num, uint_fast16_t num_threads)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(src_width)

        PREFETCH_READ(src);
        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);

        gather_scatter_args args = {.idx = idx, .src = src, .dst = dst};
        return async_for(dst, src_width, num, async_gather_adr_func, &args, num_threads);
}

void async_scatter(const void *start, size_t width, size_t len, void *args, thread_id_t tid)
{
        UNUSED(tid);
        cast(gather_scatter_args *, scatter_args, args);

        PREFETCH_READ(scatter_args->idx);
        PREFETCH_READ(scatter_args->src);
        PREFETCH_WRITE((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

        size_t global_index_start = (start - scatter_args->dst) / width;
        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;

                memcpy(scatter_args->dst + scatter_args->idx[global_index_cur] * width,
                       scatter_args->src + global_index_cur * width,
                       width);

                bool has_next = (next_i < len);
                PREFETCH_READ(has_next ? scatter_args->idx + global_index_next : NULL);
                PREFETCH_READ(has_next ? scatter_args->src + global_index_next * width : NULL);
                PREFETCH_WRITE(has_next ? scatter_args->dst + scatter_args->idx[global_index_next] * width : NULL);
        }
}

bool sync_scatter(void *dst, const void *src, size_t width, const size_t *idx,
                      size_t num)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(width)

        PREFETCH_READ(idx);
        PREFETCH_READ(src);
        PREFETCH_WRITE((num > 0) ? dst + idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
                memcpy(dst + idx[i] * width, src + i * width, width);

                bool has_next = (next_i < num);
                PREFETCH_READ(has_next ? idx + next_i : NULL);
                PREFETCH_READ(has_next ? src + next_i * width : NULL);
                PREFETCH_WRITE(has_next ? dst + idx[next_i] * width : NULL);
        }
        return true;
}

bool sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                           uint_fast16_t num_threads)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(width)

        PREFETCH_READ(src);
        PREFETCH_READ(idx);
        PREFETCH_WRITE(dst);

        gather_scatter_args args = {.idx = idx, .src = src, .dst = dst};
        return async_for(dst, width, num, async_scatter, &args, num_threads);
}

bool sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                      const size_t *src_idx, size_t idx_len)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NULL(dst_idx)
        ERROR_IF_NULL(src_idx)
        ERROR_IF_NULL(width)

        bool has_first = (idx_len > 0);
        PREFETCH_READ(src_idx);
        PREFETCH_READ(dst_idx);
        PREFETCH_READ(has_first ? src + src_idx[0] * width : NULL);
        PREFETCH_WRITE(has_first ? dst + dst_idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1) {
                memcpy(dst + dst_idx[i] * width, src + src_idx[i] * width, width);

                bool has_next = (next_i < idx_len);
                PREFETCH_READ(has_next ? src_idx + next_i : NULL);
                PREFETCH_READ(has_next ? dst_idx + next_i : NULL);
                PREFETCH_READ(has_next ? src + src_idx[next_i] * width : NULL);
                PREFETCH_WRITE(has_next ? dst + dst_idx[next_i] * width : NULL);
        }

        return true;
}

bool async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                       const size_t *src_idx, size_t idx_len)
{
        UNUSED(dst);
        UNUSED(src);
        UNUSED(width);
        UNUSED(dst_idx);
        UNUSED(src_idx);
        UNUSED(idx_len);
        NOT_IMPLEMENTED
}

bool int_sync_filter_late(size_t *positions, size_t *num_positions, const void *source,
                              size_t width, size_t length, pred_func_t predicate,
                              void *arguments)
{
        ERROR_IF_NULL(positions);
        ERROR_IF_NULL(num_positions);
        ERROR_IF_NULL(source);
        ERROR_IF_NULL(width);
        ERROR_IF_NULL(length);
        ERROR_IF_NULL(predicate);

        predicate(positions, num_positions, source, width, length, arguments, 0);

        return true;
}

void *int_sync_filter_procy_func(void *args)
{
        cast(filter_arg  *, proxy_arg, args);
        proxy_arg->pred(proxy_arg->src_positions,
                        &proxy_arg->num_positions,
                        proxy_arg->start,
                        proxy_arg->width,
                        proxy_arg->len,
                        proxy_arg->args,
                        proxy_arg->position_offset_to_add);
        return NULL;
}

bool async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                           pred_func_t pred, void *args, size_t num_threads)
{
        ERROR_IF_NULL(pos);
        ERROR_IF_NULL(num_pos);
        ERROR_IF_NULL(src);
        ERROR_IF_NULL(width);
        ERROR_IF_NULL(pred);

        if (UNLIKELY(len == 0)) {
                *num_pos = 0;
                return true;
        }

        uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

        pthread_t threads[num_threads];
        filter_arg thread_args[num_thread];

        register size_t chunk_len = len / num_thread;
        size_t chunk_len_remain = len % num_thread;
        size_t main_position_offset_to_add = num_threads * chunk_len;
        const void *main_thread_base = src + main_position_offset_to_add * width;

        PREFETCH_READ(pred);
        PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        if (LIKELY(chunk_len > 0)) {
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        filter_arg *arg = thread_args + tid;
                        arg->num_positions = 0;
                        arg->src_positions = MALLOC(chunk_len * sizeof(size_t));
                        arg->position_offset_to_add = tid * chunk_len;
                        arg->start = src + arg->position_offset_to_add * width;
                        arg->len = chunk_len;
                        arg->width = width;
                        arg->args = args;
                        arg->pred = pred;

                        PREFETCH_READ(arg->start);
                        pthread_create(threads + tid, NULL, int_sync_filter_procy_func, arg);
                }
        }
        /** run f on this thread */
        PREFETCH_READ(main_thread_base);
        size_t main_chunk_len = chunk_len + chunk_len_remain;
        size_t *main_src_positions = MALLOC(main_chunk_len * sizeof(size_t));
        size_t main_num_positions = 0;

        pred(main_src_positions,
             &main_num_positions,
             main_thread_base,
             width,
             main_chunk_len,
             args,
             main_position_offset_to_add);

        size_t total_num_matching_positions = 0;

        if (LIKELY(chunk_len > 0)) {
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        pthread_join(threads[tid], NULL);
                        const filter_arg *thread_arg = (thread_args + tid);
                        if (thread_arg->num_positions > 0) {
                                memcpy(pos + total_num_matching_positions,
                                       thread_arg->src_positions,
                                       thread_arg->num_positions * sizeof(size_t));
                                total_num_matching_positions += thread_arg->num_positions;
                        }
                        free(thread_args[tid].src_positions);
                }
        }

        if (LIKELY(main_num_positions > 0)) {
                memcpy(pos + total_num_matching_positions, main_src_positions, main_num_positions * sizeof(size_t));
                total_num_matching_positions += main_num_positions;
        }
        free(main_src_positions);

        *num_pos = total_num_matching_positions;

        return true;
}

bool async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                            size_t len, pred_func_t pred, void *args)
{
        ERROR_IF_NULL(result);
        ERROR_IF_NULL(result_size);
        ERROR_IF_NULL(src);
        ERROR_IF_NULL(width);
        ERROR_IF_NULL(len);
        ERROR_IF_NULL(pred);

        size_t num_matching_positions;
        size_t *matching_positions = MALLOC(len * sizeof(size_t));

        pred(matching_positions, &num_matching_positions, src, width, len, args, 0);

        gather(result, src, width, matching_positions, num_matching_positions, THREADING_HINT_SINGLE, 0);
        *result_size = num_matching_positions;

        free(matching_positions);

        return true;
}

bool int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                size_t len, pred_func_t pred, void *args, uint_fast16_t num_threads)
{
        ERROR_IF_NULL(result);
        ERROR_IF_NULL(result_size);
        ERROR_IF_NULL(src);
        ERROR_IF_NULL(width);
        ERROR_IF_NULL(len);
        ERROR_IF_NULL(pred);

        uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

        pthread_t threads[num_threads];
        filter_arg thread_args[num_thread];

        register size_t chunk_len = len / num_thread;
        size_t chunk_len_remain = len % num_thread;
        size_t main_position_offset_to_add = num_threads * chunk_len;
        const void *main_thread_base = src + main_position_offset_to_add * width;

        PREFETCH_READ(pred);
        PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                filter_arg *arg = thread_args + tid;
                arg->num_positions = 0;
                arg->src_positions = MALLOC(chunk_len * sizeof(size_t));
                arg->position_offset_to_add = tid * chunk_len;
                arg->start = src + arg->position_offset_to_add * width;
                arg->len = chunk_len;
                arg->width = width;
                arg->args = args;
                arg->pred = pred;

                PREFETCH_READ(arg->start);
                pthread_create(threads + tid, NULL, int_sync_filter_procy_func, arg);
        }
        /** run f on this thread */
        PREFETCH_READ(main_thread_base);
        size_t main_chunk_len = chunk_len + chunk_len_remain;
        size_t *main_src_positions = MALLOC(main_chunk_len * sizeof(size_t));
        size_t main_num_positions = 0;

        pred(main_src_positions,
             &main_num_positions,
             main_thread_base,
             width,
             main_chunk_len,
             args,
             main_position_offset_to_add);

        size_t total_num_matching_positions = main_num_positions;
        size_t partial_num_matching_positions = 0;

        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                pthread_join(threads[tid], NULL);
                const filter_arg *thread_arg = (thread_args + tid);
                total_num_matching_positions += thread_arg->num_positions;
                PREFETCH_READ(thread_arg->src_positions);
        }

        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                const filter_arg *thread_arg = (thread_args + tid);

                if (LIKELY(thread_arg->num_positions > 0)) {
                        gather(result + partial_num_matching_positions * width,
                                   src,
                                   width,
                                   thread_arg->src_positions,
                                   thread_arg->num_positions,
                                   THREADING_HINT_MULTI,
                                   num_threads);
                }

                partial_num_matching_positions += thread_arg->num_positions;
                free(thread_arg->src_positions);
        }

        if (LIKELY(main_num_positions > 0)) {
                gather(result + partial_num_matching_positions * width,
                           src,
                           width,
                           main_src_positions,
                           main_num_positions,
                           THREADING_HINT_MULTI,
                           num_threads);
        }
        free(main_src_positions);

        *result_size = total_num_matching_positions;

        return true;
}