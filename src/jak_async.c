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

#include <jak_async.h>

void *jak_async_for_proxy_function(void *args)
{
        JAK_cast(jak_async_func_proxy *, proxy_arg, args);
        proxy_arg->function(proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args, proxy_arg->tid);
        return NULL;
}

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
             jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                 jak_threading_hint hint, uint_fast16_t num_threads);

bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                 const size_t *src_idx, size_t idxLen, jak_threading_hint hint);

bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                      jak_pred_func_t pred, void *args, jak_threading_hint hint,
                      uint_fast16_t num_threads);

bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                     jak_pred_func_t pred, void *args, jak_threading_hint hint, size_t num_threads);

bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                  void *args);

bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                   void *args, uint_fast16_t num_threads);

bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                        jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads);

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
             jak_threading_hint hint, uint_fast16_t num_threads)
{
        JAK_ASYNC_MATCH(jak_sync_for(base, width, len, f, args),
                        jak_async_for(base, width, len, f, args, num_threads))
}

bool jak_map(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
             jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads)
{
        return jak_async_map_exec(dst, src, src_width, len, dst_width, f, args, hint, num_threads);
}

bool jak_gather(void *dst, const void *src, size_t width, const size_t *idx, size_t dst_src_len,
                jak_threading_hint hint, uint_fast16_t num_threads)
{
        JAK_ASYNC_MATCH(jak_sync_gather(dst, src, width, idx, dst_src_len),
                        jak_async_gather(dst, src, width, idx, dst_src_len, num_threads))
}

bool jak_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx, size_t num,
                    jak_threading_hint hint, uint_fast16_t num_threads)
{
        JAK_ASYNC_MATCH(jak_sync_gather_adr(dst, src, src_width, idx, num),
                        jak_int_async_gather_adr_func(dst, src, src_width, idx, num, num_threads))
}

bool jak_scatter(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                 jak_threading_hint hint, uint_fast16_t num_threads)
{
        JAK_ASYNC_MATCH(jak_sync_scatter(dst, src, width, idx, num),
                        jak_sync_scatter_func(dst, src, width, idx, num, num_threads))
}

bool jak_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                 const size_t *src_idx, size_t idx_len, jak_threading_hint hint)
{
        JAK_ASYNC_MATCH(jak_sync_shuffle(dst, src, width, dst_idx, src_idx, idx_len),
                        jak_async_shuffle(dst, src, width, dst_idx, src_idx, idx_len))
}

bool jak_filter_early(void *result, size_t *result_size, const void *src, size_t width, size_t len,
                      jak_pred_func_t pred, void *args, jak_threading_hint hint,
                      uint_fast16_t num_threads)
{
        JAK_ASYNC_MATCH(jak_async_filter_early(result, result_size, src, width, len, pred, args),
                        jak_int_async_filter_early(result, result_size, src, width, len, pred, args, num_threads))
}

bool jak_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                     jak_pred_func_t pred, void *args, jak_threading_hint hint, size_t num_threads)
{
        JAK_ASYNC_MATCH(jak_int_sync_filter_late(pos, num_pos, src, width, len, pred, args),
                        jak_async_filter_late(pos, num_pos, src, width, len, pred, args, num_threads))
}

bool jak_sync_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                  void *args)
{
        JAK_ERROR_IF_NULL(base)
        JAK_ERROR_IF_NULL(width)
        JAK_ERROR_IF_NULL(len)
        f(base, width, len, args, 0);
        return true;
}

bool jak_async_for(const void *base, size_t width, size_t len, jak_for_body_func_t f,
                   void *args, uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(base)
        JAK_ERROR_IF_NULL(width)

        if (len > 0) {
                uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */
                pthread_t threads[num_threads];
                jak_async_func_proxy proxyArgs[num_thread];
                size_t chunk_len = len / num_thread;
                size_t chunk_len_remain = len % num_thread;
                const void *main_thread_base = base + num_threads * chunk_len * width;

                JAK_PREFETCH_READ(f);
                JAK_PREFETCH_READ(args);

                /** run f on NTHREADS_FOR additional threads */
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        jak_async_func_proxy *proxy_arg = proxyArgs + tid;
                        proxy_arg->start = base + tid * chunk_len * width;
                        proxy_arg->len = chunk_len;
                        proxy_arg->tid = (tid + 1);
                        proxy_arg->width = width;
                        proxy_arg->args = args;
                        proxy_arg->function = f;

                        JAK_PREFETCH_READ(proxy_arg->start);
                        pthread_create(threads + tid, NULL, jak_async_for_proxy_function, proxyArgs + tid);
                }
                /** run f on this thread */
                JAK_PREFETCH_READ(main_thread_base);
                f(main_thread_base, width, chunk_len + chunk_len_remain, args, 0);

                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        pthread_join(threads[tid], NULL);
                }
        }
        return true;
}

void jak_map_proxy(const void *src, size_t src_width, size_t len, void *args, jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_cast(jak_map_args *, mapArgs, args);
        size_t globalStart = (src - mapArgs->src) / src_width;

        JAK_PREFETCH_READ(mapArgs->src);
        JAK_PREFETCH_READ(mapArgs->args);
        JAK_PREFETCH_WRITE(mapArgs->dst);
        mapArgs->map_func(mapArgs->dst + globalStart * mapArgs->dst_width,
                          src,
                          src_width,
                          mapArgs->dst_width,
                          len,
                          mapArgs->args);
}

bool jak_async_map_exec(void *dst, const void *src, size_t src_width, size_t len, size_t dst_width,
                        jak_map_body_func_t f, void *args, jak_threading_hint hint, uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(src_width)
        JAK_ERROR_IF_NULL(dst_width)
        JAK_ERROR_IF_NULL(f)

        JAK_PREFETCH_READ(f);
        JAK_PREFETCH_WRITE(dst);

        jak_map_args mapArgs = {.args = args, .map_func = f, .dst = dst, .dst_width = dst_width, .src = src};

        return jak_for((void *) src, src_width, len, &jak_map_proxy, &mapArgs, hint, num_threads);
}

void jak_int_async_gather(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_cast(jak_gather_scatter_args *, gather_args, args);
        size_t global_index_start = (start - gather_args->dst) / width;

        JAK_PREFETCH_WRITE(gather_args->dst);
        JAK_PREFETCH_WRITE(gather_args->idx);
        JAK_PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;
                memcpy(gather_args->dst + global_index_cur * width,
                       gather_args->src + gather_args->idx[global_index_cur] * width,
                       width);

                bool has_next = (next_i < len);
                JAK_PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
                JAK_PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * width : NULL);
        }
}

bool jak_sync_gather(void *dst, const void *src, size_t width, const size_t *idx,
                     size_t dst_src_len)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(width)

        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);

        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);
        JAK_PREFETCH_READ((dst_src_len > 0) ? src + idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < dst_src_len; next_i = ++i + 1) {
                memcpy(dst + i * width, src + idx[i] * width, width);

                bool has_next = (next_i < dst_src_len);
                JAK_PREFETCH_READ(has_next ? idx + next_i : NULL);
                JAK_PREFETCH_READ(has_next ? src + idx[next_i] * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? dst + next_i * width : NULL);
        }

        return true;
}

bool jak_async_gather(void *dst, const void *src, size_t width, const size_t *idx,
                      size_t dst_src_len, uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(width)

        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);

        jak_gather_scatter_args args = {.idx           = idx, .src           = src, .dst           = dst,};
        return jak_async_for(dst, width, dst_src_len, jak_int_async_gather, &args, num_threads);
}

bool jak_sync_gather_adr(void *dst, const void *src, size_t src_width, const size_t *idx,
                         size_t num)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(src_width)

        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);

        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);
        JAK_PREFETCH_READ(num > 0 ? src + idx[0] * src_width : NULL);

        for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
                const void *ptr = src + idx[i] * src_width;
                size_t adr = (size_t) ptr;
                memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

                bool has_next = (next_i < num);
                JAK_PREFETCH_READ(has_next ? idx + next_i : NULL);
                JAK_PREFETCH_READ(has_next ? src + idx[next_i] * src_width : NULL);
                JAK_PREFETCH_WRITE(has_next ? dst + next_i * sizeof(void *) : NULL);
        }
        return true;
}

void jak_async_gather_adr_func(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_cast(jak_gather_scatter_args *, gather_args, args);

        JAK_PREFETCH_READ(gather_args->idx);
        JAK_PREFETCH_WRITE(gather_args->dst);
        JAK_PREFETCH_READ((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

        size_t global_index_start = (start - gather_args->dst) / width;
        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;
                const void *ptr = gather_args->src + gather_args->idx[global_index_cur] * width;
                size_t adr = (size_t) ptr;
                memcpy(gather_args->dst + global_index_cur * sizeof(void *), &adr, sizeof(size_t));

                bool has_next = (next_i < len);
                JAK_PREFETCH_READ(has_next ? gather_args->idx + global_index_next : NULL);
                JAK_PREFETCH_READ(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? gather_args->dst + global_index_next * sizeof(void *) : NULL);
        }
}

bool jak_int_async_gather_adr_func(void *dst, const void *src, size_t src_width, const size_t *idx,
                                   size_t num, uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(src_width)

        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);

        jak_gather_scatter_args args = {.idx = idx, .src = src, .dst = dst};
        return jak_async_for(dst, src_width, num, jak_async_gather_adr_func, &args, num_threads);
}

void jak_async_scatter(const void *start, size_t width, size_t len, void *args, jak_thread_id_t tid)
{
        JAK_UNUSED(tid);
        JAK_cast(jak_gather_scatter_args *, scatter_args, args);

        JAK_PREFETCH_READ(scatter_args->idx);
        JAK_PREFETCH_READ(scatter_args->src);
        JAK_PREFETCH_WRITE((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

        size_t global_index_start = (start - scatter_args->dst) / width;
        for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
                size_t global_index_cur = global_index_start + i;
                size_t global_index_next = global_index_start + next_i;

                memcpy(scatter_args->dst + scatter_args->idx[global_index_cur] * width,
                       scatter_args->src + global_index_cur * width,
                       width);

                bool has_next = (next_i < len);
                JAK_PREFETCH_READ(has_next ? scatter_args->idx + global_index_next : NULL);
                JAK_PREFETCH_READ(has_next ? scatter_args->src + global_index_next * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? scatter_args->dst + scatter_args->idx[global_index_next] * width : NULL);
        }
}

bool jak_sync_scatter(void *dst, const void *src, size_t width, const size_t *idx,
                      size_t num)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(width)

        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_WRITE((num > 0) ? dst + idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
                memcpy(dst + idx[i] * width, src + i * width, width);

                bool has_next = (next_i < num);
                JAK_PREFETCH_READ(has_next ? idx + next_i : NULL);
                JAK_PREFETCH_READ(has_next ? src + next_i * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? dst + idx[next_i] * width : NULL);
        }
        return true;
}

bool jak_sync_scatter_func(void *dst, const void *src, size_t width, const size_t *idx, size_t num,
                           uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(width)

        JAK_PREFETCH_READ(src);
        JAK_PREFETCH_READ(idx);
        JAK_PREFETCH_WRITE(dst);

        jak_gather_scatter_args args = {.idx = idx, .src = src, .dst = dst};
        return jak_async_for(dst, width, num, jak_async_scatter, &args, num_threads);
}

bool jak_sync_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                      const size_t *src_idx, size_t idx_len)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_ERROR_IF_NULL(dst_idx)
        JAK_ERROR_IF_NULL(src_idx)
        JAK_ERROR_IF_NULL(width)

        bool has_first = (idx_len > 0);
        JAK_PREFETCH_READ(src_idx);
        JAK_PREFETCH_READ(dst_idx);
        JAK_PREFETCH_READ(has_first ? src + src_idx[0] * width : NULL);
        JAK_PREFETCH_WRITE(has_first ? dst + dst_idx[0] * width : NULL);

        for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1) {
                memcpy(dst + dst_idx[i] * width, src + src_idx[i] * width, width);

                bool has_next = (next_i < idx_len);
                JAK_PREFETCH_READ(has_next ? src_idx + next_i : NULL);
                JAK_PREFETCH_READ(has_next ? dst_idx + next_i : NULL);
                JAK_PREFETCH_READ(has_next ? src + src_idx[next_i] * width : NULL);
                JAK_PREFETCH_WRITE(has_next ? dst + dst_idx[next_i] * width : NULL);
        }

        return true;
}

bool jak_async_shuffle(void *dst, const void *src, size_t width, const size_t *dst_idx,
                       const size_t *src_idx, size_t idx_len)
{
        JAK_UNUSED(dst);
        JAK_UNUSED(src);
        JAK_UNUSED(width);
        JAK_UNUSED(dst_idx);
        JAK_UNUSED(src_idx);
        JAK_UNUSED(idx_len);
        JAK_NOT_IMPLEMENTED
}

bool jak_int_sync_filter_late(size_t *positions, size_t *num_positions, const void *source,
                              size_t width, size_t length, jak_pred_func_t predicate,
                              void *arguments)
{
        JAK_ERROR_IF_NULL(positions);
        JAK_ERROR_IF_NULL(num_positions);
        JAK_ERROR_IF_NULL(source);
        JAK_ERROR_IF_NULL(width);
        JAK_ERROR_IF_NULL(length);
        JAK_ERROR_IF_NULL(predicate);

        predicate(positions, num_positions, source, width, length, arguments, 0);

        return true;
}

void *jak_int_sync_filter_procy_func(void *args)
{
        JAK_cast(jak_filter_arg  *, proxy_arg, args);
        proxy_arg->pred(proxy_arg->src_positions,
                        &proxy_arg->num_positions,
                        proxy_arg->start,
                        proxy_arg->width,
                        proxy_arg->len,
                        proxy_arg->args,
                        proxy_arg->position_offset_to_add);
        return NULL;
}

bool jak_async_filter_late(size_t *pos, size_t *num_pos, const void *src, size_t width, size_t len,
                           jak_pred_func_t pred, void *args, size_t num_threads)
{
        JAK_ERROR_IF_NULL(pos);
        JAK_ERROR_IF_NULL(num_pos);
        JAK_ERROR_IF_NULL(src);
        JAK_ERROR_IF_NULL(width);
        JAK_ERROR_IF_NULL(pred);

        if (JAK_UNLIKELY(len == 0)) {
                *num_pos = 0;
                return true;
        }

        uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

        pthread_t threads[num_threads];
        jak_filter_arg thread_args[num_thread];

        register size_t chunk_len = len / num_thread;
        size_t chunk_len_remain = len % num_thread;
        size_t main_position_offset_to_add = num_threads * chunk_len;
        const void *main_thread_base = src + main_position_offset_to_add * width;

        JAK_PREFETCH_READ(pred);
        JAK_PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        if (JAK_LIKELY(chunk_len > 0)) {
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        jak_filter_arg *arg = thread_args + tid;
                        arg->num_positions = 0;
                        arg->src_positions = JAK_MALLOC(chunk_len * sizeof(size_t));
                        arg->position_offset_to_add = tid * chunk_len;
                        arg->start = src + arg->position_offset_to_add * width;
                        arg->len = chunk_len;
                        arg->width = width;
                        arg->args = args;
                        arg->pred = pred;

                        JAK_PREFETCH_READ(arg->start);
                        pthread_create(threads + tid, NULL, jak_int_sync_filter_procy_func, arg);
                }
        }
        /** run f on this thread */
        JAK_PREFETCH_READ(main_thread_base);
        size_t main_chunk_len = chunk_len + chunk_len_remain;
        size_t *main_src_positions = JAK_MALLOC(main_chunk_len * sizeof(size_t));
        size_t main_num_positions = 0;

        pred(main_src_positions,
             &main_num_positions,
             main_thread_base,
             width,
             main_chunk_len,
             args,
             main_position_offset_to_add);

        size_t total_num_matching_positions = 0;

        if (JAK_LIKELY(chunk_len > 0)) {
                for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                        pthread_join(threads[tid], NULL);
                        const jak_filter_arg *thread_arg = (thread_args + tid);
                        if (thread_arg->num_positions > 0) {
                                memcpy(pos + total_num_matching_positions,
                                       thread_arg->src_positions,
                                       thread_arg->num_positions * sizeof(size_t));
                                total_num_matching_positions += thread_arg->num_positions;
                        }
                        free(thread_args[tid].src_positions);
                }
        }

        if (JAK_LIKELY(main_num_positions > 0)) {
                memcpy(pos + total_num_matching_positions, main_src_positions, main_num_positions * sizeof(size_t));
                total_num_matching_positions += main_num_positions;
        }
        free(main_src_positions);

        *num_pos = total_num_matching_positions;

        return true;
}

bool jak_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                            size_t len, jak_pred_func_t pred, void *args)
{
        JAK_ERROR_IF_NULL(result);
        JAK_ERROR_IF_NULL(result_size);
        JAK_ERROR_IF_NULL(src);
        JAK_ERROR_IF_NULL(width);
        JAK_ERROR_IF_NULL(len);
        JAK_ERROR_IF_NULL(pred);

        size_t num_matching_positions;
        size_t *matching_positions = JAK_MALLOC(len * sizeof(size_t));

        pred(matching_positions, &num_matching_positions, src, width, len, args, 0);

        jak_gather(result, src, width, matching_positions, num_matching_positions, JAK_THREADING_HINT_SINGLE, 0);
        *result_size = num_matching_positions;

        free(matching_positions);

        return true;
}

bool jak_int_async_filter_early(void *result, size_t *result_size, const void *src, size_t width,
                                size_t len, jak_pred_func_t pred, void *args, uint_fast16_t num_threads)
{
        JAK_ERROR_IF_NULL(result);
        JAK_ERROR_IF_NULL(result_size);
        JAK_ERROR_IF_NULL(src);
        JAK_ERROR_IF_NULL(width);
        JAK_ERROR_IF_NULL(len);
        JAK_ERROR_IF_NULL(pred);

        uint_fast16_t num_thread = num_threads + 1; /** +1 since one is this thread */

        pthread_t threads[num_threads];
        jak_filter_arg thread_args[num_thread];

        register size_t chunk_len = len / num_thread;
        size_t chunk_len_remain = len % num_thread;
        size_t main_position_offset_to_add = num_threads * chunk_len;
        const void *main_thread_base = src + main_position_offset_to_add * width;

        JAK_PREFETCH_READ(pred);
        JAK_PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                jak_filter_arg *arg = thread_args + tid;
                arg->num_positions = 0;
                arg->src_positions = JAK_MALLOC(chunk_len * sizeof(size_t));
                arg->position_offset_to_add = tid * chunk_len;
                arg->start = src + arg->position_offset_to_add * width;
                arg->len = chunk_len;
                arg->width = width;
                arg->args = args;
                arg->pred = pred;

                JAK_PREFETCH_READ(arg->start);
                pthread_create(threads + tid, NULL, jak_int_sync_filter_procy_func, arg);
        }
        /** run f on this thread */
        JAK_PREFETCH_READ(main_thread_base);
        size_t main_chunk_len = chunk_len + chunk_len_remain;
        size_t *main_src_positions = JAK_MALLOC(main_chunk_len * sizeof(size_t));
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
                const jak_filter_arg *thread_arg = (thread_args + tid);
                total_num_matching_positions += thread_arg->num_positions;
                JAK_PREFETCH_READ(thread_arg->src_positions);
        }

        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
                const jak_filter_arg *thread_arg = (thread_args + tid);

                if (JAK_LIKELY(thread_arg->num_positions > 0)) {
                        jak_gather(result + partial_num_matching_positions * width,
                                   src,
                                   width,
                                   thread_arg->src_positions,
                                   thread_arg->num_positions,
                                   JAK_THREADING_HINT_MULTI,
                                   num_threads);
                }

                partial_num_matching_positions += thread_arg->num_positions;
                free(thread_arg->src_positions);
        }

        if (JAK_LIKELY(main_num_positions > 0)) {
                jak_gather(result + partial_num_matching_positions * width,
                           src,
                           width,
                           main_src_positions,
                           main_num_positions,
                           JAK_THREADING_HINT_MULTI,
                           num_threads);
        }
        free(main_src_positions);

        *result_size = total_num_matching_positions;

        return true;
}