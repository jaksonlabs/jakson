#include <stdx/string_dics/string_dic_async.h>
#include <stdx/vector.h>
#include <stdx/async.h>
#include <stdx/string_dics/string_dic_naive.h>
#include <apr_queue.h>
#include <stdx/time.h>

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
    apr_queue_t                  *input;
    string_dic_t                  local_dict;
    pthread_t                     thread;
    size_t                        id;
    atomic_flag                   keep_running;
    struct string_dic            *context;

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

typedef enum {
  push_type_insert_strings, push_type_heartbeat
} push_arg_type_e;

typedef struct carrier_push_arg_t
{
    vector_t of_type(char *)      strings;
    string_id_t                  *out;
    push_arg_type_e               type;
    bool                          task_done;
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

static int async_carrier_submit(struct string_dic* self, size_t carrier_id, carrier_push_arg_t* args);

static void async_carrier_create(carrier_t *carrier, size_t thread_id, size_t capacity, size_t bucket_num,
        size_t bucket_cap, const struct allocator *alloc, apr_pool_t *pool);

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
    async_setup_carriers(self, capacity, num_index_buckets, num_index_bucket_cap, nthreads);
    vector_create(&extra->carrier_mapping, &self->alloc, sizeof(carrier_t *), capacity);

    return STATUS_OK;
}

static int async_drop(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);
    async_extra_t *extra = async_extra_get(self);

    async_drop_carriers(self);

    check_success(vector_drop(&extra->carriers));
    check_success(vector_drop(&extra->carrier_mapping));
    check_success(allocator_free(&self->alloc, extra));
    return STATUS_OK;
}

static void async_carriers_sync(vector_t of_type(carrier_t) *carriers, vector_t of_type(carrier_push_arg_t *) *carrier_args, size_t timeout)
{
    timeout = timeout == 0 ? (size_t) - 1 : timeout * 1000;
    timestamp_t start = time_current_time_ms();
    debug("*SYNC*** async_carriers_sync [BEGIN]%s", "");
    bool all_carriers_done;
    assert (vector_len(carriers) == vector_len(carrier_args));
    size_t nthreads = vector_len(carriers);
    debug("INFO: %zu threads running", nthreads);
    do {
        all_carriers_done = true;
        for (size_t thread_id = 0; all_carriers_done && thread_id < nthreads; thread_id++) {
            carrier_push_arg_t *thread_push_args = *vector_get(carrier_args, thread_id, carrier_push_arg_t *);
            carrier_t *carrier = vector_get(carriers, thread_id, carrier_t);
            debug("~LOCK  ~ main thread aquires lock for carrier %zu", thread_id);
            carrier_lock(carrier);
            debug("~LOCKED~ main thread aquired lock for carrier %zu", thread_id);
            bool done = thread_push_args->task_done;
            debug("~STATE ~ carrier %zu finished task: %s", thread_id, done ? "YES" : "NO");
            carrier_unlock(carrier);
            debug("~UNLOCK~ main thread aquired lock for carrier %zu", thread_id);
            all_carriers_done &= done;
        }
        if (!all_carriers_done && (size_t) (time_current_time_ms() - start) > timeout) {
            warn("timeout for carrier sync reached: %zusec", timeout/1000);
            all_carriers_done = true;
        }
    } while (!all_carriers_done);
    debug("*SYNC*** async_carriers_sync [END]%s", "");
}

static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    async_extra_t      *extra                    = async_extra_get(self);
    size_t              nthreads                 = vector_len(&extra->carriers);

    /* map string depending on hash value to a particular carrier */
    size_t             *str_carrier_mapping      = allocator_malloc(&self->alloc, num_strings * sizeof(size_t));

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
        carrier_push_arg_t* entry = allocator_malloc(&self->alloc, sizeof(carrier_push_arg_t));
        entry->type = push_type_insert_strings;
       // atomic_flag_clear(&entry->task_done);
        vector_create(&entry->strings, &self->alloc, sizeof(char*), carrier_reserve_nstrings[i]);
        vector_push(&carrier_args, &entry, 1);
        assert (entry->strings.base!=NULL);
    }

    /* create per-carrier string subset */
    for (size_t i = 0; i < num_strings; i++) {
        size_t thread_id                     = str_carrier_mapping[i];
        carrier_push_arg_t *thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
        char *dummy = strdup(strings[i]);
        vector_push(&thread_push_args->strings, &dummy, 1);
    }

    /* schedule insert operation per carrier */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        if (carrier_reserve_nstrings[thread_id] > 0) {
            carrier_push_arg_t* thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
            async_carrier_submit(self, thread_id, thread_push_args);
        }
    }


    debug("*** MAIN-THREAD WAITING FOR TASK-COMPLETION ***%s", "\n");

    /* synchronize and wait for carrier to finish execution */
    async_carriers_sync(&extra->carriers, &carrier_args, 0);

    debug("*** MAIN-THREAD CONTINUES AFTER TASK-COMPLETION ***%s", "\n");

    /* cleanup */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_push_arg_t *thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
        allocator_free(&self->alloc, thread_push_args);
    }




    // TODO:... out
    unused(out);

    /* cleanup */
    allocator_free(&self->alloc, carrier_reserve_nstrings);
    allocator_free(&self->alloc, str_carrier_mapping);

    vector_drop(&carrier_args);

    async_unlock(self);

    return STATUS_OK;
}

static int async_remove(struct string_dic *self, string_id_t *strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    unused(strings);
    unused(num_strings);
    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_locate_safe(struct string_dic* self, string_id_t** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    unused(out);
    unused(found_mask);
    unused(num_not_found);
    unused(keys);
    unused(num_keys);
    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_locate_fast(struct string_dic* self, string_id_t** out, char* const* keys,
        size_t num_keys)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    unused(out);
    unused(keys);
    unused(num_keys);
    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static char **async_extract(struct string_dic *self, const string_id_t *ids, size_t num_ids)
{
    if (self->tag != STRING_DIC_ASYNC) {
        return NULL;
    }

    async_lock(self);

    unused(ids);
    unused(num_ids);
    // TODO:...

    async_unlock(self);

    return NULL;
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

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

static int async_counters(struct string_dic *self, struct string_lookup_counters *counters)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    unused(counters);
    async_lock(self);

    // TODO:...

    async_unlock(self);

    return STATUS_OK;
}

unused_fn inline static void exec_carrier_insert(carrier_t *self, carrier_push_arg_t *push_arg)
{
    if (push_arg->strings.num_elems > 0) {
        debug("[EXEC  ] carrier %zu starts task", self->id);
        char** data = (char**) vector_data(&push_arg->strings);

        int status = string_dic_insert(&self->local_dict, &push_arg->out, data, vector_len(&push_arg->strings));
        panic_if(status!=STATUS_OK, "internal error during thread-local string dictionary building process");

        printf("DEBUG: inserted  %zu strings for carrier '%zu'\n", push_arg->strings.num_elems, self->id);
        fflush(stdout);
        debug("[EXEC  ] carrier %zu finished task", self->id);
    } else {
        warn("carrier %zu was requested to insert empty string list %s", self->id, (push_arg->strings.base == NULL ? "[INTERNAL ERROR]" : ""));
    }
}

void *carrier_thread_func(void *args)
{
    carrier_t          *self = (carrier_t *) args;
    carrier_push_arg_t *push_arg;

    while (atomic_flag_test_and_set(&self->keep_running)) {
        debug("[WAIT  ] carrier %zu ask for task", self->id);

        apr_queue_pop(self->input, (void **) &push_arg);
        debug("[CONT  ] carrier %zu found task", self->id);

        debug("[LOCK  ] carrier %zu waits for aquires lock", self->id);
        carrier_lock(self);
        debug("[LOCKED] carrier %zu aquired lock", self->id);
        push_arg->task_done = false;
        debug("[FLAG  ] carrier %zu notified on task acknowledgement", self->id);
        carrier_unlock(self);
        debug("[UNLOCK] carrier %zu releases lock", self->id);

        assert(push_arg);

        switch (push_arg->type) {
        case push_type_insert_strings:
            debug("[INSERT] carrier %zu task found", self->id);
            exec_carrier_insert(self, push_arg);
            vector_drop(&push_arg->strings);
            debug("[INSERT] carrier %zu done", self->id);
            break;
        case push_type_heartbeat:
            debug("[HEART ] carrier %zu gets hearbeat", self->id);
            break;
        default:
            panic_wargs("Unknown type for task queue in carrier %zu detected: type id is '%d'", self->id, push_arg->type);
            break;
        }

        debug("[END   ] carrier %zu ended task select", self->id);

        debug("[LOCK  ] carrier %zu waits for aquires lock", self->id);
        carrier_lock(self);
        debug("[LOCKED] carrier %zu aquired lock", self->id);
        push_arg->task_done = true;
        debug("[FLAG  ] carrier %zu notified on task completion", self->id);
        carrier_unlock(self);
        debug("[UNLOCK] carrier %zu releases lock", self->id);
    }

    debug("[LOOP  ] carrier %zu exists main loop", self->id);

    return NULL;
}

static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    async_extra_t *extra               = async_extra_get(self);
    apr_pool_create(&extra->mem_pool, NULL);

    size_t         local_capacity      = max(1, capacity / nthreads);
    size_t         local_bucket_num    = max(1, num_index_buckets / nthreads);
    size_t         local_bucket_cap    = max(1, num_index_bucket_cap / nthreads);
    carrier_t      new_carrier, *carrier;

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        vector_push(&extra->carriers, &new_carrier, 1);
        carrier = vector_get(&extra->carriers, thread_id, carrier_t);
        carrier->context = self;
        async_carrier_create(carrier, thread_id, local_capacity, local_bucket_num, local_bucket_cap,
        &self->alloc, extra->mem_pool);
    }

    return STATUS_OK;
}

static int async_carrier_submit(struct string_dic* self, size_t carrier_id, carrier_push_arg_t* args)
{
    async_extra_t *extra  = async_extra_get(self);
    assert (carrier_id < vector_len(&extra->carriers));
    carrier_t *carrier = vector_get(&extra->carriers, carrier_id, carrier_t);
    apr_queue_push(carrier->input, (void *) args);
    return STATUS_OK;
}

static void async_carrier_create(carrier_t *carrier, size_t thread_id, size_t capacity, size_t bucket_num,
                                size_t bucket_cap, const struct allocator *alloc, apr_pool_t *pool)
{
    carrier->id = thread_id;
    spinlock_create(&carrier->spinlock);
    string_dic_create_naive(&carrier->local_dict, capacity, bucket_num, bucket_cap, 0, alloc);
    apr_queue_create(&carrier->input, 100000, pool);
    atomic_flag_test_and_set(&carrier->keep_running);
    pthread_create(&carrier->thread, NULL, carrier_thread_func, carrier);
}

static int async_drop_carriers(struct string_dic *self)
{
    debug("~DROP  ~ dropping of carriers issued %s", "");

    assert(self);
    async_extra_t                          *extra        = async_extra_get(self);
    size_t                                  nthreads     = vector_len(&extra->carriers);
    vector_t of_type(carrier_push_arg_t *)  carrier_args;

    vector_create(&carrier_args, &self->alloc, sizeof(carrier_push_arg_t *), nthreads);

    /* break the running loop of each thread */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
        debug("~DROP  ~ stop main loop for carriers %zu", thread_id);
        atomic_flag_clear(&carrier->keep_running);
    }

    /* send hearbeat to wake up threads in case of sleeping for queue input */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        debug("~DROP  ~ issue hearbeat for carriers %zu", thread_id);
        carrier_push_arg_t* entry = allocator_malloc(&self->alloc, sizeof(carrier_push_arg_t));
        entry->type = push_type_heartbeat;
        entry->task_done = false;
        vector_push(&carrier_args, &entry, 1);
        async_carrier_submit(self, thread_id, entry);
    }


    debug("~DROP  ~ synchronize [BEGIN]%s", "");
    async_carriers_sync(&extra->carriers, &carrier_args, 3);
    debug("~DROP  ~ synchronize [PASSED]%s", "");

    /* wait for threads being finished */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        debug("~DROP  ~ join carrier thread %zu", thread_id);
        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
        pthread_join(carrier->thread, NULL);
        debug("~DROP  ~ join carrier thread %zu [DONE]", thread_id);
    }

    /* cleanup */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
        debug("~DROP  ~ cleanup thread-local dictionary for carrier %zu", thread_id);
        string_dic_drop(&carrier->local_dict);

        carrier_push_arg_t* entry = *vector_get(&carrier_args, thread_id, carrier_push_arg_t*);
        allocator_free(&self->alloc, entry);
    }
    vector_drop(&carrier_args);

    apr_pool_destroy(extra->mem_pool);
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
   // debug("acquire lock for '%zu'", carrier->id);
    return spinlock_lock(&carrier->spinlock);
}

static int carrier_unlock(carrier_t *carrier)
{
  //  debug("unlock for '%zu'", carrier->id);
    return spinlock_unlock(&carrier->spinlock);
}
