
#include <stdx/ng5_string_dic_sync.h>
#include <stdx/ng5_string_dic_async.h>
#include <stdx/ng5_vector.h>
#include <stdx/ng5_spinlock.h>
#include <stdx/ng5_string_map.h>
#include <stdx/ng5_time.h>
#include <stdx/ng5_bolster.h>
#include <stdx/ng5_slice_list.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

#define STRING_DIC_ASYNC_TAG "str_dic_async"

// ---------------------------------------------------------------------------------------------------------------------
//
//  S T A T I C   C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

#define hash_func                  hash_sax

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct async_extra async_extra_t;

typedef struct carrier
{
    string_dic_t                  local_dict;
    pthread_t                     thread;
    size_t                        id;
} carrier_t;

typedef struct async_extra
{
    ng5_vector_t of_type(carrier_t)   carriers;
    ng5_vector_t of_type(carrier_t *) carrier_mapping;
    ng5_spinlock_t                    spinlock;

} async_extra_t;

typedef struct carrier_insert_arg_t
{
    ng5_vector_t of_type(char *) strings;
    string_id_t                  *out;
    carrier_t                    *carrier;
    bool                          write_out;
    bool                          did_work;
    uint_fast16_t                 insert_nthreads;
} carrier_insert_arg_t;

typedef struct carrier_remove_arg_t
{
  ng5_vector_t of_type(string_id_t) *local_ids;
  carrier_t                         *carrier;
  int                                result;
  bool                               did_work;
} carrier_remove_arg_t;

typedef struct carrier_locate_arg_t
{

  carrier_t                         *carrier;
  string_id_t                       *out_ids;
  bool                              *out_found_mask;
  size_t                             out_num_not_found;
  ng5_vector_t of_type(char *)       in_keys;
  int                                result;
  bool                               did_work;
} carrier_locate_arg_t;

typedef struct carrier_extract_arg_t
{
  ng5_vector_t of_type(string_id_t)   in_local_ids;
  char                              **out_strings;
  carrier_t                          *carrier;
  bool                                did_work;
} carrier_extract_arg_t;

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define hashcode_of(string)                                     \
    hash_func(strlen(string), string)

#define string_id_make_global(thread_id, thread_local_id)       \
    ((thread_id << 54) | thread_local_id)

#define string_id_get_owner_of_global(global_id)                \
    (global_id >> 54)

#define string_id_get_local_id_of_global(global_id)             \
    ((~((string_id_t) 0)) >> 10 & global_string_id);


// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int async_drop(struct string_dic *self);
static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings, size_t nthreads);
static int async_remove(struct string_dic *self, string_id_t *strings, size_t num_strings);
static int async_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys);
static int async_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys);
static char **async_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids);
static int async_free(struct string_dic *self, void *ptr);

static int async_reset_counters(struct string_dic *self);
static int async_counters(struct string_dic *self, struct string_map_counters *counters);

static int async_lock(struct string_dic *self);
static int async_unlock(struct string_dic *self);

static int async_extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t approx_num_unique_str, size_t nthreads);

static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t approx_num_unique_str, size_t nthreads);

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define async_extra_get(self)                       \
({                                                  \
    check_tag(self->tag, STRING_DIC_ASYNC);         \
    (async_extra_t *) self->extra;                  \
})

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int string_dic_create_async(struct string_dic* dic, size_t capacity, size_t num_index_buckets,
        size_t approx_num_unique_str, size_t nthreads, const ng5_allocator_t* alloc)
{
    check_success(allocator_this_or_default(&dic->alloc, alloc));

    dic->tag            = STRING_DIC_ASYNC;
    dic->drop           = async_drop;
    dic->insert         = async_insert;
    dic->remove         = async_remove;
    dic->locate_safe    = async_locate_safe;
    dic->locate_fast    = async_locate_fast;
    dic->extract        = async_extract;
    dic->free           = async_free;
    dic->reset_counters = async_reset_counters;
    dic->counters       = async_counters;

    check_success(async_extra_create(dic, capacity, num_index_buckets, approx_num_unique_str, nthreads));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static int async_extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t approx_num_unique_str, size_t nthreads)
{
    assert(self);

    self->extra          = allocator_malloc(&self->alloc, sizeof(async_extra_t));
    async_extra_t *extra = async_extra_get(self);
    ng5_spinlock_create(&extra->spinlock);
    ng5_vector_create(&extra->carriers, &self->alloc, sizeof(carrier_t), nthreads);
    async_setup_carriers(self, capacity, num_index_buckets, approx_num_unique_str, nthreads);
    ng5_vector_create(&extra->carrier_mapping, &self->alloc, sizeof(carrier_t*), capacity);

    return STATUS_OK;
}

static int async_drop(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);
    async_extra_t *extra = async_extra_get(self);
    for (size_t i = 0; i < extra->carriers.num_elems; i++) {
        carrier_t *carrier = ng5_vector_get(&extra->carriers, i, carrier_t);
        string_dic_drop(&carrier->local_dict);
    }
    check_success(ng5_vector_drop(&extra->carriers));
    check_success(ng5_vector_drop(&extra->carrier_mapping));
    check_success(allocator_free(&self->alloc, extra));
    return STATUS_OK;
}

void *carrier_remove_func(void *args)
{
    carrier_remove_arg_t *carrier_arg = (carrier_remove_arg_t *) args;
    string_id_t           len         = ng5_vector_len(carrier_arg->local_ids);
    carrier_arg->did_work             = len > 0;

    debug(STRING_DIC_ASYNC_TAG, "thread %zu spawned for remove task (%zu elements)", carrier_arg->carrier->id,
            ng5_vector_len(carrier_arg->local_ids));
    if (len > 0) {
        struct string_dic    *dic         = &carrier_arg->carrier->local_dict;
        string_id_t          *ids         = ng5_vector_all(carrier_arg->local_ids, string_id_t);
        carrier_arg->result               = string_dic_remove(dic, ids, len);
        debug(STRING_DIC_ASYNC_TAG, "thread %zu task done", carrier_arg->carrier->id);
    } else {
        carrier_arg->result               = STATUS_OK;
        warn(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", carrier_arg->carrier->id);
    }

    return NULL;
}

void *carrier_insert_func(void *args)
{
    carrier_insert_arg_t * restrict this_args = (carrier_insert_arg_t * restrict) args;
    this_args->did_work             = this_args->strings.num_elems > 0;

    trace(STRING_DIC_ASYNC_TAG, "thread-local insert function started (thread %zu)", this_args->carrier->id);
    debug(STRING_DIC_ASYNC_TAG, "thread %zu spawned for insert task (%zu elements)", this_args->carrier->id,
            ng5_vector_len(&this_args->strings));

    if (this_args->did_work) {
        trace(STRING_DIC_ASYNC_TAG, "thread %zu starts insertion of %zu strings", this_args->carrier->id, ng5_vector_len(&this_args->strings));
        char** data = (char**) ng5_vector_data(&this_args->strings);

        int status = string_dic_insert(&this_args->carrier->local_dict,
                this_args->write_out ? &this_args->out : NULL,
                data, ng5_vector_len(&this_args->strings), this_args->insert_nthreads);

        panic_if(status!=STATUS_OK, "internal error during thread-local string dictionary building process");
        debug(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    } else {
        warn(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

void *carrier_locate_safe_func(void *args)
{
    carrier_locate_arg_t * restrict this_args = (carrier_locate_arg_t * restrict) args;
    this_args->did_work                       = ng5_vector_len(&this_args->in_keys) > 0;

    trace(STRING_DIC_ASYNC_TAG, "thread-local 'locate' function invoked for thread %zu...", this_args->carrier->id)

    debug(STRING_DIC_ASYNC_TAG, "thread %zu spawned for locate (safe) task (%zu elements)", this_args->carrier->id,
            ng5_vector_len(&this_args->in_keys));

    if (this_args->did_work) {
        this_args->result = string_dic_locate_safe(&this_args->out_ids,
                &this_args->out_found_mask,
                &this_args->out_num_not_found,
                &this_args->carrier->local_dict,
                ng5_vector_all(&this_args->in_keys, char *),
                ng5_vector_len(&this_args->in_keys));

        debug(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    } else {
        warn(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

void *carrier_extract_func(void *args)
{
    carrier_extract_arg_t * restrict this_args = (carrier_extract_arg_t * restrict) args;
    this_args->did_work                        = ng5_vector_len(&this_args->in_local_ids) > 0;

    debug(STRING_DIC_ASYNC_TAG, "thread %zu spawned for extract task (%zu elements)", this_args->carrier->id,
            ng5_vector_len(&this_args->in_local_ids));

    if (this_args->did_work) {
        this_args->out_strings = string_dic_extract(&this_args->carrier->local_dict,
                                    ng5_vector_all(&this_args->in_local_ids, string_id_t),
                                    ng5_vector_len(&this_args->in_local_ids));
        debug(STRING_DIC_ASYNC_TAG, "thread %zu done", this_args->carrier->id);
    } else {
        warn(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", this_args->carrier->id);
    }

    return NULL;
}

static void async_sync(ng5_vector_t of_type(carrier_t) *carriers, size_t nthreads)
{
    debug(STRING_DIC_ASYNC_TAG, "barrier installed for %d threads", nthreads);

    timestamp_t begin = time_current_time_ms();
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        volatile carrier_t *carrier = ng5_vector_get(carriers, thread_id, carrier_t);
        pthread_join(carrier->thread, NULL);
        debug(STRING_DIC_ASYNC_TAG, "thread %d joined", carrier->id);
    }
    timestamp_t end = time_current_time_ms();
    timestamp_t duration = (end - begin);
    unused(duration);

    debug(STRING_DIC_ASYNC_TAG, "barrier passed for %d threads after %f seconds", nthreads, duration/1000.0f);
}

static void thread_assign_create(atomic_uint_fast16_t **str_carrier_mapping, atomic_size_t **carrier_nstrings,
        size_t **str_carrier_idx_mapping,
        ng5_allocator_t *alloc, size_t num_strings, size_t nthreads)
{
    /* map string depending on hash value to a particular carrier */
    *str_carrier_mapping      = allocator_malloc(alloc, num_strings * sizeof(atomic_uint_fast16_t));
    memset(*str_carrier_mapping, 0, num_strings * sizeof(atomic_uint_fast16_t));

    /* counters to compute how many strings go to a particular carrier */
    *carrier_nstrings         = allocator_malloc(alloc, nthreads * sizeof(atomic_size_t));
    memset(*carrier_nstrings, 0, nthreads * sizeof(atomic_size_t));

    /* an inverted index that contains the i-th position for string k that was assigned to carrier m.
     * With this, given a (global) string and and its carrier, one can have directly the position of the
     * string in the carriers "thread-local locate" args */
    *str_carrier_idx_mapping = allocator_malloc(alloc, num_strings * sizeof(size_t));
}

static void thread_assign_drop(ng5_allocator_t *alloc, atomic_uint_fast16_t *str_carrier_mapping,
        atomic_size_t *carrier_nstrings, size_t *str_carrier_idx_mapping)
{
    allocator_free(alloc, carrier_nstrings);
    allocator_free(alloc, str_carrier_mapping);
    allocator_free(alloc, str_carrier_idx_mapping);
}

typedef struct compute_thread_assign_parallel_func_args_t
{
    atomic_uint_fast16_t *str_carrier_mapping;
    size_t                nthreads;
    atomic_size_t        *carrier_nstrings;
    char * const*         base_strings;
} compute_thread_assign_parallel_func_args_t;

static void compute_thread_assign_parallel_func(const void *restrict start, size_t width, size_t len,
        void *restrict args, thread_id_t tid)
{
    unused(tid);
    unused(width);

    char * const*strings = (char * const*) start;

    compute_thread_assign_parallel_func_args_t *func_args = (compute_thread_assign_parallel_func_args_t *) args;

    while (len--) {
        size_t          i         = strings - func_args->base_strings;
        const char     *key       = *strings;
        /* re-using this hashcode for the thread-local dictionary is more costly than to compute it fresh
         * (due to more I/O with the RAM) */
        size_t          thread_id = hashcode_of(key) % func_args->nthreads;
        atomic_fetch_add(&func_args->str_carrier_mapping[i], thread_id);
        atomic_fetch_add(&func_args->carrier_nstrings[thread_id], 1);
        strings++;
    }
}

static void compute_thread_assign(atomic_uint_fast16_t *str_carrier_mapping, atomic_size_t *carrier_nstrings,
        char * const*strings, size_t num_strings, size_t nthreads)
{
    compute_thread_assign_parallel_func_args_t args = {
        .base_strings        = strings,
        .carrier_nstrings    = carrier_nstrings,
        .nthreads            = nthreads,
        .str_carrier_mapping = str_carrier_mapping
    };
    bolster_for(strings, sizeof(char * const*), num_strings, compute_thread_assign_parallel_func, &args, threading_hint_multi, nthreads);

}

static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings, size_t __nthreads)
{
    timestamp_t begin = time_current_time_ms();
    info(STRING_DIC_ASYNC_TAG, "insert operation invoked: %zu strings in total", num_strings)


    check_tag(self->tag, STRING_DIC_ASYNC);
    panic_if(__nthreads != 0, "parameter 'nthreads' must be set to 0 for async dictionary")

    async_lock(self);

    async_extra_t        *extra                    = async_extra_get(self);
    uint_fast16_t         nthreads                 = ng5_vector_len(&extra->carriers);

    atomic_uint_fast16_t *str_carrier_mapping;
    size_t               *str_carrier_idx_mapping;
    atomic_size_t        *carrier_nstrings;


    thread_assign_create(&str_carrier_mapping, &carrier_nstrings, &str_carrier_idx_mapping,
             &self->alloc, num_strings, nthreads);

    ng5_vector_t of_type(carrier_insert_arg_t *) carrier_args;
    ng5_vector_create(&carrier_args, &self->alloc, sizeof(carrier_insert_arg_t*), nthreads);

    /* compute which carrier is responsible for which string */
    compute_thread_assign(str_carrier_mapping, carrier_nstrings, strings, num_strings, nthreads);

    /* prepare to move string subsets to carriers */
    for (uint_fast16_t i = 0; i < nthreads; i++) {
        carrier_insert_arg_t* entry = allocator_malloc(&self->alloc, sizeof(carrier_insert_arg_t));
        entry->carrier       = ng5_vector_get(&extra->carriers, i, carrier_t);
        entry->insert_nthreads = nthreads;

        ng5_vector_create(&entry->strings, &self->alloc, sizeof(char*), max(1, carrier_nstrings[i]));
        ng5_vector_push(&carrier_args, &entry, 1);
        assert (entry->strings.base != NULL);

        carrier_insert_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, i, carrier_insert_arg_t *);
        carrier_arg->out = NULL;
    }

    /* create per-carrier string subset */
    /* parallizing this makes no sense but waste of resources and energy */
    for (size_t i = 0; i < num_strings; i++) {
        uint_fast16_t  thread_id          = str_carrier_mapping[i];
        carrier_insert_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_insert_arg_t *);
        carrier_arg->write_out            = out != NULL;

        /* store local index of string i inside the thread */
        str_carrier_idx_mapping[i]       = ng5_vector_len(&carrier_arg->strings);

        ng5_vector_push(&carrier_arg->strings, &strings[i], 1);
    }


    /* schedule insert operation per carrier */
    trace(STRING_DIC_ASYNC_TAG, "schedule insert operation to %zu threads", nthreads)
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_insert_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_insert_arg_t *);
        carrier_t   *carrier              = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        trace(STRING_DIC_ASYNC_TAG, "create thread %zu...", thread_id)
        pthread_create(&carrier->thread, NULL, carrier_insert_func, carrier_arg);
        trace(STRING_DIC_ASYNC_TAG, "thread %zu created", thread_id)
    }
    trace(STRING_DIC_ASYNC_TAG, "scheduling done for %zu threads", nthreads)

    /* synchronize */
    trace(STRING_DIC_ASYNC_TAG, "start synchronizing %zu threads", nthreads)
    async_sync(&extra->carriers, nthreads);
    trace(STRING_DIC_ASYNC_TAG, "%zu threads in sync", nthreads)

    /* compute string ids; the string id produced by this implementation is a compound identifier encoding
     * both the owning thread id and the thread-local string id. For this, the returned (global) string identifier
     * is split into 10bits encoded the thread (given a maximum of 1024 threads that can be handled by this
     * implementation), and 54bits used to encode the thread-local string id
     *
     * TECHNICAL LIMIT: 1024 threads
     */

    /* optionally, return the created string ids. In case 'out' is NULL, nothing has to be done (especially
     * none of the carrier threads allocated thread-local 'out's which mean that no cleanup must be done */

    /* parallelizing the following block makes no sense but waste of compute power and energy */
    if (likely(out != NULL)) {
        string_id_t *total_out   = allocator_malloc(&self->alloc, num_strings * sizeof(string_id_t));
        size_t       current_out = 0;

        for (size_t string_idx = 0; string_idx < num_strings; string_idx++) {
            uint_fast16_t         thread_id    = str_carrier_mapping[string_idx];
            size_t                local_idx    = str_carrier_idx_mapping[string_idx];
            carrier_insert_arg_t *carrier_arg  = *ng5_vector_get(&carrier_args, thread_id, carrier_insert_arg_t *);
            string_id_t global_string_owner_id = thread_id;
            string_id_t global_string_local_id = carrier_arg->out[local_idx];
            string_id_t global_string_id       = string_id_make_global(global_string_owner_id, global_string_local_id);
            total_out[current_out++]           = global_string_id;
        }

        *out = total_out;
    }

    /* cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_insert_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_insert_arg_t *);
        if (carrier_arg->did_work) {
            string_dic_free(&carrier_arg->carrier->local_dict, carrier_arg->out);
        }
        ng5_vector_drop(&carrier_arg->strings);
        allocator_free(&self->alloc, carrier_arg);
    }

    /* cleanup */
    thread_assign_drop(&self->alloc, str_carrier_mapping, carrier_nstrings, str_carrier_idx_mapping);
    ng5_vector_drop(&carrier_args);

    async_unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    info(STRING_DIC_ASYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

    return STATUS_OK;
}

static int async_remove(struct string_dic *self, string_id_t *strings, size_t num_strings)
{
    timestamp_t begin = time_current_time_ms();
    info(STRING_DIC_ASYNC_TAG, "remove operation started: %zu strings to remove", num_strings);

    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    carrier_remove_arg_t empty;
    struct async_extra  *extra                   = async_extra_get(self);
    uint_fast16_t        nthreads                = ng5_vector_len(&extra->carriers);
    size_t               est_nstrings_per_thread = max(1, num_strings/nthreads);
    ng5_vector_t of_type(string_id_t) *str_map  = allocator_malloc(&self->alloc, nthreads * sizeof(ng5_vector_t));

    ng5_vector_t of_type(carrier_remove_arg_t) carrier_args;
    ng5_vector_create(&carrier_args, &self->alloc, sizeof(carrier_remove_arg_t), nthreads);

    /* prepare thread-local subset of string ids */
    ng5_vector_repreat_push(&carrier_args, &empty, nthreads);
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        ng5_vector_create(str_map + thread_id, &self->alloc, sizeof(string_id_t), est_nstrings_per_thread);
    }

    /* compute subset of string ids per thread  */
    for (size_t i = 0; i < num_strings; i++) {
        string_id_t   global_string_id   = strings[i];
        uint_fast16_t owning_thread_id   = string_id_get_owner_of_global(global_string_id);
        string_id_t   local_string_id    = string_id_get_local_id_of_global(global_string_id);
        assert(owning_thread_id < nthreads);

        ng5_vector_push(str_map + owning_thread_id, &local_string_id, 1);
    }

    /* schedule remove operation per carrier */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t            *carrier                = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        carrier_remove_arg_t *carrier_arg            = ng5_vector_get(&carrier_args, thread_id, carrier_remove_arg_t);
        carrier_arg->carrier                         = carrier;
        carrier_arg->local_ids                       = str_map + thread_id;

        pthread_create(&carrier->thread, NULL, carrier_remove_func, carrier_arg);
    }

    /* synchronize */
    async_sync(&extra->carriers, nthreads);

    /* cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        ng5_vector_drop(str_map + thread_id);
    }

    allocator_free(&self->alloc, str_map);
    ng5_vector_data(&carrier_args);

    async_unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    info(STRING_DIC_ASYNC_TAG, "remove operation done: %f seconds spent here", (end - begin)/1000.0f)

    return STATUS_OK;
}

static int async_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys)
{
    timestamp_t begin = time_current_time_ms();
    info(STRING_DIC_ASYNC_TAG, "locate (safe) operation started: %zu strings to locate", num_keys)

    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    struct async_extra  *extra                   = async_extra_get(self);
    uint_fast16_t        nthreads                = ng5_vector_len(&extra->carriers);

    /* global result output */
    NG5_HEAP_ALLOC(string_id_t, global_out,        num_keys, &self->alloc);
    NG5_HEAP_ALLOC(bool,        global_found_mask, num_keys, &self->alloc);

    size_t global_num_not_found   = 0;

    atomic_uint_fast16_t *str_carrier_mapping;
    size_t               *str_carrier_idx_mapping;
    atomic_size_t        *carrier_nstrings;

    carrier_locate_arg_t carrier_args[nthreads];

    thread_assign_create(&str_carrier_mapping, &carrier_nstrings, &str_carrier_idx_mapping,
            &self->alloc, num_keys, nthreads);

    /* compute which carrier is responsible for which string */
    compute_thread_assign(str_carrier_mapping, carrier_nstrings, keys, num_keys, nthreads);

    /* prepare to move string subsets to carriers */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_locate_arg_t *arg = carrier_args + thread_id;
        ng5_vector_create(&arg->in_keys, &self->alloc, sizeof(char*), carrier_nstrings[thread_id]);
        assert (&arg->in_keys.base != NULL);
    }

    trace(STRING_DIC_ASYNC_TAG, "computing per-thread string subset for %zu strings", num_keys)
    /* create per-carrier string subset */
    for (size_t i = 0; i < num_keys; i++) {
        /* get thread responsible for this particular string */
        uint_fast16_t         thread_id  = str_carrier_mapping[i];

        /* get the thread-local argument list for the thread that is responsible for this particular string */
        carrier_locate_arg_t *arg        = carrier_args + thread_id;

        /* store local index of string i inside the thread */
        str_carrier_idx_mapping[i]       = ng5_vector_len(&arg->in_keys);

        /* push that string into the thread-local vector */
        ng5_vector_push(&arg->in_keys, &keys[i], 1);
    }

    trace(STRING_DIC_ASYNC_TAG, "schedule operation to threads to %zu threads...", nthreads)
    /* schedule operation to threads */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t            *carrier    = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        carrier_locate_arg_t *arg        = carrier_args + thread_id;

        carrier_args[thread_id].carrier   = carrier;
        pthread_create(&carrier->thread, NULL, carrier_locate_safe_func, arg);
    }

    /* synchronize */
    trace(STRING_DIC_ASYNC_TAG, "start syncing %zu threads...", nthreads)
    async_sync(&extra->carriers, nthreads);
    trace(STRING_DIC_ASYNC_TAG, "%zu threads in sync.", nthreads)

    /* collect and merge results */
    trace(STRING_DIC_ASYNC_TAG, "merging results of %zu threads", nthreads)
    for (size_t i = 0; i < num_keys; i++) {
        /* get thread responsible for this particular string, and local position of that string inside the
         * thread storage */
        uint_fast16_t  thread_id        = str_carrier_mapping[i];
        size_t         thread_local_idx = str_carrier_idx_mapping[i];

        /* get the thread-local argument list for the thread that is responsible for this particular string */
        carrier_locate_arg_t *arg        = carrier_args + thread_id;

        /* merge into global result */
        string_id_t    global_string_id_owner     = thread_id;
        string_id_t    global_string_id_local_idx = arg->out_ids[thread_local_idx];
        string_id_t    global_string_id           = string_id_make_global(global_string_id_owner, global_string_id_local_idx);
        global_out[i]                             = global_string_id;
        global_found_mask[i]                      = arg->out_found_mask[thread_local_idx];
    }
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        /* compute total number of not-found elements */
        carrier_locate_arg_t *arg        = carrier_args + thread_id;

        global_num_not_found += arg->out_num_not_found;

        /* cleanup */
        if (likely(arg->did_work)) {
            string_dic_free(&arg->carrier->local_dict, arg->out_found_mask);
            string_dic_free(&arg->carrier->local_dict, arg->out_ids);
        }
    }

    trace(STRING_DIC_ASYNC_TAG, "cleanup%s", "...")

    /* cleanup */
    thread_assign_drop(&self->alloc, str_carrier_mapping, carrier_nstrings, str_carrier_idx_mapping);

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_locate_arg_t *arg        = carrier_args + thread_id;
        ng5_vector_drop(&arg->in_keys);
    }

    /* return results */
    *out           = global_out;
    *found_mask    = global_found_mask;
    *num_not_found = global_num_not_found;

    async_unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    info(STRING_DIC_ASYNC_TAG, "locate (safe) operation done: %f seconds spent here", (end - begin)/1000.0f)

    return STATUS_OK;
}

static int async_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    bool   *found_mask;
    size_t  num_not_found;
    int     result;

    /* use safer but in principle more slower implementation */
    result = async_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /* cleanup */
    async_free(self, found_mask);

    async_unlock(self);

    return result;
}

static char **async_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids)
{
    timestamp_t begin = time_current_time_ms();
    info(STRING_DIC_ASYNC_TAG, "extract (safe) operation started: %zu strings to extract", num_ids)

    if (self->tag != STRING_DIC_ASYNC) {
        return NULL;
    }

    async_lock(self);

    NG5_HEAP_ALLOC(char *, global_result, num_ids, &self->alloc);

    struct async_extra   *extra                   = (struct async_extra *) self->extra;
    uint_fast16_t         nthreads                = ng5_vector_len(&extra->carriers);
    size_t                est_nstrings_per_thread = max(1, num_ids/nthreads);

    NG5_HEAP_ALLOC(size_t, thread_local_idx, num_ids, &self->alloc);
    NG5_HEAP_ALLOC(uint_fast16_t, owning_thread_ids, num_ids, &self->alloc);
    NG5_HEAP_ALLOC(carrier_extract_arg_t, thread_args, nthreads, &self->alloc);

    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_extract_arg_t *arg = thread_args + thread_id;
        ng5_vector_create(&arg->in_local_ids, &self->alloc, sizeof(string_id_t), est_nstrings_per_thread);
    }

    /* compute subset of string ids per thread  */
    for (size_t i = 0; i < num_ids; i++) {
        string_id_t   global_string_id   = ids[i];
        owning_thread_ids[i]             = string_id_get_owner_of_global(global_string_id);
        string_id_t   local_string_id    = string_id_get_local_id_of_global(global_string_id);
        assert(owning_thread_ids[i] < nthreads);

        carrier_extract_arg_t *arg = thread_args + owning_thread_ids[i];
        thread_local_idx[i] = ng5_vector_len(&arg->in_local_ids);
        ng5_vector_push(&arg->in_local_ids, &local_string_id, 1);
    }

    /* schedule remove operation per carrier */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t             *carrier                = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        carrier_extract_arg_t *carrier_arg            = thread_args + thread_id;
        carrier_arg->carrier                          = carrier;
        pthread_create(&carrier->thread, NULL, carrier_extract_func, carrier_arg);
    }

    /* synchronize */
    async_sync(&extra->carriers, nthreads);

    for (size_t i = 0; i < num_ids; i++) {
        uint_fast16_t          owning_thread_id = owning_thread_ids[i];
        size_t                 local_idx        = thread_local_idx[i];
        carrier_extract_arg_t *carrier_arg      = thread_args + owning_thread_id;
        char                  *extracted_string = carrier_arg->out_strings[local_idx];
        global_result[i]                        = extracted_string;
    }

    /* cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_extract_arg_t *carrier_arg            = thread_args + thread_id;
        ng5_vector_drop(&carrier_arg->in_local_ids);
        if (likely(carrier_arg->did_work)) {
            string_dic_free(&carrier_arg->carrier->local_dict, carrier_arg->out_strings);
        }
    }

    NG5_HEAP_FREE(thread_local_idx, &self->alloc);
    NG5_HEAP_FREE(owning_thread_ids, &self->alloc);
    NG5_HEAP_FREE(thread_args, &self->alloc);

    async_unlock(self);

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    info(STRING_DIC_ASYNC_TAG, "extract (safe) operation done: %f seconds spent here", (end - begin)/1000.0f)

    return global_result;
}

static int async_free(struct string_dic *self, void *ptr)
{
    check_tag(self->tag, STRING_DIC_ASYNC);
    allocator_free(&self->alloc, ptr);
    return STATUS_OK;
}


static int async_reset_counters(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    struct async_extra  *extra                   = async_extra_get(self);
    size_t               nthreads                = ng5_vector_len(&extra->carriers);

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        string_dic_reset_counters(&carrier->local_dict);
    }

    async_unlock(self);

    return STATUS_OK;
}

static int async_counters(struct string_dic *self, struct string_map_counters *counters)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    struct async_extra  *extra                   = async_extra_get(self);
    size_t               nthreads                = ng5_vector_len(&extra->carriers);

    check_success(string_map_counters_init(counters));

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        struct string_map_counters local_counters;
        string_dic_counters(&local_counters, &carrier->local_dict);
        string_map_counters_add(counters, &local_counters);
    }

    async_unlock(self);

    return STATUS_OK;
}

typedef struct carrier_parallel_create_args_t {
  size_t                     local_capacity;
  size_t                     local_bucket_num;
  size_t                     local_bucket_cap;
  const ng5_allocator_t     *alloc;
} carrier_parallel_create_args_t;

static void carrier_parallel_create(const void *restrict start, size_t width, size_t len, void *restrict args,
        thread_id_t tid)
{
    unused(tid);
    unused(width);

    carrier_t *carrier = (carrier_t *) start;
    const carrier_parallel_create_args_t *create_args = (const carrier_parallel_create_args_t *) args;
    while (len--) {
        string_dic_create_sync(&carrier->local_dict, create_args->local_capacity, create_args->local_bucket_num,
                create_args->local_bucket_cap, 0, create_args->alloc);
        memset(&carrier->thread, 0, sizeof(pthread_t));
        carrier++;
    }
}


static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t approx_num_unique_str, size_t nthreads)
{
    async_extra_t *extra               = async_extra_get(self);
    size_t         local_bucket_num    = max(1, num_index_buckets / nthreads);
    carrier_t      new_carrier;

    carrier_parallel_create_args_t create_args = {
            .local_capacity      = max(1, capacity / nthreads),
            .local_bucket_num    = local_bucket_num,
            .local_bucket_cap    = max(1, approx_num_unique_str / nthreads / local_bucket_num / SLICE_KEY_COLUMN_MAX_ELEMS),
            .alloc               = &self->alloc
    };

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        new_carrier.id = thread_id;
        ng5_vector_push(&extra->carriers, &new_carrier, 1);
    }

    bolster_for(ng5_vector_all(&extra->carriers, carrier_t), sizeof(carrier_t), nthreads, carrier_parallel_create,
            &create_args, threading_hint_multi, nthreads);

    return STATUS_OK;
}

static int async_lock(struct string_dic *self)
{
    async_extra_t *extra = async_extra_get(self);
    check_success(ng5_spinlock_lock(&extra->spinlock));
    return STATUS_OK;
}

static int async_unlock(struct string_dic *self)
{
    async_extra_t *extra = async_extra_get(self);
    check_success(ng5_spinlock_unlock(&extra->spinlock));
    return STATUS_OK;
}
