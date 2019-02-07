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

#ifndef CARBON_PARALLEL_H
#define CARBON_PARALLEL_H

#include "carbon-common.h"
#include "stdlib.h"

CARBON_BEGIN_DECL

#define PARALLEL_MSG_UNKNOWN_HINT "Unknown threading hint"

typedef uint_fast16_t thread_id_t;

typedef void (*ForFunctionBody)(const void *restrict start, size_t width, size_t len, void *restrict args, thread_id_t tid);

typedef void (*MapFunctionBody)(void *restrict dst, const void *restrict src, size_t srcWidth, size_t dstWidth,
                                size_t len, void *restrict args);

typedef void (*Predicate)(size_t *matchingPositions, size_t *numMatchingPositions, const void *restrict src,
                            size_t width, size_t len, void *restrict args, size_t positionOffsetToAdd);

typedef enum ThreadingHint
{
    ThreadingHint_Single,
    CARBON_PARALLEL_THREAD_HINT_MULTI
} ThreadingHint;

typedef struct FunctionProxy
{
    ForFunctionBody function;
    const void *restrict start;
    size_t width;
    size_t len;
    thread_id_t tid;
    void *args;
} FunctionProxy;

inline static void *forProxyFunction(void *restrict args)
{
    CARBON_CAST(FunctionProxy *restrict, proxyArg, args);
    proxyArg->function(proxyArg->start, proxyArg->width, proxyArg->len, proxyArg->args, proxyArg->tid);
    return NULL;
}

#define PARALLEL_ERROR(msg, retval)                                                                                    \
{                                                                                                                      \
    perror(msg);                                                                                                       \
    return retval;                                                                                                     \
}

#define PARALLEL_MATCH(forSingle, forMulti)                                                                            \
{                                                                                                                      \
    if (CARBON_BRANCH_LIKELY(hint == CARBON_PARALLEL_THREAD_HINT_MULTI)) {                                                           \
        return (forMulti);                                                                                             \
    } else if (hint == ThreadingHint_Single) {                                                                         \
        return (forSingle);                                                                                            \
    } else PARALLEL_ERROR(PARALLEL_MSG_UNKNOWN_HINT, false);                                                           \
}

inline static bool
carbon_parallel_for(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                              void *restrict args, ThreadingHint hint, uint_fast16_t num_threads);

inline static bool
ParallelMap(void *restrict dst,
                              const void *restrict src,
                              size_t srcWidth,
                              size_t len,
                              size_t dstWidth,
                              MapFunctionBody f,
                              void *restrict args,
                              ThreadingHint hint,
                              uint_fast16_t num_threads);

inline static bool
ParallelGather(void *restrict dst,
                                 const void *restrict src,
                                 size_t width,
                                 const size_t *restrict idx,
                                 size_t dstSrcLen,
                                 ThreadingHint hint,
                                 uint_fast16_t num_threads);

inline static bool
ParallelGatherAddress(void *restrict dst,
                                     const void *restrict src,
                                     size_t srcWidth,
                                     const size_t *restrict idx,
                                     size_t num,
                                     ThreadingHint hint,
                                     uint_fast16_t num_threads);

inline static bool
ParallelScatter(void *restrict dst,
                                  const void *restrict src,
                                  size_t width,
                                  const size_t *restrict idx,
                                  size_t num,
                                  ThreadingHint hint,
                                  uint_fast16_t num_threads);

inline static bool
ParallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idxLen, ThreadingHint hint);

inline static bool
ParallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                       const void *restrict src,
                                       size_t width, size_t len, Predicate pred, void *restrict args,
                                       ThreadingHint hint, uint_fast16_t num_threads);

inline static bool
ParallelFilterLate(size_t *restrict pos, size_t *restrict numPos, const void *restrict src,
                                      size_t width, size_t len, Predicate pred, void *restrict args,
                                      ThreadingHint hint, size_t num_threads);

inline static bool
sequentialFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                   void *restrict args);
inline static bool
parallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                 void *restrict args, uint_fast16_t num_threads);

inline static bool
map(void *restrict dst,
                        const void *restrict src,
                        size_t srcWidth,
                        size_t len,
                        size_t dstWidth,
                        MapFunctionBody f,
                        void *restrict args,
                        ThreadingHint hint,
                        uint_fast16_t num_threads);

inline static bool
sequentialGather(void *restrict dst, const void *restrict src, size_t width,
                                      const size_t *restrict idx, size_t dstSrcLen);
inline static bool
parallelGather(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t dstSrcLen, uint_fast16_t num_threads);

inline static bool
sequentialGatherAddress(void *restrict dst, const void *restrict src, size_t srcWidth,
                                          const size_t *restrict idx, size_t num);
inline static bool
parallelGather_adr(void *restrict dst, const void *restrict src, size_t srcWidth,
                                        const size_t *restrict idx, size_t num, uint_fast16_t num_threads);

inline static bool
sequentialScatter(void *restrict dst, const void *restrict src, size_t width,
                                       const size_t *restrict idx, size_t num);
inline static bool
parallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                     const size_t *restrict idx, size_t num, uint_fast16_t num_threads);

inline static bool
sequentialShuffle(void *restrict dst, const void *restrict src, size_t width,
                                       const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                       size_t idx_len);

inline static bool
parallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                     const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                     size_t idx_len);

inline static bool
sequentialFilterEarly(void *restrict result, size_t *restrict resultSize,
                                            const void *restrict src, size_t width, size_t len, Predicate pred,
                                            void *restrict args);

inline static bool
parallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                          const void *restrict src, size_t width, size_t len, Predicate pred,
                                          void *restrict args, uint_fast16_t num_threads);

inline static bool
sequentialFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                           const void *restrict src, size_t width, size_t len, Predicate pred,
                                           void *restrict args);

inline static bool
parallelFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                         const void *restrict src, size_t width, size_t len, Predicate pred,
                                         void *restrict args, size_t num_threads);

inline static bool
carbon_parallel_for(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                              void *restrict args, ThreadingHint hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(sequentialFor(base, width, len, f, args),
                   parallelFor(base, width, len, f, args, num_threads))
}

inline static bool
ParallelMap(void *restrict dst,
                              const void *restrict src,
                              size_t srcWidth,
                              size_t len,
                              size_t dstWidth,
                              MapFunctionBody f,
                              void *restrict args,
                              ThreadingHint hint,
                              uint_fast16_t num_threads)
{
    return map(dst, src, srcWidth, len, dstWidth, f, args, hint, num_threads);
}

inline static bool
ParallelGather(void *restrict dst,
                                 const void *restrict src,
                                 size_t width,
                                 const size_t *restrict idx,
                                 size_t dst_src_len,
                                 ThreadingHint hint,
                                 uint_fast16_t num_threads)
{
    PARALLEL_MATCH(sequentialGather(dst, src, width, idx, dst_src_len),
                   parallelGather(dst, src, width, idx, dst_src_len, num_threads))
}

inline static bool
ParallelGatherAddress(void *restrict dst,
                                     const void *restrict src,
                                     size_t srcWidth,
                                     const size_t *restrict idx,
                                     size_t num,
                                     ThreadingHint hint,
                                     uint_fast16_t num_threads)
{
    PARALLEL_MATCH(sequentialGatherAddress(dst, src, srcWidth, idx, num),
                   parallelGather_adr(dst, src, srcWidth, idx, num, num_threads))
}

inline static bool
ParallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict idx, size_t num, ThreadingHint hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(sequentialScatter(dst, src, width, idx, num),
                   parallelScatter(dst, src, width, idx, num, num_threads))
}

inline static bool
ParallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idx_len, ThreadingHint hint)
{
    PARALLEL_MATCH(sequentialShuffle(dst, src, width, dstIdx, srcIdx, idx_len),
                   parallelShuffle(dst, src, width, dstIdx, srcIdx, idx_len))
}

inline static bool
ParallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                       const void *restrict src,
                                       size_t width, size_t len, Predicate pred, void *restrict args,
                                       ThreadingHint hint, uint_fast16_t num_threads)
{
    PARALLEL_MATCH(sequentialFilterEarly(result, resultSize, src, width, len, pred, args),
                   parallelFilterEarly(result, resultSize, src, width, len, pred, args, num_threads))
}

inline static bool
ParallelFilterLate(size_t *restrict pos, size_t *restrict numPos, const void *restrict src,
                                      size_t width, size_t len, Predicate pred, void *restrict args,
                                      ThreadingHint hint, size_t num_threads)
{
    PARALLEL_MATCH(sequentialFilterLate(pos, numPos, src, width, len, pred, args),
                   parallelFilterLate(pos, numPos, src, width, len, pred, args, num_threads))
}

inline static bool
sequentialFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                   void *restrict args)
{
    CARBON_NON_NULL_OR_ERROR(base)
    CARBON_NON_NULL_OR_ERROR(width)
    CARBON_NON_NULL_OR_ERROR(len)
    f(base, width, len, args, 0);
    return true;
}

inline static bool
parallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                 void *restrict args, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(base)
    CARBON_NON_NULL_OR_ERROR(width)

    if (len > 0) {
        uint_fast16_t numThread = num_threads + 1; /** +1 since one is this thread */
        pthread_t threads[num_threads];
        FunctionProxy proxyArgs[numThread];
        register size_t chunkLen = len / numThread;
        size_t chunkLenRemain = len % numThread;
        const void *restrict mainThreadBase = base + num_threads * chunkLen * width;

        CARBON_PREFETCH_READ(f);
        CARBON_PREFETCH_READ(args);

        /** run f on NTHREADS_FOR additional threads */
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            FunctionProxy *proxyArg = proxyArgs + tid;
            proxyArg->start = base + tid * chunkLen * width;
            proxyArg->len = chunkLen;
            proxyArg->tid = (tid + 1);
            proxyArg->width = width;
            proxyArg->args = args;
            proxyArg->function = f;

            CARBON_PREFETCH_READ(proxyArg->start);
            pthread_create(threads + tid, NULL, forProxyFunction, proxyArgs + tid);
        }
        /** run f on this thread */
        CARBON_PREFETCH_READ(mainThreadBase);
        f(mainThreadBase, width, chunkLen + chunkLenRemain, args, 0);

        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            pthread_join(threads[tid], NULL);
        }
    }
    return true;
}

typedef struct MapArgs
{
    MapFunctionBody mapFunction;
    void *restrict dst;
    const void *restrict src;
    size_t dstWidth;
    void *args;
} MapArgs;

inline static void
mapProxy(const void *restrict src, size_t srcWidth, size_t len, void *restrict args,
                               thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_CAST(MapArgs *, mapArgs, args);
    size_t globalStart = (src - mapArgs->src) / srcWidth;

    CARBON_PREFETCH_READ(mapArgs->src);
    CARBON_PREFETCH_READ(mapArgs->args);
    CARBON_PREFETCH_WRITE(mapArgs->dst);
    mapArgs->mapFunction(mapArgs->dst + globalStart * mapArgs->dstWidth,
                       src, srcWidth, mapArgs->dstWidth, len, mapArgs->args);
}

inline static bool
map(void *restrict dst, const void *restrict src, size_t srcWidth, size_t len, size_t
                      dstWidth, MapFunctionBody f, void *restrict args, ThreadingHint hint, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(srcWidth)
    CARBON_NON_NULL_OR_ERROR(dstWidth)
    CARBON_NON_NULL_OR_ERROR(f)

    CARBON_PREFETCH_READ(f);
    CARBON_PREFETCH_WRITE(dst);

    MapArgs mapArgs = {
        .args = args,
        .mapFunction = f,
        .dst = dst,
        .dstWidth = dstWidth,
        .src = src
    };

    return carbon_parallel_for((void *restrict) src, srcWidth, len, &mapProxy, &mapArgs, hint, num_threads);
}

typedef struct GatherScatterArgs
{
    const size_t *restrict idx;
    const void *restrict src;
    void *restrict dst;
} GatherScatterArgs;

inline static void
gatherFunction(const void *restrict start, size_t width, size_t len, void *restrict args,
                                 thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_CAST(GatherScatterArgs *, gatherArgs, args);
    size_t globalIndexStart = (start - gatherArgs->dst) / width;

    CARBON_PREFETCH_WRITE(gatherArgs->dst);
    CARBON_PREFETCH_WRITE(gatherArgs->idx);
    CARBON_PREFETCH_READ((len > 0) ? gatherArgs->src + gatherArgs->idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;
        memcpy(gatherArgs->dst + globalIndexCur * width,
               gatherArgs->src + gatherArgs->idx[globalIndexCur] * width,
               width);

        bool has_next = (next_i < len);
        CARBON_PREFETCH_READ(has_next ? gatherArgs->idx + globalIndexNext : NULL);
        CARBON_PREFETCH_READ(has_next ? gatherArgs->src + gatherArgs->idx[globalIndexNext] * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? gatherArgs->dst + globalIndexNext * width : NULL);
    }
}

inline static bool
sequentialGather(void *restrict dst, const void *restrict src, size_t width,
                                      const size_t *restrict idx, size_t dst_src_len)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(width)

    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);

    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);
    CARBON_PREFETCH_READ((dst_src_len > 0) ? src + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < dst_src_len; next_i = ++i + 1) {
        memcpy(dst + i * width, src + idx[i] * width, width);

        bool has_next = (next_i < dst_src_len);
        CARBON_PREFETCH_READ(has_next ? idx + next_i : NULL);
        CARBON_PREFETCH_READ(has_next ? src + idx[next_i] * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? dst + next_i * width : NULL);
    }

    return true;
}

inline static bool
parallelGather(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t dst_src_len, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(width)

    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx           = idx,
        .src           = src,
        .dst           = dst,
    };
    return parallelFor(dst, width, dst_src_len, gatherFunction, &args, num_threads);
}

inline static bool
sequentialGatherAddress(void *restrict dst, const void *restrict src, size_t srcWidth,
                                          const size_t *restrict idx, size_t num)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(srcWidth)

    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);

    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);
    CARBON_PREFETCH_READ(num > 0 ? src + idx[0] * srcWidth : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
        const void *ptr = src + idx[i] * srcWidth;
        size_t adr = (size_t) ptr;
        memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < num);
        CARBON_PREFETCH_READ(has_next ? idx + next_i : NULL);
        CARBON_PREFETCH_READ(has_next ? src + idx[next_i] * srcWidth : NULL);
        CARBON_PREFETCH_WRITE(has_next ? dst + next_i * sizeof(void *) : NULL);
    }
    return true;
}

inline static void
gatherAddressFunc(const void *restrict start, size_t width, size_t len, void *restrict args,
                                     thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_CAST(GatherScatterArgs *, gatherArgs, args);

    CARBON_PREFETCH_READ(gatherArgs->idx);
    CARBON_PREFETCH_WRITE(gatherArgs->dst);
    CARBON_PREFETCH_READ((len > 0) ? gatherArgs->src + gatherArgs->idx[0] * width : NULL);

    size_t globalIndexStart = (start - gatherArgs->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;
        const void *ptr = gatherArgs->src + gatherArgs->idx[globalIndexCur] * width;
        size_t adr = (size_t) ptr;
        memcpy(gatherArgs->dst + globalIndexCur * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < len);
        CARBON_PREFETCH_READ(has_next ? gatherArgs->idx + globalIndexNext : NULL);
        CARBON_PREFETCH_READ(has_next ? gatherArgs->src + gatherArgs->idx[globalIndexNext] * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? gatherArgs->dst + globalIndexNext * sizeof(void *) : NULL);
    }
}

inline static bool
parallelGather_adr(void *restrict dst, const void *restrict src, size_t srcWidth,
                                     const size_t *restrict idx, size_t num, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(srcWidth)

    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return parallelFor(dst, srcWidth, num, gatherAddressFunc, &args, num_threads);
}

inline static void
scatterFunction(const void *restrict start, size_t width, size_t len, void *restrict args,
                                   thread_id_t tid)
{
    CARBON_UNUSED(tid);
    CARBON_CAST(GatherScatterArgs *, scatter_args, args);

    CARBON_PREFETCH_READ(scatter_args->idx);
    CARBON_PREFETCH_READ(scatter_args->src);
    CARBON_PREFETCH_WRITE((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

    size_t globalIndexStart = (start - scatter_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;

        memcpy(scatter_args->dst + scatter_args->idx[globalIndexCur] * width,
               scatter_args->src + globalIndexCur * width,
               width);

        bool has_next = (next_i < len);
        CARBON_PREFETCH_READ(has_next ? scatter_args->idx + globalIndexNext : NULL);
        CARBON_PREFETCH_READ(has_next ? scatter_args->src + globalIndexNext * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? scatter_args->dst + scatter_args->idx[globalIndexNext] * width : NULL);
    }
}

inline static bool
sequentialScatter(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t num)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(width)

    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_WRITE((num > 0) ? dst + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
        memcpy(dst + idx[i] * width, src + i * width, width);

        bool has_next = (next_i < num);
        CARBON_PREFETCH_READ(has_next ? idx + next_i : NULL);
        CARBON_PREFETCH_READ(has_next ? src + next_i * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? dst + idx[next_i] * width : NULL);
    }
    return true;
}

inline static bool
parallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict idx, size_t num, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(idx)
    CARBON_NON_NULL_OR_ERROR(width)

    CARBON_PREFETCH_READ(src);
    CARBON_PREFETCH_READ(idx);
    CARBON_PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return parallelFor(dst, width, num, scatterFunction, &args, num_threads);
}

inline static bool
sequentialShuffle(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                    size_t idx_len)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_NON_NULL_OR_ERROR(dstIdx)
    CARBON_NON_NULL_OR_ERROR(srcIdx)
    CARBON_NON_NULL_OR_ERROR(width)

    bool has_first = (idx_len > 0);
    CARBON_PREFETCH_READ(srcIdx);
    CARBON_PREFETCH_READ(dstIdx);
    CARBON_PREFETCH_READ(has_first ? src + srcIdx[0] * width : NULL);
    CARBON_PREFETCH_WRITE(has_first ? dst + dstIdx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1) {
        memcpy(dst + dstIdx[i] * width, src + srcIdx[i] * width, width);

        bool has_next = (next_i < idx_len);
        CARBON_PREFETCH_READ(has_next ? srcIdx + next_i : NULL);
        CARBON_PREFETCH_READ(has_next ? dstIdx + next_i : NULL);
        CARBON_PREFETCH_READ(has_next ? src + srcIdx[next_i] * width : NULL);
        CARBON_PREFETCH_WRITE(has_next ? dst + dstIdx[next_i] * width : NULL);
    }

    return true;
}

inline static bool
parallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idx_len)
{
    CARBON_UNUSED(dst);
    CARBON_UNUSED(src);
    CARBON_UNUSED(width);
    CARBON_UNUSED(dstIdx);
    CARBON_UNUSED(srcIdx);
    CARBON_UNUSED(idx_len);
    CARBON_NOT_IMPLEMENTED
}

typedef struct FilterArg
{
    size_t numPositions;
    size_t *restrict srcPositions;
    const void *restrict start;
    size_t len;
    size_t width;
    void *restrict args;
    Predicate pred;
    size_t positionOffsetToAdd;
} FilterArg;

inline static bool
sequentialFilterLate(size_t *restrict positions, size_t *restrict numPositions,
                                       const void *restrict source, size_t width, size_t length, Predicate predicate,
                                       void *restrict arguments)
{
    CARBON_NON_NULL_OR_ERROR(positions);
    CARBON_NON_NULL_OR_ERROR(numPositions);
    CARBON_NON_NULL_OR_ERROR(source);
    CARBON_NON_NULL_OR_ERROR(width);
    CARBON_NON_NULL_OR_ERROR(length);
    CARBON_NON_NULL_OR_ERROR(predicate);

    predicate(positions, numPositions, source, width, length, arguments, 0);

    return true;
}

inline static void *
filterProxyFunc(void *restrict args)
{
    CARBON_CAST(FilterArg *restrict, proxyArg, args);
    proxyArg->pred(proxyArg->srcPositions, &proxyArg->numPositions,
                   proxyArg->start, proxyArg->width, proxyArg->len, proxyArg->args,
                   proxyArg->positionOffsetToAdd);
    return NULL;
}

inline static bool
parallelFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                         const void *restrict src, size_t width, size_t len, Predicate pred,
                                         void *restrict args, size_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(pos);
    CARBON_NON_NULL_OR_ERROR(numPos);
    CARBON_NON_NULL_OR_ERROR(src);
    CARBON_NON_NULL_OR_ERROR(width);
    CARBON_NON_NULL_OR_ERROR(pred);

    if (CARBON_BRANCH_UNLIKELY(len == 0)) {
        *numPos = 0;
        return true;
    }

    uint_fast16_t numThread = num_threads + 1; /** +1 since one is this thread */

    pthread_t threads[num_threads];
    FilterArg thread_args[numThread];

    register size_t chunkLen = len / numThread;
    size_t chunkLenRemain = len % numThread;
    size_t mainPositionOffsetToAdd = num_threads * chunkLen;
    const void *restrict mainThreadBase = src + mainPositionOffsetToAdd * width;

    CARBON_PREFETCH_READ(pred);
    CARBON_PREFETCH_READ(args);

    /** run f on NTHREADS_FOR additional threads */
    if (CARBON_BRANCH_LIKELY(chunkLen > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            FilterArg *arg = thread_args + tid;
            arg->numPositions = 0;
            arg->srcPositions = malloc(chunkLen * sizeof(size_t));
            arg->positionOffsetToAdd = tid * chunkLen;
            arg->start = src + arg->positionOffsetToAdd * width;
            arg->len = chunkLen;
            arg->width = width;
            arg->args = args;
            arg->pred = pred;

            CARBON_PREFETCH_READ(arg->start);
            pthread_create(threads + tid, NULL, filterProxyFunc, arg);
        }
    }
    /** run f on this thread */
    CARBON_PREFETCH_READ(mainThreadBase);
    size_t mainChunkLen = chunkLen + chunkLenRemain;
    size_t *mainSrcPositions = malloc(mainChunkLen * sizeof(size_t));
    size_t mainNumPositions = 0;

    pred(mainSrcPositions,
         &mainNumPositions,
         mainThreadBase,
         width,
         mainChunkLen,
         args,
         mainPositionOffsetToAdd);

    size_t totalNumMatchingPositions = 0;

    if (CARBON_BRANCH_LIKELY(chunkLen > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
            pthread_join(threads[tid], NULL);
            const FilterArg *restrict threadArg = (thread_args + tid);
            if (threadArg->numPositions > 0) {
                memcpy(pos + totalNumMatchingPositions, threadArg->srcPositions,
                       threadArg->numPositions * sizeof(size_t));
                totalNumMatchingPositions += threadArg->numPositions;
            }
            free(thread_args[tid].srcPositions);
        }
    }

    if (CARBON_BRANCH_LIKELY(mainNumPositions > 0)) {
        memcpy(pos + totalNumMatchingPositions, mainSrcPositions,
               mainNumPositions * sizeof(size_t));
        totalNumMatchingPositions += mainNumPositions;
    }
    free(mainSrcPositions);

    *numPos = totalNumMatchingPositions;

    return true;
}

inline static bool
sequentialFilterEarly(void *restrict result, size_t *restrict resultSize,
                                            const void *restrict src, size_t width, size_t len, Predicate pred,
                                            void *restrict args)
{
    CARBON_NON_NULL_OR_ERROR(result);
    CARBON_NON_NULL_OR_ERROR(resultSize);
    CARBON_NON_NULL_OR_ERROR(src);
    CARBON_NON_NULL_OR_ERROR(width);
    CARBON_NON_NULL_OR_ERROR(len);
    CARBON_NON_NULL_OR_ERROR(pred);

    size_t numMatchingPositions;
    size_t *restrict matchingPositions = malloc(len * sizeof(size_t));

    pred(matchingPositions, &numMatchingPositions, src, width, len, args, 0);

    ParallelGather(result, src, width, matchingPositions, numMatchingPositions, ThreadingHint_Single, 0);
    *resultSize = numMatchingPositions;

    free(matchingPositions);

    return true;
}

inline static bool
parallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                          const void *restrict src, size_t width, size_t len, Predicate pred,
                                          void *restrict args, uint_fast16_t num_threads)
{
    CARBON_NON_NULL_OR_ERROR(result);
    CARBON_NON_NULL_OR_ERROR(resultSize);
    CARBON_NON_NULL_OR_ERROR(src);
    CARBON_NON_NULL_OR_ERROR(width);
    CARBON_NON_NULL_OR_ERROR(len);
    CARBON_NON_NULL_OR_ERROR(pred);

    uint_fast16_t numThread = num_threads + 1; /** +1 since one is this thread */

    pthread_t threads[num_threads];
    FilterArg thread_args[numThread];

    register size_t chunkLen = len / numThread;
    size_t chunkLenRemain = len % numThread;
    size_t mainPositionOffsetToAdd = num_threads * chunkLen;
    const void *restrict mainThreadBase = src + mainPositionOffsetToAdd * width;

    CARBON_PREFETCH_READ(pred);
    CARBON_PREFETCH_READ(args);

    /** run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        FilterArg *arg = thread_args + tid;
        arg->numPositions = 0;
        arg->srcPositions = malloc(chunkLen * sizeof(size_t));
        arg->positionOffsetToAdd = tid * chunkLen;
        arg->start = src + arg->positionOffsetToAdd * width;
        arg->len = chunkLen;
        arg->width = width;
        arg->args = args;
        arg->pred = pred;

        CARBON_PREFETCH_READ(arg->start);
        pthread_create(threads + tid, NULL, filterProxyFunc, arg);
    }
    /** run f on this thread */
    CARBON_PREFETCH_READ(mainThreadBase);
    size_t mainChunkLen = chunkLen + chunkLenRemain;
    size_t *mainSrcPositions = malloc(mainChunkLen * sizeof(size_t));
    size_t mainNumPositions = 0;

    pred(mainSrcPositions, &mainNumPositions, mainThreadBase, width, mainChunkLen, args,
         mainPositionOffsetToAdd);


    size_t totalNumMatchingPositions = mainNumPositions;
    size_t partial_numMatchingPositions = 0;

    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        pthread_join(threads[tid], NULL);
        const FilterArg *restrict threadArg = (thread_args + tid);
        totalNumMatchingPositions += threadArg->numPositions;
        CARBON_PREFETCH_READ(threadArg->srcPositions);
    }

    for (register uint_fast16_t tid = 0; tid < num_threads; tid++) {
        const FilterArg *restrict threadArg = (thread_args + tid);

        if (CARBON_BRANCH_LIKELY(threadArg->numPositions > 0)) {
            ParallelGather(result + partial_numMatchingPositions * width, src, width, threadArg->srcPositions,
                           threadArg->numPositions,
                           CARBON_PARALLEL_THREAD_HINT_MULTI, num_threads);
        }

        partial_numMatchingPositions += threadArg->numPositions;
        free(threadArg->srcPositions);
    }

    if (CARBON_BRANCH_LIKELY(mainNumPositions > 0)) {
        ParallelGather(result + partial_numMatchingPositions * width, src, width, mainSrcPositions,
                       mainNumPositions, CARBON_PARALLEL_THREAD_HINT_MULTI, num_threads);
    }
    free(mainSrcPositions);

    *resultSize = totalNumMatchingPositions;

    return true;
}

CARBON_END_DECL

#endif
