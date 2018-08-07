// Bolster Framework
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef NG5_BOLSTER_H
#define NG5_BOLSTER_H

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <common.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G U R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BOLSTER_NTHREADS
#define BOLSTER_NTHREADS 7
#endif

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define BOLSTER_MSG_UNKNOWN_HINT "Unknown threading hint"

__BEGIN_DECLS

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef void (*functor_t)(const void *restrict start, size_t width, size_t len, void *restrict args);

typedef void (*map_func_t)(void *restrict dst, const void *restrict src, size_t src_width, size_t dst_width, size_t len,
        void *restrict args);

typedef void (*pred_func_t)(size_t *matching_positions, size_t *num_matching_positions, const void *restrict src,
        size_t width, size_t len, void *restrict args, size_t position_offset_to_add);


typedef enum threading_hint_e
{
  threading_hint_single,
  threading_hint_multi
} threading_hint_e;


typedef struct __func_proxy_arg_t
{
  functor_t        f;
  const void      *restrict start;
  size_t           width;
  size_t           len;
  void            *args;
} __for_proxy_arg_t;

inline static void *__for_proxy_func(void *restrict args)
{
    cast(__for_proxy_arg_t *restrict, proxy_arg, args);
    proxy_arg->f(proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args);
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define bolster_error(msg, retval)                                      \
{                                                                       \
    perror(msg);                                                        \
    return retval;                                                      \
}

#define bolster_threading_switch(for_single, for_multi)                 \
{                                                                       \
    if (likely(hint == threading_hint_multi)) {                         \
        return (for_multi);                                             \
    } else if (hint == threading_hint_single) {                         \
        return (for_single);                                            \
    } else bolster_error(BOLSTER_MSG_UNKNOWN_HINT, STATUS_INTERNALERR); \
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

inline static int bolster_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args, threading_hint_e hint);

inline static int bolster_map(void *restrict dst, const void *restrict src, size_t src_width, size_t len,
        size_t dst_width, map_func_t f, void *restrict args, threading_hint_e hint);

inline static int bolster_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len, threading_hint_e hint);

inline static int bolster_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num, threading_hint_e hint);

inline static int bolster_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num, threading_hint_e hint);

inline static int bolster_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len, threading_hint_e hint);

inline static int bolster_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src,
        size_t width, size_t len, pred_func_t pred, void *restrict args,
        threading_hint_e hint);

inline static int bolster_filter_late(size_t *restrict pos, size_t *restrict num_pos, const void *restrict src,
        size_t width, size_t len, pred_func_t pred, void *restrict args,
        threading_hint_e hint, size_t num_threads);


// ---------------------------------------------------------------------------------------------------------------------
//  I N T E R N A L I N T E R F A C E   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

inline static int __sequential_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args);
inline static int __parallel_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args);

inline static int __map(void *restrict dst, const void *restrict src, size_t src_width, size_t len,
        size_t dst_width, map_func_t f, void *restrict args, threading_hint_e hint);

inline static int __sequential_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len);
inline static int __parallel_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len);

inline static int __sequential_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num);
inline static int __parallel_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num);

inline static int __sequential_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num);
inline static int __parallel_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num);

inline static int __sequential_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len);

inline static int __parallel_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len);

inline static int __sequential_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args);

inline static int __parallel_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args);

inline static int __sequential_filter_late(size_t *restrict pos, size_t *restrict num_pos,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args);

inline static int __parallel_filter_late(size_t *restrict pos, size_t *restrict num_pos,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args, size_t num_threads);


// ---------------------------------------------------------------------------------------------------------------------
//  I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

inline static int bolster_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args, threading_hint_e hint)
{
    bolster_threading_switch(__sequential_for(base, width, len, f, args),
            __parallel_for  (base, width, len, f, args))
}

inline static int bolster_map(void *restrict dst, const void *restrict src, size_t src_width, size_t len,
        size_t dst_width, map_func_t f, void *restrict args, threading_hint_e hint)
{
    return __map(dst, src, src_width, len, dst_width, f, args, hint);
}

inline static int bolster_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len, threading_hint_e hint)
{
    bolster_threading_switch(__sequential_gather(dst, src, width, idx, dst_src_len),
            __parallel_gather(dst, src, width, idx, dst_src_len))
}

inline static int bolster_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num, threading_hint_e hint)
{
    bolster_threading_switch(__sequential_gather_adr(dst, src, src_width, idx, num),
            __parallel_gather_adr(dst, src, src_width, idx, num))
}

inline static int bolster_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num, threading_hint_e hint)
{
    bolster_threading_switch(__sequential_scatter(dst, src, width, idx, num),
            __parallel_scatter(dst, src, width, idx, num))
}

inline static int bolster_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len, threading_hint_e hint)
{
    bolster_threading_switch(__sequential_shuffle(dst, src, width, dst_idx, src_idx, idx_len),
            __parallel_shuffle(dst, src, width, dst_idx, src_idx, idx_len))
}

inline static int bolster_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src,
        size_t width, size_t len, pred_func_t pred, void *restrict args,
        threading_hint_e hint)
{
    bolster_threading_switch(__sequential_filter_early(result, result_size, src, width, len, pred, args),
            __parallel_filter_early(result, result_size, src, width, len, pred, args))
}

inline static int bolster_filter_late(size_t *restrict pos, size_t *restrict num_pos, const void *restrict src,
        size_t width, size_t len, pred_func_t pred, void *restrict args,
        threading_hint_e hint, size_t num_threads)
{
    bolster_threading_switch(__sequential_filter_late(pos, num_pos, src, width, len, pred, args),
            __parallel_filter_late(pos, num_pos, src, width, len, pred, args, num_threads))
}

// ---------------------------------------------------------------------------------------------------------------------
//  I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

//  B O L S T E R   F O R  ---------------------------------------------------------------------------------------------

inline static int __sequential_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args)
{
    check_non_null(base)
    check_non_null(width)
    check_non_null(len)
    f(base, width, len, args);
    return STATUS_OK;
}

inline static int __parallel_for(const void *restrict base, size_t width, size_t len, functor_t f,
        void *restrict args)
{
    check_non_null(base)
    check_non_null(width)
    check_non_null(len)

    uint_fast16_t          num_thread               = BOLSTER_NTHREADS + 1; /* +1 since one is this thread */
    pthread_t              threads[BOLSTER_NTHREADS];
    __for_proxy_arg_t      proxy_args[num_thread];
    register size_t        chunk_len                = len / num_thread;
    size_t                 chunk_len_remain         = len % num_thread;
    const void *restrict   main_thread_base         = base + BOLSTER_NTHREADS * chunk_len * width;

    prefetch_read(f);
    prefetch_read(args);

    /* run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < BOLSTER_NTHREADS; tid++)
    {
        __for_proxy_arg_t *proxy_arg = proxy_args + tid;
        proxy_arg->start  = base + tid * chunk_len * width;
        proxy_arg->len    = chunk_len;
        proxy_arg->width  = width;
        proxy_arg->args   = args;
        proxy_arg->f      = f;

        prefetch_read(proxy_arg->start);
        pthread_create(threads + tid, NULL, __for_proxy_func, proxy_args + tid);
    }
    /* run f on this thread */
    prefetch_read(main_thread_base);
    f(main_thread_base, width, chunk_len + chunk_len_remain, args);

    for (register uint_fast16_t tid = 0; tid < BOLSTER_NTHREADS; tid++) {
        pthread_join(threads[tid], NULL);
    }

    return STATUS_OK;
}

//  B O L S T E R   M A P  ---------------------------------------------------------------------------------------------

typedef struct __map_args_t {
  map_func_t           map_func;
  void *restrict       dst;
  const void *restrict src;
  size_t               dst_width;
  void *               args;
} __map_args_t;

inline static void __map_proxy(const void *restrict src, size_t src_width, size_t len, void *restrict args)
{
    cast(__map_args_t *, map_args, args);
    size_t global_start = (src - map_args->src) / src_width;

    prefetch_read(map_args->src);
    prefetch_read(map_args->args);
    prefetch_write(map_args->dst);
    map_args->map_func(map_args->dst + global_start * map_args->dst_width,
            src, src_width, map_args->dst_width, len, map_args->args);
}

inline static int __map(void *restrict dst, const void *restrict src, size_t src_width, size_t len, size_t
dst_width, map_func_t f, void *restrict args, threading_hint_e hint)
{
    check_non_null(src)
    check_non_null(src_width)
    check_non_null(dst_width)
    check_non_null(f)

    prefetch_read(f);
    prefetch_write(dst);

    __map_args_t map_args = {
            .args = args,
            .map_func = f,
            .dst = dst,
            .dst_width = dst_width,
            .src = src
    };

    return bolster_for((void *restrict) src, src_width, len, &__map_proxy, &map_args, hint);
}

//  B O L S T E R   G A T H E R  ---------------------------------------------------------------------------------------

typedef struct __gather_scatter_args_t
{
  const size_t *restrict idx;
  const void *restrict   src;
  void *restrict         dst;
} __gather_scatter_args_t;

inline static void __gather_func(const void *restrict start, size_t width, size_t len, void *restrict args)
{
    cast(__gather_scatter_args_t *, gather_args, args);
    size_t global_index_start = (start - gather_args->dst) / width;

    prefetch_write(gather_args->dst);
    prefetch_write(gather_args->idx);
    prefetch_read((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1)
    {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;
        memcpy(gather_args->dst + global_index_cur * width,
                gather_args->src + gather_args->idx[global_index_cur] * width,
                width);

        bool has_next = (next_i < len);
        prefetch_read(has_next ? gather_args->idx + global_index_next : NULL);
        prefetch_read(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
        prefetch_write(has_next ? gather_args->dst + global_index_next * width : NULL);
    }
}

inline static int __sequential_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(width)

    prefetch_read(src);
    prefetch_read(idx);
    prefetch_write(dst);

    prefetch_read(idx);
    prefetch_write(dst);
    prefetch_read((dst_src_len > 0) ? src + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < dst_src_len; next_i = ++i + 1)
    {
        memcpy(dst + i * width, src + idx[i] * width, width);

        bool has_next = (next_i < dst_src_len);
        prefetch_read(has_next ? idx + next_i : NULL);
        prefetch_read(has_next ? src + idx[next_i] * width : NULL);
        prefetch_write(has_next ? dst + next_i * width : NULL);
    }

    return STATUS_OK;
}

inline static int __parallel_gather(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t dst_src_len)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(width)

    prefetch_read(src);
    prefetch_read(idx);
    prefetch_write(dst);

    __gather_scatter_args_t args = {
            .idx           = idx,
            .src           = src,
            .dst           = dst,
    };
    return __parallel_for(dst, width, dst_src_len, __gather_func, &args);
}

inline static int __sequential_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(src_width)

    prefetch_read(src);
    prefetch_read(idx);
    prefetch_write(dst);

    prefetch_read(idx);
    prefetch_write(dst);
    prefetch_read(num > 0 ? src + idx[0] * src_width : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1)
    {
        const void *ptr = src + idx[i] * src_width;
        size_t adr = (size_t) ptr;
        memcpy(dst + i * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < num);
        prefetch_read(has_next ? idx + next_i : NULL);
        prefetch_read(has_next ? src + idx[next_i] * src_width : NULL);
        prefetch_write(has_next ? dst + next_i * sizeof(void *) : NULL);
    }
    return STATUS_OK;
}

inline static void __gather_adr_func(const void *restrict start, size_t width, size_t len, void *restrict args)
{
    cast(__gather_scatter_args_t *, gather_args, args);

    prefetch_read(gather_args->idx);
    prefetch_write(gather_args->dst);
    prefetch_read((len > 0) ? gather_args->src + gather_args->idx[0] * width : NULL);

    size_t global_index_start = (start - gather_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1)
    {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;
        const void *ptr = gather_args->src + gather_args->idx[global_index_cur] * width;
        size_t adr = (size_t) ptr;
        memcpy(gather_args->dst + global_index_cur * sizeof(void *), &adr, sizeof(size_t));

        bool has_next = (next_i < len);
        prefetch_read(has_next ? gather_args->idx + global_index_next: NULL);
        prefetch_read(has_next ? gather_args->src + gather_args->idx[global_index_next] * width : NULL);
        prefetch_write(has_next ? gather_args->dst + global_index_next * sizeof(void *) : NULL);
    }
}

inline static int __parallel_gather_adr(void *restrict dst, const void *restrict src, size_t src_width,
        const size_t *restrict idx, size_t num)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(src_width)

    prefetch_read(src);
    prefetch_read(idx);
    prefetch_write(dst);

    __gather_scatter_args_t args = {
            .idx = idx,
            .src = src,
            .dst = dst
    };
    return __parallel_for(dst, src_width, num, __gather_adr_func, &args);
}

//  B O L S T E R   S C A T T E R  -------------------------------------------------------------------------------------

inline static void __scatter_func(const void *restrict start, size_t width, size_t len, void *restrict args)
{
    cast(__gather_scatter_args_t *, scatter_args, args);

    prefetch_read(scatter_args->idx);
    prefetch_read(scatter_args->src);
    prefetch_write((len > 0) ? scatter_args->dst + scatter_args->idx[0] * width : NULL);

    size_t global_index_start = (start - scatter_args->dst) / width;
    for (register size_t i = 0, next_i = 1; i < len; next_i = ++i + 1)
    {
        size_t global_index_cur = global_index_start + i;
        size_t global_index_next = global_index_start + next_i;

        memcpy(scatter_args->dst + scatter_args->idx[global_index_cur] * width,
                scatter_args->src + global_index_cur * width,
                width);

        bool has_next = (next_i < len);
        prefetch_read(has_next ? scatter_args->idx + global_index_next : NULL);
        prefetch_read(has_next ? scatter_args->src + global_index_next * width : NULL);
        prefetch_write(has_next ? scatter_args->dst + scatter_args->idx[global_index_next] * width : NULL);
    }
}

inline static int __sequential_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(width)

    prefetch_read(idx);
    prefetch_read(src);
    prefetch_write((num > 0) ? dst + idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < num; next_i = ++i + 1)
    {
        memcpy(dst + idx[i] * width, src + i * width, width);

        bool has_next = (next_i < num);
        prefetch_read(has_next ? idx + next_i : NULL);
        prefetch_read(has_next ? src + next_i * width : NULL);
        prefetch_write(has_next ? dst + idx[next_i] * width : NULL);
    }
    return STATUS_OK;
}

inline static int __parallel_scatter(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict idx, size_t num)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(idx)
    check_non_null(width)

    prefetch_read(src);
    prefetch_read(idx);
    prefetch_write(dst);

    __gather_scatter_args_t args = {
            .idx = idx,
            .src = src,
            .dst = dst
    };
    return __parallel_for(dst, width, num, __scatter_func, &args);
}

//  B O L S T E R   S H U F F L E  -------------------------------------------------------------------------------------

inline static int __sequential_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len)
{
    check_non_null(dst)
    check_non_null(src)
    check_non_null(dst_idx)
    check_non_null(src_idx)
    check_non_null(width)

    bool has_first = (idx_len > 0);
    prefetch_read(src_idx);
    prefetch_read(dst_idx);
    prefetch_read(has_first ? src + src_idx[0] * width : NULL);
    prefetch_write(has_first ? dst + dst_idx[0] * width : NULL);

    for (register size_t i = 0, next_i = 1; i < idx_len; next_i = ++i + 1)
    {
        memcpy(dst + dst_idx[i] * width, src + src_idx[i] * width, width);

        bool has_next = (next_i < idx_len);
        prefetch_read(has_next ? src_idx + next_i : NULL);
        prefetch_read(has_next ? dst_idx + next_i : NULL);
        prefetch_read(has_next ? src + src_idx[next_i] * width : NULL);
        prefetch_write(has_next ? dst + dst_idx[next_i] * width : NULL);
    }

    return STATUS_OK;
}

inline static int __parallel_shuffle(void *restrict dst, const void *restrict src, size_t width,
        const size_t *restrict dst_idx, const size_t *restrict src_idx,
        size_t idx_len)
{
    unused(dst);
    unused(src);
    unused(width);
    unused(dst_idx);
    unused(src_idx);
    unused(idx_len);
    NOT_YET_IMPLEMENTED
}

//  B O L S T E R   F I L T E R  ---------------------------------------------------------------------------------------

typedef struct __filter_arg_t
{
  size_t               num_positions;
  size_t *restrict     src_positions;
  const void *restrict start;
  size_t               len;
  size_t               width;
  void *restrict       args;
  pred_func_t    pred;
  size_t               position_offset_to_add;
} __filter_arg_t;


inline static int __sequential_filter_late(size_t *restrict pos, size_t *restrict num_pos,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args)
{
    check_non_null(pos);
    check_non_null(num_pos);
    check_non_null(src);
    check_non_null(width);
    check_non_null(len);
    check_non_null(pred);

    pred(pos, num_pos, src, width, len, args, 0);

    return STATUS_OK;
}

inline static void *__filter_proxy_func(void *restrict args)
{
    cast(__filter_arg_t *restrict, proxy_arg, args);
    proxy_arg->pred(proxy_arg->src_positions, &proxy_arg->num_positions,
            proxy_arg->start, proxy_arg->width, proxy_arg->len, proxy_arg->args,
            proxy_arg->position_offset_to_add);
    return NULL;
}

inline static int __parallel_filter_late(size_t *restrict pos, size_t *restrict num_pos,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args, size_t num_threads)
{
    check_non_null(pos);
    check_non_null(num_pos);
    check_non_null(src);
    check_non_null(width);
    check_non_null(pred);

    if (unlikely(len == 0)) {
        *num_pos = 0;
        return STATUS_OK;
    }

    uint_fast16_t          num_thread                  = num_threads + 1; /* +1 since one is this thread */

    pthread_t              threads[num_threads];
    __filter_arg_t         thread_args[num_thread];

    register size_t        chunk_len                   = len / num_thread;
    size_t                 chunk_len_remain            = len % num_thread;
    size_t                 main_position_offset_to_add = num_threads * chunk_len;
    const void *restrict   main_thread_base            = src + main_position_offset_to_add * width;

    prefetch_read(pred);
    prefetch_read(args);

    /* run f on NTHREADS_FOR additional threads */
    if (likely(chunk_len > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++)
        {
            __filter_arg_t *arg    = thread_args + tid;
            arg->num_positions          = 0;
            arg->src_positions          = malloc(chunk_len * sizeof(size_t));
            arg->position_offset_to_add = tid * chunk_len;
            arg->start                  = src + arg->position_offset_to_add * width;
            arg->len                    = chunk_len;
            arg->width                  = width;
            arg->args                   = args;
            arg->pred                   = pred;

            prefetch_read(arg->start);
            pthread_create(threads + tid, NULL, __filter_proxy_func, arg);
        }
    }
    /* run f on this thread */
    prefetch_read(main_thread_base);
    size_t main_chunk_len = chunk_len + chunk_len_remain;
    size_t *main_src_positions = malloc(main_chunk_len * sizeof(size_t));
    size_t main_num_positions = 0;

    pred(main_src_positions, &main_num_positions, main_thread_base, width, main_chunk_len, args, main_position_offset_to_add);

    size_t total_num_matching_positions = 0;

    if (likely(chunk_len > 0)) {
        for (register uint_fast16_t tid = 0; tid < num_threads; tid++)
        {
            pthread_join(threads[tid], NULL);
            const __filter_arg_t *restrict thread_arg = (thread_args + tid);
            if (thread_arg->num_positions > 0) {
                memcpy(pos + total_num_matching_positions, thread_arg->src_positions,
                        thread_arg->num_positions * sizeof(size_t));
                total_num_matching_positions += thread_arg->num_positions;
            }
            free (thread_args[tid].src_positions);
        }
    }

    if (likely(main_num_positions > 0))
    {
        memcpy(pos + total_num_matching_positions, main_src_positions,
                main_num_positions * sizeof(size_t));
        total_num_matching_positions += main_num_positions;
    }
    free (main_src_positions);

    *num_pos = total_num_matching_positions;

    return STATUS_OK;
}

inline static int __sequential_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args)
{
    check_non_null(result);
    check_non_null(result_size);
    check_non_null(src);
    check_non_null(width);
    check_non_null(len);
    check_non_null(pred);

    size_t num_matching_positions;
    size_t *restrict matching_positions = malloc(len * sizeof(size_t));

    pred(matching_positions, &num_matching_positions, src, width, len, args, 0);

    bolster_gather(result, src, width, matching_positions, num_matching_positions, threading_hint_single);
    *result_size = num_matching_positions;

    free (matching_positions);

    return STATUS_OK;
}

inline static int __parallel_filter_early(void *restrict result, size_t *restrict result_size,
        const void *restrict src, size_t width, size_t len, pred_func_t pred,
        void *restrict args)
{
    check_non_null(result);
    check_non_null(result_size);
    check_non_null(src);
    check_non_null(width);
    check_non_null(len);
    check_non_null(pred);

    uint_fast16_t          num_thread                  = BOLSTER_NTHREADS + 1; /* +1 since one is this thread */

    pthread_t              threads[BOLSTER_NTHREADS];
    __filter_arg_t         thread_args[num_thread];

    register size_t        chunk_len                   = len / num_thread;
    size_t                 chunk_len_remain            = len % num_thread;
    size_t                 main_position_offset_to_add = BOLSTER_NTHREADS * chunk_len;
    const void *restrict   main_thread_base            = src + main_position_offset_to_add * width;

    prefetch_read(pred);
    prefetch_read(args);

    /* run f on NTHREADS_FOR additional threads */
    for (register uint_fast16_t tid = 0; tid < BOLSTER_NTHREADS; tid++)
    {
        __filter_arg_t *arg = thread_args + tid;
        arg->num_positions          = 0;
        arg->src_positions          = malloc(chunk_len * sizeof(size_t));
        arg->position_offset_to_add = tid * chunk_len;
        arg->start                  = src + arg->position_offset_to_add * width;
        arg->len                    = chunk_len;
        arg->width                  = width;
        arg->args                   = args;
        arg->pred                   = pred;

        prefetch_read(arg->start);
        pthread_create(threads + tid, NULL, __filter_proxy_func, arg);
    }
    /* run f on this thread */
    prefetch_read(main_thread_base);
    size_t main_chunk_len = chunk_len + chunk_len_remain;
    size_t *main_src_positions = malloc(main_chunk_len * sizeof(size_t));
    size_t main_num_positions = 0;

    pred(main_src_positions, &main_num_positions, main_thread_base, width, main_chunk_len, args,
            main_position_offset_to_add);


    size_t total_num_matching_positions = main_num_positions;
    size_t partial_num_matching_positions = 0;

    for (register uint_fast16_t tid = 0; tid < BOLSTER_NTHREADS; tid++)
    {
        pthread_join(threads[tid], NULL);
        const __filter_arg_t *restrict thread_arg = (thread_args + tid);
        total_num_matching_positions += thread_arg->num_positions;
        prefetch_read(thread_arg->src_positions);
    }

    for (register uint_fast16_t tid = 0; tid < BOLSTER_NTHREADS; tid++)
    {
        const __filter_arg_t *restrict thread_arg = (thread_args + tid);

        if (likely(thread_arg->num_positions > 0))
        {
            bolster_gather(result + partial_num_matching_positions * width, src, width, thread_arg->src_positions,
                    thread_arg->num_positions,
                    threading_hint_multi);
        }

        partial_num_matching_positions += thread_arg->num_positions;
        free (thread_arg->src_positions);
    }

    if (likely(main_num_positions > 0))
    {
        bolster_gather(result + partial_num_matching_positions * width, src, width, main_src_positions,
                main_num_positions, threading_hint_multi);
    }
    free (main_src_positions);

    *result_size = total_num_matching_positions;

    return STATUS_OK;
}

__END_DECLS

#endif //BOLSTER_H
