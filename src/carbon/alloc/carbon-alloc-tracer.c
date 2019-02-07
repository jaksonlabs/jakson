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

#include "carbon/carbon-vector.h"

#include "carbon/alloc/carbon-alloc_tracer.h"
#include "carbon/carbon-sort.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-time.h"

#define TRACE_ALLOC_TAG "trace_alloc"

#define TO_GIB(x)       (x/1024.0f/1024.0f/1024.0f)

typedef struct trace_stats_t
{
    size_t num_malloc_calls;
    size_t num_realloc_calls;
    size_t num_free_calls;
    size_t total_size;
    carbon_vec_t ofType(size_t) *malloc_sizes;
    carbon_spinlock_t *spinlock;
    FILE *statistics_file;
    carbon_timestamp_t startup_timestamp;
} trace_stats_t;


#define DEFINE_PAGE_WITH_SIZE(x)                                                                                       \
typedef struct page_##x##b_t                                                                                           \
{                                                                                                                      \
    size_t                        user_size;                                                                           \
    size_t                        capacity;                                                                            \
    char                          data[x];                                                                             \
} page_##x##b_t;                                                                                                       \
                                                                                                                       \
CARBON_FUNC_UNUSED static inline void *page_##x##b_new(size_t user_size) {                                             \
    assert (user_size <= x);                                                                                           \
    struct page_##x##b_t *page = malloc(sizeof(struct page_##x##b_t));                                                 \
    page->user_size = user_size;                                                                                       \
    page->capacity = x;                                                                                                \
    return (((void *)page) + 2 * sizeof(size_t));                                                                      \
}

DEFINE_PAGE_WITH_SIZE(1)

DEFINE_PAGE_WITH_SIZE(2)

DEFINE_PAGE_WITH_SIZE(4)

DEFINE_PAGE_WITH_SIZE(8)

DEFINE_PAGE_WITH_SIZE(16)

DEFINE_PAGE_WITH_SIZE(32)

DEFINE_PAGE_WITH_SIZE(64)

DEFINE_PAGE_WITH_SIZE(128)

DEFINE_PAGE_WITH_SIZE(256)

DEFINE_PAGE_WITH_SIZE(512)

DEFINE_PAGE_WITH_SIZE(1024)

DEFINE_PAGE_WITH_SIZE(2048)

DEFINE_PAGE_WITH_SIZE(4096)

DEFINE_PAGE_WITH_SIZE(8192)

DEFINE_PAGE_WITH_SIZE(16384)

DEFINE_PAGE_WITH_SIZE(32768)

DEFINE_PAGE_WITH_SIZE(65536)

DEFINE_PAGE_WITH_SIZE(131072)

DEFINE_PAGE_WITH_SIZE(262144)

DEFINE_PAGE_WITH_SIZE(524288)

DEFINE_PAGE_WITH_SIZE(1048576)

DEFINE_PAGE_WITH_SIZE(2097152)

DEFINE_PAGE_WITH_SIZE(4194304)

DEFINE_PAGE_WITH_SIZE(8388608)

DEFINE_PAGE_WITH_SIZE(16777216)

DEFINE_PAGE_WITH_SIZE(33554432)

DEFINE_PAGE_WITH_SIZE(67108864)

DEFINE_PAGE_WITH_SIZE(134217728)

DEFINE_PAGE_WITH_SIZE(268435456)

DEFINE_PAGE_WITH_SIZE(536870912)

DEFINE_PAGE_WITH_SIZE(1073741824)

DEFINE_PAGE_WITH_SIZE(2147483648)

DEFINE_PAGE_WITH_SIZE(4294967296)

DEFINE_PAGE_WITH_SIZE(8589934592)

DEFINE_PAGE_WITH_SIZE(17179869184)

DEFINE_PAGE_WITH_SIZE(34359738368)

DEFINE_PAGE_WITH_SIZE(68719476736)

DEFINE_PAGE_WITH_SIZE(137438953472)

DEFINE_PAGE_WITH_SIZE(274877906944)

DEFINE_PAGE_WITH_SIZE(549755813888)

#define REGISTER_PAGE(x)    \
    {x, page_##x##b_new}

struct page_template_entry
{
    size_t size;
    void *(*new_ptr_func)(size_t user_size);
} page_template_register[] = {

    REGISTER_PAGE(1),
    REGISTER_PAGE(2),
    REGISTER_PAGE(4),
    REGISTER_PAGE(8),
    REGISTER_PAGE(16),
    REGISTER_PAGE(32),
    REGISTER_PAGE(64),
    REGISTER_PAGE(128),
    REGISTER_PAGE(256),
    REGISTER_PAGE(512),
    REGISTER_PAGE(1024),
    REGISTER_PAGE(2048),
    REGISTER_PAGE(4096),
    REGISTER_PAGE(8192),
    REGISTER_PAGE(16384),
    REGISTER_PAGE(32768),
    REGISTER_PAGE(65536),
    REGISTER_PAGE(131072),
    REGISTER_PAGE(262144),
    REGISTER_PAGE(524288),
    REGISTER_PAGE(1048576),
    REGISTER_PAGE(2097152),
    REGISTER_PAGE(4194304),
    REGISTER_PAGE(8388608),
    REGISTER_PAGE(16777216),
    REGISTER_PAGE(33554432),
    REGISTER_PAGE(67108864),
    REGISTER_PAGE(134217728),
    REGISTER_PAGE(268435456),
    REGISTER_PAGE(536870912),
    REGISTER_PAGE(1073741824),
    REGISTER_PAGE(2147483648),
    REGISTER_PAGE(4294967296),
    REGISTER_PAGE(8589934592),
    REGISTER_PAGE(17179869184),
    REGISTER_PAGE(34359738368),
    REGISTER_PAGE(68719476736),
    REGISTER_PAGE(137438953472),
    REGISTER_PAGE(274877906944),
    REGISTER_PAGE(549755813888)
};

static inline void *alloc_register(size_t size)
{
    size_t num_options = CARBON_ARRAY_LENGTH(page_template_register);
    for (size_t i = 0; i < num_options; i++) {
        struct page_template_entry *entry = page_template_register + i;
        if (size <= entry->size) {
            return entry->new_ptr_func(size);
        }
    }
    CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_MALLOCERR)
    return NULL;
}

trace_stats_t global_trace_stats = {
    .num_malloc_calls   = 0,
    .num_realloc_calls  = 0,
    .num_free_calls     = 0,
    .total_size         = 0,
    .malloc_sizes       = NULL,
    .spinlock           = NULL
};

static void *invoke_malloc(carbon_alloc_t *self, size_t size);
static void *invoke_realloc(carbon_alloc_t *self, void *ptr, size_t size);
static void invoke_free(carbon_alloc_t *self, void *ptr);
static void invoke_clone(carbon_alloc_t *dst, const carbon_alloc_t *self);

#define LAZY_INIT()                                                                                                    \
if (!global_trace_stats.malloc_sizes) {                                                                                \
    global_trace_stats.malloc_sizes = malloc(sizeof(carbon_vec_t));                                                    \
    carbon_vec_create(global_trace_stats.malloc_sizes, &default_alloc, sizeof(size_t), 1000000);                       \
    global_trace_stats.spinlock = carbon_malloc(&default_alloc, sizeof(carbon_spinlock_t));                            \
    carbon_spinlock_init(global_trace_stats.spinlock);                                                                 \
    global_trace_stats.statistics_file = fopen("trace-alloc-stats.csv", "a");                                          \
    fprintf(global_trace_stats.statistics_file,                                                                        \
            "system_time;num_alloc_calls;num_realloc_calls;num_free_calls;memory_in_use\n");                           \
    global_trace_stats.startup_timestamp = carbon_time_now_wallclock();                                                \
}

#define WRITE_STATS_FILE()                                                                                             \
{                                                                                                                      \
    fprintf(global_trace_stats.statistics_file, "%lld;%zu;%zu;%zu;%zu\n",                                              \
            carbon_time_now_wallclock() - global_trace_stats.startup_timestamp,                                        \
            global_trace_stats.num_malloc_calls,                                                                       \
            global_trace_stats.num_realloc_calls,                                                                      \
            global_trace_stats.num_free_calls,                                                                         \
            global_trace_stats.total_size);                                                                            \
    fflush(global_trace_stats.statistics_file);                                                                        \
}

int carbon_tracer_alloc_create(carbon_alloc_t *alloc)
{
    CARBON_NON_NULL_OR_ERROR(alloc);
    carbon_alloc_t default_alloc;
    carbon_alloc_create_std(&default_alloc);

    alloc->extra = NULL;
    LAZY_INIT();

    alloc->malloc = invoke_malloc;
    alloc->realloc = invoke_realloc;
    alloc->free = invoke_free;
    alloc->clone = invoke_clone;

    return true;
}

static void *invoke_malloc(carbon_alloc_t *self, size_t size)
{
    CARBON_UNUSED(self);

    carbon_spinlock_acquire(global_trace_stats.spinlock);

    carbon_alloc_t default_alloc;
    carbon_alloc_create_std(&default_alloc);

    LAZY_INIT();

    global_trace_stats.num_malloc_calls++;

    //DEBUG(TRACE_ALLOC_TAG, "malloc %zu B, call to malloc: %zu so far (allocator %p)", size,
    //      global_trace_stats.num_malloc_calls, self);
    carbon_vec_push(global_trace_stats.malloc_sizes, &size, 1);

    size_t min_alloc_size = carbon_sort_get_min(CARBON_VECTOR_ALL(global_trace_stats.malloc_sizes, size_t),
                                                carbon_vec_length(global_trace_stats.malloc_sizes));
    size_t max_alloc_size = carbon_sort_get_max(CARBON_VECTOR_ALL(global_trace_stats.malloc_sizes, size_t),
                                                carbon_vec_length(global_trace_stats.malloc_sizes));
    double avg_alloc_size = carbon_sort_get_avg(CARBON_VECTOR_ALL(global_trace_stats.malloc_sizes, size_t),
                                                carbon_vec_length(global_trace_stats.malloc_sizes));
    global_trace_stats.total_size += size;

    CARBON_UNUSED(min_alloc_size);
    CARBON_UNUSED(max_alloc_size);
    CARBON_UNUSED(avg_alloc_size);

    //DEBUG(TRACE_ALLOC_TAG, "min/max/avg alloc size: %zu/%zu/%f B (allocator %p)", min_alloc_size, max_alloc_size,
    //      avg_alloc_size, self);
    //DEBUG(TRACE_ALLOC_TAG, "allocated size in at total: %zu B (%f GiB)", global_trace_stats.total_size,
    //      TO_GIB(global_trace_stats.total_size));

    void *result = alloc_register(size);

    WRITE_STATS_FILE();

    carbon_spinlock_release(global_trace_stats.spinlock);


    return result;
}

static void *invoke_realloc(carbon_alloc_t *self, void *ptr, size_t size)
{
    CARBON_UNUSED(self);

    carbon_spinlock_acquire(global_trace_stats.spinlock);

    carbon_alloc_t default_alloc;
    carbon_alloc_create_std(&default_alloc);

    LAZY_INIT();

    size_t page_capacity = *(size_t *) (ptr - sizeof(size_t));
    size_t page_usersize = *(size_t *) (ptr - 2 * sizeof(size_t));

    global_trace_stats.total_size -= page_usersize;
    global_trace_stats.total_size += size;

    global_trace_stats.num_realloc_calls++;

    //DEBUG(TRACE_ALLOC_TAG, "num realloc calls %zu (allocator %p)", global_trace_stats.num_realloc_calls, self);

    //DEBUG(TRACE_ALLOC_TAG, "allocated size in at total: %zu B (%f GiB)", global_trace_stats.total_size,
    //      TO_GIB(global_trace_stats.total_size));

    WRITE_STATS_FILE();

    if (size <= page_capacity) {
        *(size_t *) (ptr - 2 * sizeof(size_t)) = size;
        carbon_spinlock_release(global_trace_stats.spinlock);
        return ptr;
    }
    else {
        void *page_ptr = ptr - 2 * sizeof(size_t);
        free(page_ptr);
        void *result = alloc_register(size);
        carbon_spinlock_release(global_trace_stats.spinlock);
        return result;
    }
}

static void invoke_free(carbon_alloc_t *self, void *ptr)
{
    CARBON_UNUSED(self);

    carbon_spinlock_acquire(global_trace_stats.spinlock);

    carbon_alloc_t default_alloc;
    carbon_alloc_create_std(&default_alloc);

    void *page_ptr = ptr - 2 * sizeof(size_t);

    global_trace_stats.total_size -= *(size_t *) (page_ptr);

    global_trace_stats.num_free_calls++;

    //DEBUG(TRACE_ALLOC_TAG, "freed %zu B, num free calls %zu (allocator %p)", *(size_t *) (page_ptr),
    //      global_trace_stats.num_free_calls, self);

    //DEBUG(TRACE_ALLOC_TAG, "allocated size in at total: %zu B (%f GiB)", global_trace_stats.total_size,
    //      TO_GIB(global_trace_stats.total_size));

    WRITE_STATS_FILE();

    free(page_ptr);

    carbon_spinlock_release(global_trace_stats.spinlock);
}

static void invoke_clone(carbon_alloc_t *dst, const carbon_alloc_t *self)
{
    *dst = *self;
}
