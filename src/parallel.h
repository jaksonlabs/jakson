// file: parallel.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NG5_PARALLEL
#define NG5_PARALLEL

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "stdlib.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

#define PARALLEL_MSG_UNKNOWN_HINT "Unknown threading hint"

// ---------------------------------------------------------------------------------------------------------------------
//
//  D A T A   T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef uint_fast16_t ThreadId;

typedef void (*ForFunctionBody)(const void *restrict start, size_t width, size_t len, void *restrict args, ThreadId tid);

typedef void (*MapFunctionBody)(void *restrict dst, const void *restrict src, size_t srcWidth, size_t dstWidth,
                                size_t len, void *restrict args);

typedef void (*Predicate)(size_t *matchingPositions, size_t *numMatchingPositions, const void *restrict src,
                            size_t width, size_t len, void *restrict args, size_t positionOffsetToAdd);

typedef enum ThreadingHint
{
    ThreadingHint_Single,
    ThreadingHint_Multi
} ThreadingHint;

typedef struct FunctionProxy
{
    ForFunctionBody function;
    const void *restrict start;
    size_t width;
    size_t len;
    ThreadId tid;
    void *args;
} FunctionProxy;

inline static void *forProxyFunction(void *restrict args)
{
    CAST(FunctionProxy *restrict, proxyArg, args);
    proxyArg->function(proxyArg->start, proxyArg->width, proxyArg->len, proxyArg->args, proxyArg->tid);
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define PARALLEL_ERROR(msg, retval)                                      \
{                                                                        \
    perror(msg);                                                         \
    return retval;                                                       \
}

#define PARALLEL_MATCH(forSingle, forMulti)                              \
{                                                                        \
    if (BRANCH_LIKELY(hint == ThreadingHint_Multi)) {                    \
        return (forMulti);                                               \
    } else if (hint == ThreadingHint_Single) {                           \
        return (forSingle);                                              \
    } else PARALLEL_ERROR(PARALLEL_MSG_UNKNOWN_HINT, STATUS_INTERNALERR);\
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

inline static int ParallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                              void *restrict args, ThreadingHint hint, uint_fast16_t numThreads);

inline static int ParallelMap(void *restrict dst,
                              const void *restrict src,
                              size_t srcWidth,
                              size_t len,
                              size_t dstWidth,
                              MapFunctionBody f,
                              void *restrict args,
                              ThreadingHint hint,
                              uint_fast16_t numThreads);

inline static int ParallelGather(void *restrict dst,
                                 const void *restrict src,
                                 size_t width,
                                 const size_t *restrict idx,
                                 size_t dstSrcLen,
                                 ThreadingHint hint,
                                 uint_fast16_t numThreads);

inline static int ParallelGatherAddress(void *restrict dst,
                                     const void *restrict src,
                                     size_t srcWidth,
                                     const size_t *restrict idx,
                                     size_t num,
                                     ThreadingHint hint,
                                     uint_fast16_t numThreads);

inline static int ParallelScatter(void *restrict dst,
                                  const void *restrict src,
                                  size_t width,
                                  const size_t *restrict idx,
                                  size_t num,
                                  ThreadingHint hint,
                                  uint_fast16_t numThreads);

inline static int ParallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idxLen, ThreadingHint hint);

inline static int ParallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                       const void *restrict src,
                                       size_t width, size_t len, Predicate pred, void *restrict args,
                                       ThreadingHint hint, uint_fast16_t numThreads);

inline static int ParallelFilterLate(size_t *restrict pos, size_t *restrict numPos, const void *restrict src,
                                      size_t width, size_t len, Predicate pred, void *restrict args,
                                      ThreadingHint hint, size_t numThreads);


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R N A L   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

inline static int sequentialFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                   void *restrict args);
inline static int parallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                 void *restrict args, uint_fast16_t numThreads);

inline static int map(void *restrict dst,
                        const void *restrict src,
                        size_t srcWidth,
                        size_t len,
                        size_t dstWidth,
                        MapFunctionBody f,
                        void *restrict args,
                        ThreadingHint hint,
                        uint_fast16_t numThreads);

inline static int sequentialGather(void *restrict dst, const void *restrict src, size_t width,
                                      const size_t *restrict idx, size_t dstSrcLen);
inline static int parallelGather(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t dstSrcLen, uint_fast16_t numThreads);

inline static int sequentialGatherAddress(void *restrict dst, const void *restrict src, size_t srcWidth,
                                          const size_t *restrict idx, size_t num);
inline static int parallelGather_adr(void *restrict dst, const void *restrict src, size_t srcWidth,
                                        const size_t *restrict idx, size_t num, uint_fast16_t numThreads);

inline static int sequentialScatter(void *restrict dst, const void *restrict src, size_t width,
                                       const size_t *restrict idx, size_t num);
inline static int parallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                     const size_t *restrict idx, size_t num, uint_fast16_t numThreads);

inline static int sequentialShuffle(void *restrict dst, const void *restrict src, size_t width,
                                       const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                       size_t idx_len);

inline static int parallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                     const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                     size_t idx_len);

inline static int sequentialFilterEarly(void *restrict result, size_t *restrict resultSize,
                                            const void *restrict src, size_t width, size_t len, Predicate pred,
                                            void *restrict args);

inline static int parallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                          const void *restrict src, size_t width, size_t len, Predicate pred,
                                          void *restrict args, uint_fast16_t numThreads);

inline static int sequentialFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                           const void *restrict src, size_t width, size_t len, Predicate pred,
                                           void *restrict args);

inline static int parallelFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                         const void *restrict src, size_t width, size_t len, Predicate pred,
                                         void *restrict args, size_t numThreads);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R N A L   I M P L E M E N A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

inline static int ParallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                              void *restrict args, ThreadingHint hint, uint_fast16_t numThreads)
{
    PARALLEL_MATCH(sequentialFor(base, width, len, f, args),
                   parallelFor(base, width, len, f, args, numThreads))
}

inline static int ParallelMap(void *restrict dst,
                              const void *restrict src,
                              size_t srcWidth,
                              size_t len,
                              size_t dstWidth,
                              MapFunctionBody f,
                              void *restrict args,
                              ThreadingHint hint,
                              uint_fast16_t numThreads)
{
    return map(dst, src, srcWidth, len, dstWidth, f, args, hint, numThreads);
}

inline static int ParallelGather(void *restrict dst,
                                 const void *restrict src,
                                 size_t width,
                                 const size_t *restrict idx,
                                 size_t dst_src_len,
                                 ThreadingHint hint,
                                 uint_fast16_t numThreads)
{
    PARALLEL_MATCH(sequentialGather(dst, src, width, idx, dst_src_len),
                   parallelGather(dst, src, width, idx, dst_src_len, numThreads))
}

inline static int ParallelGatherAddress(void *restrict dst,
                                     const void *restrict src,
                                     size_t srcWidth,
                                     const size_t *restrict idx,
                                     size_t num,
                                     ThreadingHint hint,
                                     uint_fast16_t numThreads)
{
    PARALLEL_MATCH(sequentialGatherAddress(dst, src, srcWidth, idx, num),
                   parallelGather_adr(dst, src, srcWidth, idx, num, numThreads))
}

inline static int ParallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict idx, size_t num, ThreadingHint hint, uint_fast16_t numThreads)
{
    PARALLEL_MATCH(sequentialScatter(dst, src, width, idx, num),
                   parallelScatter(dst, src, width, idx, num, numThreads))
}

inline static int ParallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idx_len, ThreadingHint hint)
{
    PARALLEL_MATCH(sequentialShuffle(dst, src, width, dstIdx, srcIdx, idx_len),
                   parallelShuffle(dst, src, width, dstIdx, srcIdx, idx_len))
}

inline static int ParallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                       const void *restrict src,
                                       size_t width, size_t len, Predicate pred, void *restrict args,
                                       ThreadingHint hint, uint_fast16_t numThreads)
{
    PARALLEL_MATCH(sequentialFilterEarly(result, resultSize, src, width, len, pred, args),
                   parallelFilterEarly(result, resultSize, src, width, len, pred, args, numThreads))
}

inline static int ParallelFilterLate(size_t *restrict pos, size_t *restrict numPos, const void *restrict src,
                                      size_t width, size_t len, Predicate pred, void *restrict args,
                                      ThreadingHint hint, size_t numThreads)
{
    PARALLEL_MATCH(sequentialFilterLate(pos, numPos, src, width, len, pred, args),
                   parallelFilterLate(pos, numPos, src, width, len, pred, args, numThreads))
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  F O R
//
// ---------------------------------------------------------------------------------------------------------------------

inline static int sequentialFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                   void *restrict args)
{
    CHECK_NON_NULL(base)
    CHECK_NON_NULL(width)
    CHECK_NON_NULL(len)
    f(base, width, len, args, 0);
    return STATUS_OK;
}

inline static int parallelFor(const void *restrict base, size_t width, size_t len, ForFunctionBody f,
                                 void *restrict args, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(base)
    CHECK_NON_NULL(width)
    CHECK_NON_NULL(len)

    uint_fast16_t numThread = numThreads + 1; /* +1 since one is this thread */
    pthread_t threads[numThreads];
    FunctionProxy proxyArgs[numThread];
    register size_t chunkLen = len / numThread;
    size_t chunkLenRemain = len % numThread;
    const void *restrict mainThreadBase = base + numThreads * chunkLen * width;

    PREFETCH_READ(f);
    PREFETCH_READ(args);

    /* run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
        FunctionProxy *proxyArg = proxyArgs + tid;
        proxyArg->start = base + tid * chunkLen * width;
        proxyArg->len = chunkLen;
        proxyArg->tid = (tid + 1);
        proxyArg->width = width;
        proxyArg->args = args;
        proxyArg->function = f;

        PREFETCH_READ(proxyArg->start);
        pthread_create(threads + tid, NULL, forProxyFunction, proxyArgs + tid);
    }
    /* run f on this thread */
    PREFETCH_READ(mainThreadBase);
    f(mainThreadBase, width, chunkLen + chunkLenRemain, args, 0);

    for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
        pthread_join(threads[tid], NULL);
    }

    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A P
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct MapArgs
{
    MapFunctionBody mapFunction;
    void *restrict dst;
    const void *restrict src;
    size_t dstWidth;
    void *args;
} MapArgs;

inline static void mapProxy(const void *restrict src, size_t srcWidth, size_t len, void *restrict args,
                               ThreadId tid)
{
    UNUSED(tid);
    CAST(MapArgs *, mapArgs, args);
    size_t globalStart = (src - mapArgs->src) / srcWidth;

    PREFETCH_READ(mapArgs->src);
    PREFETCH_READ(mapArgs->args);
    PREFETCH_WRITE(mapArgs->dst);
    mapArgs->mapFunction(mapArgs->dst + globalStart * mapArgs->dstWidth,
                       src, srcWidth, mapArgs->dstWidth, len, mapArgs->args);
}

inline static int map(void *restrict dst, const void *restrict src, size_t srcWidth, size_t len, size_t
                      dstWidth, MapFunctionBody f, void *restrict args, ThreadingHint hint, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(srcWidth)
    CHECK_NON_NULL(dstWidth)
    CHECK_NON_NULL(f)

    PREFETCH_READ(f);
    PREFETCH_WRITE(dst);

    MapArgs mapArgs = {
        .args = args,
        .mapFunction = f,
        .dst = dst,
        .dstWidth = dstWidth,
        .src = src
    };

    return ParallelFor((void *restrict) src, srcWidth, len, &mapProxy, &mapArgs, hint, numThreads);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  G A T H E R
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct GatherScatterArgs
{
    const size_t *restrict idx;
    const void *restrict src;
    void *restrict dst;
} GatherScatterArgs;

inline static void gatherFunction(const void *restrict start, size_t width, size_t len, void *restrict args,
                                 ThreadId tid)
{
    UNUSED(tid);
    CAST(GatherScatterArgs *, gatherArgs, args);
    size_t globalIndexStart = (start - gatherArgs->dst) / width;

    PREFETCH_WRITE(gatherArgs->dst);
    PREFETCH_WRITE(gatherArgs->idx);
    PREFETCH_READ((len > 0) ? gatherArgs->src + gatherArgs->idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;
        memcpy(gatherArgs->dst + globalIndexCur * width,
               gatherArgs->src + gatherArgs->idx[globalIndexCur] * width,
               width);

        bool has_next = (next_i < len);
        PREFETCH_READ(has_next ? gatherArgs->idx + globalIndexNext : NULL);
        PREFETCH_READ(has_next ? gatherArgs->src + gatherArgs->idx[globalIndexNext] * width : NULL);
        PREFETCH_WRITE(has_next ? gatherArgs->dst + globalIndexNext * width : NULL);
    }
}

inline static int sequentialGather(void *restrict dst, const void *restrict src, size_t width,
                                      const size_t *restrict idx, size_t dst_src_len)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(width)

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

    return STATUS_OK;
}

inline static int parallelGather(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t dst_src_len, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(width)

    PREFETCH_READ(src);
    PREFETCH_READ(idx);
    PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx           = idx,
        .src           = src,
        .dst           = dst,
    };
    return parallelFor(dst, width, dst_src_len, gatherFunction, &args, numThreads);
}

inline static int sequentialGatherAddress(void *restrict dst, const void *restrict src, size_t srcWidth,
                                          const size_t *restrict idx, size_t num)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(srcWidth)

    PREFETCH_READ(src);
    PREFETCH_READ(idx);
    PREFETCH_WRITE(dst);

    PREFETCH_READ(idx);
    PREFETCH_WRITE(dst);
    PREFETCH_READ(num > 0 ? src + idx[0] * srcWidth : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1) {
        const void *ptr = src + idx[i] * srcWidth;
        size_t adr = (size_t) ptr;
        memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < num);
        PREFETCH_READ(has_next ? idx + next_i : NULL);
        PREFETCH_READ(has_next ? src + idx[next_i] * srcWidth : NULL);
        PREFETCH_WRITE(has_next ? dst + next_i * sizeof(void *) : NULL);
    }
    return STATUS_OK;
}

inline static void gatherAddressFunc(const void *restrict start, size_t width, size_t len, void *restrict args,
                                     ThreadId tid)
{
    UNUSED(tid);
    CAST(GatherScatterArgs *, gatherArgs, args);

    PREFETCH_READ(gatherArgs->idx);
    PREFETCH_WRITE(gatherArgs->dst);
    PREFETCH_READ((len > 0) ? gatherArgs->src + gatherArgs->idx[0] * width : NULL);

    size_t globalIndexStart = (start - gatherArgs->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;
        const void *ptr = gatherArgs->src + gatherArgs->idx[globalIndexCur] * width;
        size_t adr = (size_t) ptr;
        memcpy(gatherArgs->dst + globalIndexCur * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < len);
        PREFETCH_READ(has_next ? gatherArgs->idx + globalIndexNext : NULL);
        PREFETCH_READ(has_next ? gatherArgs->src + gatherArgs->idx[globalIndexNext] * width : NULL);
        PREFETCH_WRITE(has_next ? gatherArgs->dst + globalIndexNext * sizeof(void *) : NULL);
    }
}

inline static int parallelGather_adr(void *restrict dst, const void *restrict src, size_t srcWidth,
                                     const size_t *restrict idx, size_t num, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(srcWidth)

    PREFETCH_READ(src);
    PREFETCH_READ(idx);
    PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return parallelFor(dst, srcWidth, num, gatherAddressFunc, &args, numThreads);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  S C A T T E R
//
// ---------------------------------------------------------------------------------------------------------------------

inline static void scatterFunction(const void *restrict start, size_t width, size_t len, void *restrict args,
                                   ThreadId tid)
{
    UNUSED(tid);
    CAST(GatherScatterArgs *, scatter_args, args);

    PREFETCH_READ(scatter_args->idx);
    PREFETCH_READ(scatter_args->src);
    PREFETCH_WRITE((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

    size_t globalIndexStart = (start - scatter_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1) {
        size_t globalIndexCur = globalIndexStart + i;
        size_t globalIndexNext = globalIndexStart + next_i;

        memcpy(scatter_args->dst + scatter_args->idx[globalIndexCur] * width,
               scatter_args->src + globalIndexCur * width,
               width);

        bool has_next = (next_i < len);
        PREFETCH_READ(has_next ? scatter_args->idx + globalIndexNext : NULL);
        PREFETCH_READ(has_next ? scatter_args->src + globalIndexNext * width : NULL);
        PREFETCH_WRITE(has_next ? scatter_args->dst + scatter_args->idx[globalIndexNext] * width : NULL);
    }
}

inline static int sequentialScatter(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict idx, size_t num)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(width)

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
    return STATUS_OK;
}

inline static int parallelScatter(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict idx, size_t num, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(idx)
    CHECK_NON_NULL(width)

    PREFETCH_READ(src);
    PREFETCH_READ(idx);
    PREFETCH_WRITE(dst);

    GatherScatterArgs args = {
        .idx = idx,
        .src = src,
        .dst = dst
    };
    return parallelFor(dst, width, num, scatterFunction, &args, numThreads);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  S H U F F L E
//
// ---------------------------------------------------------------------------------------------------------------------

inline static int sequentialShuffle(void *restrict dst, const void *restrict src, size_t width,
                                    const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                    size_t idx_len)
{
    CHECK_NON_NULL(dst)
    CHECK_NON_NULL(src)
    CHECK_NON_NULL(dstIdx)
    CHECK_NON_NULL(srcIdx)
    CHECK_NON_NULL(width)

    bool has_first = (idx_len > 0);
    PREFETCH_READ(srcIdx);
    PREFETCH_READ(dstIdx);
    PREFETCH_READ(has_first ? src + srcIdx[0] * width : NULL);
    PREFETCH_WRITE(has_first ? dst + dstIdx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1) {
        memcpy(dst + dstIdx[i] * width, src + srcIdx[i] * width, width);

        bool has_next = (next_i < idx_len);
        PREFETCH_READ(has_next ? srcIdx + next_i : NULL);
        PREFETCH_READ(has_next ? dstIdx + next_i : NULL);
        PREFETCH_READ(has_next ? src + srcIdx[next_i] * width : NULL);
        PREFETCH_WRITE(has_next ? dst + dstIdx[next_i] * width : NULL);
    }

    return STATUS_OK;
}

inline static int parallelShuffle(void *restrict dst, const void *restrict src, size_t width,
                                  const size_t *restrict dstIdx, const size_t *restrict srcIdx,
                                  size_t idx_len)
{
    UNUSED(dst);
    UNUSED(src);
    UNUSED(width);
    UNUSED(dstIdx);
    UNUSED(srcIdx);
    UNUSED(idx_len);
    NOT_YET_IMPLEMENTED
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  F I L T E R
//
// ---------------------------------------------------------------------------------------------------------------------

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

inline static int sequentialFilterLate(size_t *restrict positions, size_t *restrict numPositions,
                                       const void *restrict source, size_t width, size_t length, Predicate predicate,
                                       void *restrict arguments)
{
    CHECK_NON_NULL(positions);
    CHECK_NON_NULL(numPositions);
    CHECK_NON_NULL(source);
    CHECK_NON_NULL(width);
    CHECK_NON_NULL(length);
    CHECK_NON_NULL(predicate);

    predicate(positions, numPositions, source, width, length, arguments, 0);

    return STATUS_OK;
}

inline static void *filterProxyFunc(void *restrict args)
{
    CAST(FilterArg *restrict, proxyArg, args);
    proxyArg->pred(proxyArg->srcPositions, &proxyArg->numPositions,
                   proxyArg->start, proxyArg->width, proxyArg->len, proxyArg->args,
                   proxyArg->positionOffsetToAdd);
    return NULL;
}

inline static int parallelFilterLate(size_t *restrict pos, size_t *restrict numPos,
                                         const void *restrict src, size_t width, size_t len, Predicate pred,
                                         void *restrict args, size_t numThreads)
{
    CHECK_NON_NULL(pos);
    CHECK_NON_NULL(numPos);
    CHECK_NON_NULL(src);
    CHECK_NON_NULL(width);
    CHECK_NON_NULL(pred);

    if (BRANCH_UNLIKELY(len == 0)) {
        *numPos = 0;
        return STATUS_OK;
    }

    uint_fast16_t numThread = numThreads + 1; /* +1 since one is this thread */

    pthread_t threads[numThreads];
    FilterArg threadArgs[numThread];

    register size_t chunkLen = len / numThread;
    size_t chunkLenRemain = len % numThread;
    size_t mainPositionOffsetToAdd = numThreads * chunkLen;
    const void *restrict mainThreadBase = src + mainPositionOffsetToAdd * width;

    PREFETCH_READ(pred);
    PREFETCH_READ(args);

    /* run f on NTHREADS_FOR additional threads */
    if (BRANCH_LIKELY(chunkLen > 0)) {
        for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
            FilterArg *arg = threadArgs + tid;
            arg->numPositions = 0;
            arg->srcPositions = malloc(chunkLen * sizeof(size_t));
            arg->positionOffsetToAdd = tid * chunkLen;
            arg->start = src + arg->positionOffsetToAdd * width;
            arg->len = chunkLen;
            arg->width = width;
            arg->args = args;
            arg->pred = pred;

            PREFETCH_READ(arg->start);
            pthread_create(threads + tid, NULL, filterProxyFunc, arg);
        }
    }
    /* run f on this thread */
    PREFETCH_READ(mainThreadBase);
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

    if (BRANCH_LIKELY(chunkLen > 0)) {
        for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
            pthread_join(threads[tid], NULL);
            const FilterArg *restrict threadArg = (threadArgs + tid);
            if (threadArg->numPositions > 0) {
                memcpy(pos + totalNumMatchingPositions, threadArg->srcPositions,
                       threadArg->numPositions * sizeof(size_t));
                totalNumMatchingPositions += threadArg->numPositions;
            }
            free(threadArgs[tid].srcPositions);
        }
    }

    if (BRANCH_LIKELY(mainNumPositions > 0)) {
        memcpy(pos + totalNumMatchingPositions, mainSrcPositions,
               mainNumPositions * sizeof(size_t));
        totalNumMatchingPositions += mainNumPositions;
    }
    free(mainSrcPositions);

    *numPos = totalNumMatchingPositions;

    return STATUS_OK;
}

inline static int sequentialFilterEarly(void *restrict result, size_t *restrict resultSize,
                                            const void *restrict src, size_t width, size_t len, Predicate pred,
                                            void *restrict args)
{
    CHECK_NON_NULL(result);
    CHECK_NON_NULL(resultSize);
    CHECK_NON_NULL(src);
    CHECK_NON_NULL(width);
    CHECK_NON_NULL(len);
    CHECK_NON_NULL(pred);

    size_t numMatchingPositions;
    size_t *restrict matchingPositions = malloc(len * sizeof(size_t));

    pred(matchingPositions, &numMatchingPositions, src, width, len, args, 0);

    ParallelGather(result, src, width, matchingPositions, numMatchingPositions, ThreadingHint_Single, 0);
    *resultSize = numMatchingPositions;

    free(matchingPositions);

    return STATUS_OK;
}

inline static int parallelFilterEarly(void *restrict result, size_t *restrict resultSize,
                                          const void *restrict src, size_t width, size_t len, Predicate pred,
                                          void *restrict args, uint_fast16_t numThreads)
{
    CHECK_NON_NULL(result);
    CHECK_NON_NULL(resultSize);
    CHECK_NON_NULL(src);
    CHECK_NON_NULL(width);
    CHECK_NON_NULL(len);
    CHECK_NON_NULL(pred);

    uint_fast16_t numThread = numThreads + 1; /* +1 since one is this thread */

    pthread_t threads[numThreads];
    FilterArg threadArgs[numThread];

    register size_t chunkLen = len / numThread;
    size_t chunkLenRemain = len % numThread;
    size_t mainPositionOffsetToAdd = numThreads * chunkLen;
    const void *restrict mainThreadBase = src + mainPositionOffsetToAdd * width;

    PREFETCH_READ(pred);
    PREFETCH_READ(args);

    /* run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
        FilterArg *arg = threadArgs + tid;
        arg->numPositions = 0;
        arg->srcPositions = malloc(chunkLen * sizeof(size_t));
        arg->positionOffsetToAdd = tid * chunkLen;
        arg->start = src + arg->positionOffsetToAdd * width;
        arg->len = chunkLen;
        arg->width = width;
        arg->args = args;
        arg->pred = pred;

        PREFETCH_READ(arg->start);
        pthread_create(threads + tid, NULL, filterProxyFunc, arg);
    }
    /* run f on this thread */
    PREFETCH_READ(mainThreadBase);
    size_t mainChunkLen = chunkLen + chunkLenRemain;
    size_t *mainSrcPositions = malloc(mainChunkLen * sizeof(size_t));
    size_t mainNumPositions = 0;

    pred(mainSrcPositions, &mainNumPositions, mainThreadBase, width, mainChunkLen, args,
         mainPositionOffsetToAdd);


    size_t totalNumMatchingPositions = mainNumPositions;
    size_t partial_numMatchingPositions = 0;

    for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
        pthread_join(threads[tid], NULL);
        const FilterArg *restrict threadArg = (threadArgs + tid);
        totalNumMatchingPositions += threadArg->numPositions;
        PREFETCH_READ(threadArg->srcPositions);
    }

    for (register uint_fast16_t tid = 0; tid < numThreads; tid++) {
        const FilterArg *restrict threadArg = (threadArgs + tid);

        if (BRANCH_LIKELY(threadArg->numPositions > 0)) {
            ParallelGather(result + partial_numMatchingPositions * width, src, width, threadArg->srcPositions,
                           threadArg->numPositions,
                           ThreadingHint_Multi, numThreads);
        }

        partial_numMatchingPositions += threadArg->numPositions;
        free(threadArg->srcPositions);
    }

    if (BRANCH_LIKELY(mainNumPositions > 0)) {
        ParallelGather(result + partial_numMatchingPositions * width, src, width, mainSrcPositions,
                       mainNumPositions, ThreadingHint_Multi, numThreads);
    }
    free(mainSrcPositions);

    *resultSize = totalNumMatchingPositions;

    return STATUS_OK;
}

NG5_END_DECL

#endif //BOLSTER_H
