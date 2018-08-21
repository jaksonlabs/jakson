#include <stdx/string_dics/string_dic_async.h>
#include <stdx/vector.h>
#include <stdx/async.h>
#include <stdx/string_dics/string_dic_naive.h>
#include <apr-1/apr_queue.h>

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

typedef struct carrier
{
    apr_queue_t                  *input;
    string_dic_t                  local_dict;
    pthread_t                     thread;
    size_t                        id;
    atomic_flag                   keep_running;
    atomic_flag                   is_waiting_for_jobs;

    /* lock used to sync all carriers regarding their "waiting for jobs" state */
    spinlock_t                    spinlock;
} carrier_t;

typedef struct async_extra
{
    vector_t of_type(carrier_t)   carriers;
    vector_t of_type(carrier_t *) carrier_mapping;
    spinlock_t                    spinlock;
    apr_pool_t                   *mem_pool;

} async_extra_t;

typedef struct carrier_push_arg_t
{
    vector_t of_type(char *)      strings;
} carrier_push_arg_t;

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define hashcode_of(string)             \
    hash_func(strlen(string), string)

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int async_drop(struct string_dic *self);
static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings);
static int async_remove(struct string_dic *self, string_id_t *strings, size_t num_strings);
static int async_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys);
static int async_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys);
static char **async_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids);
static int async_free(struct string_dic *self, void *ptr);

static int async_reset_counters(struct string_dic *self);
static int async_counters(struct string_dic *self, struct string_lookup_counters *counters);

static int async_lock(struct string_dic *self);
static int async_unlock(struct string_dic *self);
static int carrier_lock(carrier_t *carrier);
static int carrier_unlock(carrier_t *carrier);

static int async_extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);

static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);
static int async_drop_carriers(struct string_dic *self);

static int async_carrier_insert(struct string_dic *self, size_t carrier_id, const carrier_push_arg_t *args);

static int async_carrier_sync(struct string_dic *self);

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
        size_t num_index_bucket_cap, size_t nthreads, const struct allocator* alloc)
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

    check_success(async_extra_create(dic, capacity, num_index_buckets, num_index_bucket_cap, nthreads));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static int async_extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    assert(self);
    self->extra          = allocator_malloc(&self->alloc, sizeof(async_extra_t));
    async_extra_t *extra = async_extra_get(self);
    spinlock_create(&extra->spinlock);
    vector_create(&extra->carriers, &self->alloc, sizeof(carrier_t), nthreads);
    async_setup_carriers(self);
    vector_create(&extra->carrier_mapping, &self->alloc, sizeof(carrier_t *), capacity);

    return STATUS_OK;
}

static int async_drop(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);
    async_extra_t *extra = async_extra_get(self);
    check_success(vector_drop(&extra->carriers));
    check_success(vector_drop(&extra->carrier_mapping));
    check_success(allocator_free(&self->alloc, extra));
    return STATUS_OK;
}

static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    async_extra_t      *extra                    = async_extra_get(self);
    size_t              nthreads                 = vector_len(&extra->carriers);

    /* map string depending on hash value to a particular carrier */
    uint_fast16_t      *str_carrier_mapping      = allocator_malloc(&self->alloc, num_strings * sizeof(uint_fast16_t));

    /* counters to compute how many strings go to a particular carrier */
    size_t             *carrier_reserve_nstrings = allocator_malloc(&self->alloc, nthreads * sizeof(size_t));
    memset(carrier_reserve_nstrings, 0, nthreads * sizeof(size_t));

    vector_t of_type(carrier_push_arg_t *) carrier_args;
    vector_create(&carrier_args, &self->alloc, sizeof(carrier_push_arg_t *), nthreads);

    /* compute which carrier is responsible for which string */
    for (size_t i = 0; i < num_strings; i++) {
        const char     *key       = strings[i];
        size_t          thread_id = hashcode_of(key) % nthreads;
        str_carrier_mapping[i]    = thread_id;
        carrier_reserve_nstrings[thread_id]++;
    }

    /* prepare to move string subsets to carriers */
    for (size_t i = 0; i < nthreads; i++) {
        carrier_push_arg_t *entry = allocator_malloc(&self->alloc, sizeof(carrier_push_arg_t));
        vector_push(&carrier_args, &entry, 1);
        vector_create(&entry->strings, &self->alloc, sizeof(char *), carrier_reserve_nstrings[nthreads]);
    }

    /* create per-carrier string subset */
    for (size_t i = 0; i < num_strings; i++) {
        size_t thread_id                     = str_carrier_mapping[i];
        carrier_push_arg_t *thread_push_args = *(carrier_push_arg_t **) vector_at(&carrier_args, thread_id);
        vector_push(&thread_push_args->strings, strings[i], 1);
    }

    /* schedule insert operation per carrier */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_push_arg_t *thread_push_args = *(carrier_push_arg_t **) vector_at(&carrier_args, thread_id);
        async_carrier_insert(self, thread_id, thread_push_args);
    }

    /* synchronize and wait for carrier to finish execution */
    async_carrier_sync(self);

    /* cleanup */
    allocator_free(&self->alloc, &carrier_reserve_nstrings);
    allocator_free(&self->alloc, &str_carrier_mapping);

    for (size_t i = 0; i < nthreads; i++) {
        carrier_push_arg_t *entry = *(carrier_push_arg_t **) vector_at(&carrier_args, i);
        vector_drop(&entry->strings);
        allocator_free(entry, &self->alloc);
    }

    vector_drop(&carrier_args);

    async_unlock(self);

    return STATUS_OK;
}

static int async_remove(struct string_dic *self, string_id_t *strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static char **async_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_free(struct string_dic *self, void *ptr)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}


static int async_reset_counters(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_counters(struct string_dic *self, struct string_lookup_counters *counters)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

inline static void register_carrier_as_working(carrier_t *self )
{
    atomic_flag_clear(&self->is_waiting_for_jobs);
}

inline static void update_carrier_working_state(carrier_t *self )
{
    apr_status_t        status;
    carrier_push_arg_t *push_arg;

    while ((status = apr_queue_trypop(&self->input, &push_arg)) == APR_EINTR)
    { }

    switch (status) {
    case APR_EAGAIN:
        atomic_flag_test_and_set(&self->is_waiting_for_jobs);
        break;
    case APR_SUCCESS:
        apr_queue_push(&self->input, &push_arg);
        break;
    default:
    panic("Unknown string dictionary carrier state, or queue was already terminated.");
    }
}

inline static void exec_carrier_insert(carrier_t *self, const carrier_push_arg_t *push_arg)
{
    char **data = (char **) vector_data(&push_arg->strings);
    for (size_t i = 0; i < push_arg->strings.num_elems; i++) {
        printf("DEBUG: insert '%s' for carrier '%zu'\n", data[i], self->id);
        data++;
    }
}

void *carrier_thread_func(void *args)
{
    carrier_t          *self = (carrier_t *) args;
    carrier_push_arg_t *push_arg;

    while (atomic_flag_test_and_set(&self->keep_running)) {
        apr_queue_pop(&self->input, &push_arg);

        carrier_lock(self);
        register_carrier_as_working(self);
        exec_carrier_insert(self, push_arg);
        update_carrier_working_state(self);
        carrier_unlock(self);
    }

    return NULL;
}

static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    async_extra_t *extra               = async_extra_get(self);
    apr_pool_create(&extra->mem_pool, NULL);

    size_t         local_capacity      = max(1, capacity / nthreads);
    size_t         local_bucket_cap    = max(1, num_index_buckets / nthreads);
    size_t         per_thread_capacity = max(1, num_index_bucket_cap / nthreads);
    carrier_t      new_carrier, *carrier;

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        vector_push(&extra->carriers, &new_carrier, 1);
        carrier = vector_at(&extra->carriers, thread_id);
        carrier->id = thread_id;
        spinlock_create(&carrier->spinlock);
        string_dic_create_naive(&carrier->local_dict, local_capacity, local_bucket_cap,
                per_thread_capacity, 0, &self->alloc);
        apr_queue_create(&carrier->input, 100, extra->mem_pool);
        atomic_flag_test_and_set(&carrier->keep_running);
        atomic_flag_test_and_set(&carrier->is_waiting_for_jobs);
        pthread_create(&carrier->thread, NULL, carrier_thread_func, carrier);
    }

    return STATUS_OK;
}

static int async_carrier_insert(struct string_dic *self, size_t carrier_id, const carrier_push_arg_t *args)
{
    async_extra_t *extra  = async_extra_get(self);
    assert (carrier_id < vector_len(&extra->carriers));
    carrier_t *carrier = vector_at(&extra->carriers, carrier_id);
    apr_queue_push(&carrier->input, args);
    return STATUS_OK;
}

static int async_carrier_sync(struct string_dic *self)
{
    async_extra_t *extra          = async_extra_get(self);
    bool           one_is_working = true;

    do {
        for (size_t carrier_id = 0; carrier_id < vector_len(&extra->carriers); carrier_id++) {
            carrier_t *carrier = vector_at(&extra->carriers, carrier_id);
            carrier_lock(carrier);
        }

        for (size_t carrier_id = 0; carrier_id < vector_len(&extra->carriers); carrier_id++) {
            carrier_t *carrier = vector_at(&extra->carriers, carrier_id);
            bool carrier_is_waiting = atomic_flag_test_and_set(carrier->is_waiting_for_jobs);
            one_is_working &= !carrier_is_waiting;
            if (!carrier_is_waiting) {
                atomic_flag_clear(&carrier->is_waiting_for_jobs);
            }
        }

        for (size_t carrier_id = 0; carrier_id < vector_len(&extra->carriers); carrier_id++) {
            carrier_t *carrier = vector_at(&extra->carriers, carrier_id);
            carrier_unlock(carrier);
        }
    } while (one_is_working);

    return STATUS_OK;
}

static int async_drop_carriers(struct string_dic *self)
{
    assert(self);
    async_extra_t *extra    = async_extra_get(self);
    size_t         nthreads = vector_len(&extra->carriers);

    /* break the running loop of each thread */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        atomic_flag_clear(&carrier->keep_running);
    }

    /* wait for threads being finished */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = vector_at(&extra->carriers, thread_id);
        pthread_join(&carrier->thread, NULL);
    }

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = vector_at(&extra->carriers, thread_id);
        string_dic_drop(&carrier->local_dict);
    }

    apr_pool_destroy(&extra->mem_pool);
    return STATUS_OK;
}

static int async_lock(struct string_dic *self)
{
    async_extra_t *extra = async_extra_get(self);
    check_success(spinlock_lock(&extra->spinlock));
    return STATUS_OK;
}

static int async_unlock(struct string_dic *self)
{
    async_extra_t *extra = async_extra_get(self);
    check_success(spinlock_unlock(&extra->spinlock));
    return STATUS_OK;
}

static int carrier_lock(carrier_t *carrier)
{
    return spinlock_lock(&carrier->spinlock);
}

static int carrier_unlock(carrier_t *carrier)
{
    return spinlock_unlock(&carrier->spinlock);
}
