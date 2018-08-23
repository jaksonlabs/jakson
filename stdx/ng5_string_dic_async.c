
#include <stdx/ng5_string_dic_sync.h>
#include <stdx/ng5_string_dic_async.h>
#include <stdx/ng5_vector.h>
#include <stdx/ng5_spinlock.h>

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
    spinlock_t                    spinlock;

} async_extra_t;

typedef struct carrier_arg_t
{
    ng5_vector_t of_type(char *)      strings;
    string_id_t                  *out;
    carrier_t                    *carrier;
    bool                          write_out;
} carrier_arg_t;

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define hashcode_of(string)                                     \
    hash_func(strlen(string), string)

#define make_global(thread_id, thread_local_id)                 \
    ((thread_id << 54) | thread_local_id);

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
        size_t bucket_cap, const ng5_allocator_t *alloc);

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
        size_t num_index_bucket_cap, size_t nthreads, const ng5_allocator_t* alloc)
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
    ng5_vector_create(&extra->carriers, &self->alloc, sizeof(carrier_t), nthreads);
    async_setup_carriers(self, capacity, num_index_buckets, num_index_bucket_cap, nthreads);
    ng5_vector_create(&extra->carrier_mapping, &self->alloc, sizeof(carrier_t*), capacity);

    return STATUS_OK;
}

static int async_drop(struct string_dic *self)
{
    check_tag(self->tag, STRING_DIC_ASYNC);
    async_extra_t *extra = async_extra_get(self);

    check_success(ng5_vector_drop(&extra->carriers));
    check_success(ng5_vector_drop(&extra->carrier_mapping));
    check_success(allocator_free(&self->alloc, extra));
    return STATUS_OK;
}


void *carrier_insert_func(void *args)
{
    carrier_arg_t * restrict this_args = (carrier_arg_t * restrict) args;

    if (likely(this_args->strings.num_elems > 0)) {
        debug("[EXEC  ] carrier %zu starts task", this_args->carrier->id);
        char** data = (char**) ng5_vector_data(&this_args->strings);

        int status = string_dic_insert(&this_args->carrier->local_dict,
                this_args->write_out ? &this_args->out : NULL,
                data, ng5_vector_len(&this_args->strings));

        panic_if(status!=STATUS_OK, "internal error during thread-local string dictionary building process");
        debug("[EXEC  ] carrier %zu finished task", this_args->carrier->id);
    } else {
        warn("carrier %zu was requested to insert empty string list %s", this_args->carrier->id,
                (this_args->strings.base == NULL ? "[INTERNAL ERROR]" : ""));
    }

    return NULL;
}

static int async_insert(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings)
{
    check_tag(self->tag, STRING_DIC_ASYNC);

    async_lock(self);

    async_extra_t      *extra                    = async_extra_get(self);
    uint_fast16_t       nthreads                 = ng5_vector_len(&extra->carriers);

    /* map string depending on hash value to a particular carrier */
    uint_fast16_t      *str_carrier_mapping      = allocator_malloc(&self->alloc, num_strings * sizeof(uint_fast16_t));

    /* counters to compute how many strings go to a particular carrier */
    size_t             *carrier_nstrings         = allocator_malloc(&self->alloc, nthreads * sizeof(size_t));
    memset(carrier_nstrings, 0, nthreads * sizeof(size_t));

    ng5_vector_t of_type(carrier_arg_t *) carrier_args;
    ng5_vector_create(&carrier_args, &self->alloc, sizeof(carrier_arg_t*), nthreads);

    /* compute which carrier is responsible for which string */
    for (size_t i = 0; i < num_strings; i++) {
        const char     *key       = strings[i];
        size_t          thread_id = hashcode_of(key) % nthreads;
        str_carrier_mapping[i]    = thread_id;
        carrier_nstrings[thread_id]++;
    }

    /* prepare to move string subsets to carriers */
    for (uint_fast16_t i = 0; i < nthreads; i++) {
        carrier_arg_t* entry = allocator_malloc(&self->alloc, sizeof(carrier_arg_t));
        entry->carrier       = ng5_vector_get(&extra->carriers, i, carrier_t);

        ng5_vector_create(&entry->strings, &self->alloc, sizeof(char*), carrier_nstrings[i]);
        ng5_vector_push(&carrier_args, &entry, 1);
        assert (entry->strings.base != NULL);
    }

    /* create per-carrier string subset */
    for (size_t i = 0; i < num_strings; i++) {
        uint_fast16_t  thread_id   = str_carrier_mapping[i];
        carrier_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_arg_t *);
        carrier_arg->write_out     = out != NULL;

        ng5_vector_push(&carrier_arg->strings, &strings[i], 1);
    }

    /* schedule insert operation per carrier */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        if (carrier_nstrings[thread_id] > 0) {
            carrier_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_arg_t *);
            carrier_t      *carrier    = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
            pthread_create(&carrier->thread, NULL, carrier_insert_func, carrier_arg);
        }
    }

    /* synchronize */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_t *carrier = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        pthread_join(carrier->thread, NULL);
    }

    /* compute string ids; the string id produced by this implementation is a compound identifier encoding
     * both the owning thread id and the thread-local string id. For this, the returned (global) string identifier
     * is split into 10bits encoded the thread (given a maximum of 1024 threads that can be handled by this
     * implementation), and 54bits used to encode the thread-local string id
     *
     * TECHNICAL LIMIT: 1024 threads
     */

    /* optionally, return the created string ids. In case 'out' is NULL, nothing has to be done (especially
     * none of the carrier threads allocated thread-local 'out's which mean that no cleanup must be done */
    if (likely(out != NULL)) {
        string_id_t *total_out   = allocator_malloc(&self->alloc, num_strings * sizeof(string_id_t));
        size_t       current_out = 0;

        for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
            carrier_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_arg_t *);
            for (size_t local_idx = 0; local_idx < carrier_nstrings[thread_id]; local_idx++) {
                string_id_t global_string_owner_id = thread_id;
                string_id_t global_string_local_id = carrier_arg->out[local_idx];
                string_id_t global_string_id       = make_global(global_string_owner_id, global_string_local_id);
                total_out[current_out++]           = global_string_id;
            }

            /* cleanup */
            allocator_free(&self->alloc, carrier_arg->out);
        }
        *out = total_out;
    }

    /* cleanup */
    for (uint_fast16_t thread_id = 0; thread_id < nthreads; thread_id++) {
        carrier_arg_t *carrier_arg = *ng5_vector_get(&carrier_args, thread_id, carrier_arg_t *);
        ng5_vector_drop(&carrier_arg->strings);
        allocator_free(&self->alloc, carrier_arg);
    }

    /* cleanup */
    allocator_free(&self->alloc, carrier_nstrings);
    allocator_free(&self->alloc, str_carrier_mapping);
    ng5_vector_drop(&carrier_args);

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

    size_t         local_capacity      = max(1, capacity / nthreads);
    size_t         local_bucket_num    = max(1, num_index_buckets / nthreads);
    size_t         local_bucket_cap    = max(1, num_index_bucket_cap / nthreads);
    carrier_t      new_carrier, *carrier;

    for (size_t thread_id = 0; thread_id < nthreads; thread_id++) {
        ng5_vector_push(&extra->carriers, &new_carrier, 1);
        carrier = ng5_vector_get(&extra->carriers, thread_id, carrier_t);
        async_carrier_create(carrier, thread_id, local_capacity, local_bucket_num, local_bucket_cap,
        &self->alloc);
    }

    return STATUS_OK;
}

static void async_carrier_create(carrier_t *carrier, size_t thread_id, size_t capacity, size_t bucket_num,
                                size_t bucket_cap, const ng5_allocator_t *alloc)
{
    carrier->id = thread_id;
    string_dic_create_naive(&carrier->local_dict, capacity, bucket_num, bucket_cap, 0, alloc);
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
