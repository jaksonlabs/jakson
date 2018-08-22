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
    string_dic_t                  local_dict;
    pthread_t                     thread;
    size_t                        id;
    struct string_dic            *context;
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
    carrier_t                    *carrier;
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

static int async_extra_create(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);

static int async_setup_carriers(struct string_dic *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);

static void async_carrier_create(carrier_t *carrier, size_t thread_id, size_t capacity, size_t bucket_num,
        size_t bucket_cap, const struct allocator *alloc);

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

    check_success(vector_drop(&extra->carriers));
    check_success(vector_drop(&extra->carrier_mapping));
    check_success(allocator_free(&self->alloc, extra));
    return STATUS_OK;
}


void *carrier_thread_func(void *args)
{
    carrier_push_arg_t *push_arg = (carrier_push_arg_t *) args;

    if (push_arg->strings.num_elems > 0) {
        debug("[EXEC  ] carrier %zu starts task", push_arg->carrier->id);
        char** data = (char**) vector_data(&push_arg->strings);

        int status = string_dic_insert(&push_arg->carrier->local_dict, &push_arg->out, data, vector_len(&push_arg->strings));
        panic_if(status!=STATUS_OK, "internal error during thread-local string dictionary building process");

        printf("DEBUG: inserted  %zu strings for carrier '%zu'\n", push_arg->strings.num_elems, push_arg->carrier->id);
        fflush(stdout);
        debug("[EXEC  ] carrier %zu finished task", push_arg->carrier->id);
    } else {
        warn("carrier %zu was requested to insert empty string list %s", push_arg->carrier->id, (push_arg->strings.base == NULL ? "[INTERNAL ERROR]" : ""));
    }

    return NULL;
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
        entry->carrier = vector_get(&extra->carriers, i, carrier_t);
        vector_create(&entry->strings, &self->alloc, sizeof(char*), carrier_reserve_nstrings[i]);
        vector_push(&carrier_args, &entry, 1);
        assert (entry->strings.base != NULL);
    }

    /* create per-carrier string subset */
    for (size_t i = 0; i < num_strings; i++) {
        size_t thread_id                     = str_carrier_mapping[i];
        carrier_push_arg_t *thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
        char *dummy = strdup(strings[i]); // XXXXXXXX
        vector_push(&thread_push_args->strings, &dummy, 1);
    }

    /* schedule insert operation per carrier */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        if (carrier_reserve_nstrings[thread_id] > 0) {
            carrier_push_arg_t* thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
            carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
            pthread_create(&carrier->thread, NULL, carrier_thread_func, thread_push_args);
        }
    }


    debug("*** MAIN-THREAD WAITING FOR TASK-COMPLETION ***%s", "\n");

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
        pthread_join(carrier->thread, NULL);
    }

    debug("*** MAIN-THREAD CONTINUES AFTER TASK-COMPLETION ***%s", "\n");

    /* cleanup */
    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_push_arg_t *thread_push_args = *vector_get(&carrier_args, thread_id, carrier_push_arg_t *);
        vector_drop(&thread_push_args->strings);
        // TODO: get "out" result
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
        &self->alloc);
    }

    return STATUS_OK;
}
/*
static int async_carrier_submit(struct string_dic* self, size_t carrier_id, carrier_push_arg_t* args)
{
    async_extra_t *extra  = async_extra_get(self);
    assert (carrier_id < vector_len(&extra->carriers));
    carrier_t *carrier = vector_get(&extra->carriers, carrier_id, carrier_t);
    apr_queue_push(carrier->input, (void *) args);
    return STATUS_OK;
}*/

static void async_carrier_create(carrier_t *carrier, size_t thread_id, size_t capacity, size_t bucket_num,
                                size_t bucket_cap, const struct allocator *alloc)
{
    carrier->id = thread_id;
    string_dic_create_naive(&carrier->local_dict, capacity, bucket_num, bucket_cap, 0, alloc);
}

//
//static int async_drop_carriers(struct string_dic *self)
//{
//    debug("~DROP  ~ dropping of carriers issued %s", "");
//
//    assert(self);
//    async_extra_t                          *extra        = async_extra_get(self);
//    size_t                                  nthreads     = vector_len(&extra->carriers);
//    vector_t of_type(carrier_push_arg_t *)  carrier_args;
//
//    vector_create(&carrier_args, &self->alloc, sizeof(carrier_push_arg_t *), nthreads);
//
//    /* break the running loop of each thread */
//    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
//        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
//        debug("~DROP  ~ stop main loop for carriers %zu", thread_id);
//        atomic_flag_clear(&carrier->keep_running);
//    }
//
//    /* send hearbeat to wake up threads in case of sleeping for queue input */
//    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
//        debug("~DROP  ~ issue hearbeat for carriers %zu", thread_id);
//        carrier_push_arg_t* entry = allocator_malloc(&self->alloc, sizeof(carrier_push_arg_t));
//        entry->type = push_type_heartbeat;
//        entry->task_done = false;
//        vector_push(&carrier_args, &entry, 1);
//        async_carrier_submit(self, thread_id, entry);
//    }
//
//
//    debug("~DROP  ~ synchronize [BEGIN]%s", "");
//    async_carriers_sync(&extra->carriers, &carrier_args, 3);
//    debug("~DROP  ~ synchronize [PASSED]%s", "");
//
//    /* wait for threads being finished */
//    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
//        debug("~DROP  ~ join carrier thread %zu", thread_id);
//        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
//        pthread_join(carrier->thread, NULL);
//        debug("~DROP  ~ join carrier thread %zu [DONE]", thread_id);
//    }
//
//    /* cleanup */
//    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
//        carrier_t *carrier = vector_get(&extra->carriers, thread_id, carrier_t);
//        debug("~DROP  ~ cleanup thread-local dictionary for carrier %zu", thread_id);
//        string_dic_drop(&carrier->local_dict);
//
//        carrier_push_arg_t* entry = *vector_get(&carrier_args, thread_id, carrier_push_arg_t*);
//        allocator_free(&self->alloc, entry);
//    }
//    vector_drop(&carrier_args);
//
//    apr_pool_destroy(extra->mem_pool);
//    return STATUS_OK;
//}

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
//
//static int carrier_lock(carrier_t *carrier)
//{
//   // debug("acquire lock for '%zu'", carrier->id);
//    return spinlock_lock(&carrier->spinlock);
//}
//
//static int carrier_unlock(carrier_t *carrier)
//{
//  //  debug("unlock for '%zu'", carrier->id);
//    return spinlock_unlock(&carrier->spinlock);
//}
