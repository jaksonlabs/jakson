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

#include "carbon/carbon-alloc.h"
#include "carbon/carbon-vector.h"
#include "carbon/strdic/carbon-strdic-sync.h"
#include "carbon/strdic/carbon-strdic-async.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-strhash.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-parallel.h"
#include "carbon/carbon-slicelist.h"

#define STRING_DIC_ASYNC_TAG "strdic_async"

#define HASH_FUNCTION                  CARBON_HASH_SAX

typedef struct AsyncExtra AsyncExtra;

typedef struct Carrier
{
    carbon_strdic_t localDictionary;
    pthread_t thread;
    size_t id;
} Carrier;

typedef struct AsyncExtra
{
    carbon_vec_t ofType(Carrier) carriers;
    carbon_vec_t ofType(Carrier *) carrierMapping;
    carbon_spinlock_t lock;
} AsyncExtra;

typedef struct ParallelInsertArg
{
    carbon_vec_t ofType(char *) strings;
    carbon_string_id_t *out;
    Carrier *carrier;
    bool enableWriteOut;
    bool didWork;
    uint_fast16_t insertNumThreads;
} ParallelInsertArg;

typedef struct ParallelRemoveArg
{
    carbon_vec_t ofType(carbon_string_id_t) *localIds;
    Carrier *carrier;
    int result;
    bool didWork;
} ParallelRemoveArg;

typedef struct ParallelLocateArg
{
    Carrier *carrier;
    carbon_string_id_t *idsOut;
    bool *foundMaskOut;
    size_t numNotFoundOut;
    carbon_vec_t ofType(char *) keysIn;
    int result;
    bool didWork;
} ParallelLocateArg;

typedef struct ParallelExtractArg
{
    carbon_vec_t ofType(carbon_string_id_t) localIdsIn;
    char **stringsOut;
    Carrier *carrier;
    bool didWork;
} ParallelExtractArg;

#define HASHCODE_OF(string)                                                                                            \
    HASH_FUNCTION(strlen(string), string)

#define MAKE_GLOBAL(threadId, localcarbon_string_id_t)                                                                           \
    ((threadId << 54) | localcarbon_string_id_t)

#define GET_OWNER(globalId)                                                                                            \
    (globalId >> 54)

#define GET_carbon_string_id_t(globalId)                                                                                         \
    ((~((carbon_string_id_t) 0)) >> 10 & globalcarbon_string_id_t);

static bool this_drop(carbon_strdic_t *self);
static bool this_insert(carbon_strdic_t *self,
                      carbon_string_id_t **out,
                      char *const *strings,
                      size_t numStrings,
                      size_t __numThreads);
static bool this_remove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t numStrings);
static bool this_locate_safe(carbon_strdic_t *self, carbon_string_id_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys);
static bool this_locate_fast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t num_keys);
static char **this_extract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t num_ids);
static bool this_free(carbon_strdic_t *self, void *ptr);

static bool this_num_distinct(carbon_strdic_t *self, size_t *num);
static bool this_get_contents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                           carbon_vec_t ofType(carbon_string_id_t) * carbon_string_id_ts);

static bool this_reset_counters(carbon_strdic_t *self);
static bool this_counters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters);

static bool thisLock(carbon_strdic_t *self);
static bool thisUnlock(carbon_strdic_t *self);

static bool thisCreateExtra(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                           size_t approxNumUniqueStr, size_t num_threads);

static bool thisSetupCarriers(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                             size_t approxNumUniqueStr, size_t num_threads);

#define THIS_EXTRAS(self)                                                                                              \
({                                                                                                                     \
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);                                                                         \
    (AsyncExtra *) self->extra;                                                                                        \
})

CARBON_EXPORT (int)
carbon_strdic_create_async(carbon_strdic_t *dic, size_t capacity, size_t num_index_buckets,
                               size_t approx_num_unique_strs, size_t num_threads, const carbon_alloc_t *alloc)
{
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&dic->alloc, alloc));

    dic->tag = CARBON_STRDIC_TYPE_ASYNC;
    dic->drop = this_drop;
    dic->insert = this_insert;
    dic->remove = this_remove;
    dic->locate_safe = this_locate_safe;
    dic->locate_fast = this_locate_fast;
    dic->extract = this_extract;
    dic->free = this_free;
    dic->resetCounters = this_reset_counters;
    dic->counters = this_counters;
    dic->num_distinct = this_num_distinct;
    dic->get_contents = this_get_contents;

    CARBON_CHECK_SUCCESS(thisCreateExtra(dic, capacity, num_index_buckets, approx_num_unique_strs, num_threads));
    return true;
}

static bool thisCreateExtra(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                           size_t approxNumUniqueStr, size_t num_threads)
{
    assert(self);

    self->extra = carbon_malloc(&self->alloc, sizeof(AsyncExtra));
    AsyncExtra *extra = THIS_EXTRAS(self);
    carbon_spinlock_init(&extra->lock);
    carbon_vec_create(&extra->carriers, &self->alloc, sizeof(Carrier), num_threads);
    thisSetupCarriers(self, capacity, num_index_buckets, approxNumUniqueStr, num_threads);
    carbon_vec_create(&extra->carrierMapping, &self->alloc, sizeof(Carrier *), capacity);

    return true;
}

static bool this_drop(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    AsyncExtra *extra = THIS_EXTRAS(self);
    for (size_t i = 0; i < extra->carriers.numElems; i++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, i, Carrier);
        carbon_strdic_drop(&carrier->localDictionary);
    }
    CARBON_CHECK_SUCCESS(VectorDrop(&extra->carriers));
    CARBON_CHECK_SUCCESS(VectorDrop(&extra->carrierMapping));
    CARBON_CHECK_SUCCESS(carbon_free(&self->alloc, extra));
    return true;
}

void *parallelRemoveFunction(void *args)
{
    ParallelRemoveArg *carrierArg = (ParallelRemoveArg *) args;
    carbon_string_id_t len = carbon_vec_length(carrierArg->localIds);
    carrierArg->didWork = len > 0;

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for remove task (%zu elements)", carrierArg->carrier->id,
          carbon_vec_length(carrierArg->localIds));
    if (len > 0) {
        carbon_strdic_t *dic = &carrierArg->carrier->localDictionary;
        carbon_string_id_t *ids = VECTOR_ALL(carrierArg->localIds, carbon_string_id_t);
        carrierArg->result = carbon_strdic_remove(dic, ids, len);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu task done", carrierArg->carrier->id);
    }
    else {
        carrierArg->result = true;
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", carrierArg->carrier->id);
    }

    return NULL;
}

void *parallelInsertFunction(void *args)
{
    ParallelInsertArg *restrict thisArgs = (ParallelInsertArg *restrict) args;
    thisArgs->didWork = thisArgs->strings.numElems > 0;

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread-local insert function started (thread %zu)", thisArgs->carrier->id);
    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for insert task (%zu elements)", thisArgs->carrier->id,
          carbon_vec_length(&thisArgs->strings));

    if (thisArgs->didWork) {
        CARBON_TRACE(STRING_DIC_ASYNC_TAG,
              "thread %zu starts insertion of %zu strings",
              thisArgs->carrier->id,
              carbon_vec_length(&thisArgs->strings));
        char **data = (char **) VectorData(&thisArgs->strings);

        int status = carbon_strdic_insert(&thisArgs->carrier->localDictionary,
                                          thisArgs->enableWriteOut ? &thisArgs->out : NULL,
                                          data, carbon_vec_length(&thisArgs->strings), thisArgs->insertNumThreads);

        /** internal error during thread-local string dictionary building process */
        CARBON_PRINT_ERROR_AND_DIE_IF(status != true, CARBON_ERR_INTERNALERR);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

void *parallelLocateSafeFunction(void *args)
{
    ParallelLocateArg *restrict thisArgs = (ParallelLocateArg *restrict) args;
    thisArgs->didWork = carbon_vec_length(&thisArgs->keysIn) > 0;

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread-local 'locate' function invoked for thread %zu...", thisArgs->carrier->id)

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for locate (safe) task (%zu elements)", thisArgs->carrier->id,
          carbon_vec_length(&thisArgs->keysIn));

    if (thisArgs->didWork) {
        thisArgs->result = carbon_strdic_locate_safe(&thisArgs->idsOut,
                                                     &thisArgs->foundMaskOut,
                                                     &thisArgs->numNotFoundOut,
                                                     &thisArgs->carrier->localDictionary,
                                                     VECTOR_ALL(&thisArgs->keysIn, char *),
                                                     carbon_vec_length(&thisArgs->keysIn));

        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

void *parallelExtractFunction(void *args)
{
    ParallelExtractArg *restrict thisArgs = (ParallelExtractArg *restrict) args;
    thisArgs->didWork = carbon_vec_length(&thisArgs->localIdsIn) > 0;

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for extract task (%zu elements)", thisArgs->carrier->id,
          carbon_vec_length(&thisArgs->localIdsIn));

    if (thisArgs->didWork) {
        thisArgs->stringsOut = carbon_strdic_extract(&thisArgs->carrier->localDictionary,
                                                     VECTOR_ALL(&thisArgs->localIdsIn, carbon_string_id_t),
                                                     carbon_vec_length(&thisArgs->localIdsIn));
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        CARBON_WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

static void Synchronize(carbon_vec_t ofType(Carrier) *carriers, size_t num_threads)
{
    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "barrier installed for %d threads", num_threads);

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        volatile Carrier *carrier = VECTOR_GET(carriers, threadId, Carrier);
        pthread_join(carrier->thread, NULL);
        CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "thread %d joined", carrier->id);
    }
    carbon_timestamp_t end = carbon_time_now_wallclock();
    carbon_timestamp_t duration = (end - begin);
    CARBON_UNUSED(duration);

    CARBON_DEBUG(STRING_DIC_ASYNC_TAG, "barrier passed for %d threads after %f seconds", num_threads, duration / 1000.0f);
}

static void createThreadAssignment(atomic_uint_fast16_t **strCarrierMapping, atomic_size_t **carrierNumStrings,
                                   size_t **strCarrierIdxMapping,
                                   carbon_alloc_t *alloc, size_t numStrings, size_t num_threads)
{
    /** map string depending on hash values to a particular Carrier */
    *strCarrierMapping = carbon_malloc(alloc, numStrings * sizeof(atomic_uint_fast16_t));
    memset(*strCarrierMapping, 0, numStrings * sizeof(atomic_uint_fast16_t));

    /** counters to compute how many strings go to a particular Carrier */
    *carrierNumStrings = carbon_malloc(alloc, num_threads * sizeof(atomic_size_t));
    memset(*carrierNumStrings, 0, num_threads * sizeof(atomic_size_t));

    /** an inverted index that contains the i-th position for string k that was assigned to Carrier m.
     * With this, given a (global) string and and its Carrier, one can have directly the position of the
     * string in the carriers "thread-local locate" args */
    *strCarrierIdxMapping = carbon_malloc(alloc, numStrings * sizeof(size_t));
}

static void dropThreadAssignment(carbon_alloc_t *alloc, atomic_uint_fast16_t *strCarrierMapping,
                                 atomic_size_t *carrierNumStrings, size_t *strCarrierIdxMapping)
{
    carbon_free(alloc, carrierNumStrings);
    carbon_free(alloc, strCarrierMapping);
    carbon_free(alloc, strCarrierIdxMapping);
}

typedef struct ParallelComputeThreadAssignmentArg
{
    atomic_uint_fast16_t *strCarrierMapping;
    size_t num_threads;
    atomic_size_t *carrierNumStrings;
    char *const *baseStrings;
} ParallelComputeThreadAssignmentArg;

static void ParallelComputeThreadAssignmentFunction(const void *restrict start, size_t width, size_t len,
                                                    void *restrict args, ThreadId tid)
{
    CARBON_UNUSED(tid);
    CARBON_UNUSED(width);

    char *const *strings = (char *const *) start;

    ParallelComputeThreadAssignmentArg *funcArgs = (ParallelComputeThreadAssignmentArg *) args;

    while (len--) {
        size_t i = strings - funcArgs->baseStrings;
        const char *key = *strings;
        /** re-using this hashcode for the thread-local dictionary is more costly than to compute it fresh
         * (due to more I/O with the RAM) */
        size_t threadId = HASHCODE_OF(key) % funcArgs->num_threads;
        atomic_fetch_add(&funcArgs->strCarrierMapping[i], threadId);
        atomic_fetch_add(&funcArgs->carrierNumStrings[threadId], 1);
        strings++;
    }
}

static void computeThreadAssignment(atomic_uint_fast16_t *strCarrierMapping, atomic_size_t *carrierNumStrings,
                                    char *const *strings, size_t numStrings, size_t num_threads)
{
    ParallelComputeThreadAssignmentArg args = {
        .baseStrings = strings,
        .carrierNumStrings = carrierNumStrings,
        .num_threads = num_threads,
        .strCarrierMapping = strCarrierMapping
    };
    ParallelFor(strings,
                sizeof(char *const *),
                numStrings,
                ParallelComputeThreadAssignmentFunction,
                &args,
                ThreadingHint_Multi,
                num_threads);

}

static bool this_insert(carbon_strdic_t *self,
                      carbon_string_id_t **out,
                      char *const *strings,
                      size_t numStrings,
                      size_t __numThreads)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "insert operation invoked: %zu strings in total", numStrings)


    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    /** parameter 'num_threads' must be set to 0 for async dictionary */
    CARBON_PRINT_ERROR_AND_DIE_IF(__numThreads != 0, CARBON_ERR_INTERNALERR);

    thisLock(self);

    AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);

    atomic_uint_fast16_t *strCarrierMapping;
    size_t *strCarrierIdxMapping;
    atomic_size_t *carrierNumStrings;


    createThreadAssignment(&strCarrierMapping, &carrierNumStrings, &strCarrierIdxMapping,
                           &self->alloc, numStrings, num_threads);

    carbon_vec_t ofType(ParallelInsertArg *) carrierArgs;
    carbon_vec_create(&carrierArgs, &self->alloc, sizeof(ParallelInsertArg *), num_threads);

    /** compute which Carrier is responsible for which string */
    computeThreadAssignment(strCarrierMapping, carrierNumStrings, strings, numStrings, num_threads);

    /** prepare to move string subsets to carriers */
    for (uint_fast16_t i = 0; i < num_threads; i++) {
        ParallelInsertArg *entry = carbon_malloc(&self->alloc, sizeof(ParallelInsertArg));
        entry->carrier = VECTOR_GET(&extra->carriers, i, Carrier);
        entry->insertNumThreads = num_threads;

        carbon_vec_create(&entry->strings, &self->alloc, sizeof(char *), CARBON_MAX(1, carrierNumStrings[i]));
        carbon_vec_push(&carrierArgs, &entry, 1);
        assert (entry->strings.base != NULL);

        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, i, ParallelInsertArg *);
        carrierArg->out = NULL;
    }

    /** create per-Carrier string subset */
    /** parallizing this makes no sense but waste of resources and energy */
    for (size_t i = 0; i < numStrings; i++) {
        uint_fast16_t threadId = strCarrierMapping[i];
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        carrierArg->enableWriteOut = out != NULL;

        /** store local index of string i inside the thread */
        strCarrierIdxMapping[i] = carbon_vec_length(&carrierArg->strings);

        carbon_vec_push(&carrierArg->strings, &strings[i], 1);
    }


    /** schedule insert operation per Carrier */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "schedule insert operation to %zu threads", num_threads)
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        CARBON_TRACE(STRING_DIC_ASYNC_TAG, "create thread %zu...", threadId)
        pthread_create(&carrier->thread, NULL, parallelInsertFunction, carrierArg);
        CARBON_TRACE(STRING_DIC_ASYNC_TAG, "thread %zu created", threadId)
    }
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "scheduling done for %zu threads", num_threads)

    /** synchronize */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "start synchronizing %zu threads", num_threads)
    Synchronize(&extra->carriers, num_threads);
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync", num_threads)

    /** compute string ids; the string id produced by this implementation is a compound identifier encoding
     * both the owning thread id and the thread-local string id. For this, the returned (global) string identifier
     * is split into 10bits encoded the thread (given a maximum of 1024 threads that can be handled by this
     * implementation), and 54bits used to encode the thread-local string id
     *
     * TECHNICAL LIMIT: 1024 threads
     */

    /** optionally, return the created string ids. In case 'out' is NULL, nothing has to be done (especially
     * none of the Carrier threads allocated thread-local 'out's which mean that no cleanup must be done */

    /** parallelizing the following block makes no sense but waste of compute power and energy */
    if (CARBON_BRANCH_LIKELY(out != NULL)) {
        carbon_string_id_t *totalOut = carbon_malloc(&self->alloc, numStrings * sizeof(carbon_string_id_t));
        size_t currentOut = 0;

        for (size_t string_idx = 0; string_idx < numStrings; string_idx++) {
            uint_fast16_t threadId = strCarrierMapping[string_idx];
            size_t localIdx = strCarrierIdxMapping[string_idx];
            ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
            carbon_string_id_t globalStringOwnerId = threadId;
            carbon_string_id_t globalStringLocalId = carrierArg->out[localIdx];
            carbon_string_id_t globalcarbon_string_id_t = MAKE_GLOBAL(globalStringOwnerId, globalStringLocalId);
            totalOut[currentOut++] = globalcarbon_string_id_t;
        }

        *out = totalOut;
    }

    /** cleanup */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        if (carrierArg->didWork) {
            carbon_strdic_free(&carrierArg->carrier->localDictionary, carrierArg->out);
        }
        VectorDrop(&carrierArg->strings);
        carbon_free(&self->alloc, carrierArg);
    }

    /** cleanup */
    dropThreadAssignment(&self->alloc, strCarrierMapping, carrierNumStrings, strCarrierIdxMapping);
    VectorDrop(&carrierArgs);

    thisUnlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_remove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t numStrings)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "remove operation started: %zu strings to remove", numStrings);

    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    thisLock(self);

    ParallelRemoveArg empty;
    struct AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);
    size_t approxNumStringsPerThread = CARBON_MAX(1, numStrings / num_threads);
    carbon_vec_t ofType(carbon_string_id_t) *stringMap =
        carbon_malloc(&self->alloc, num_threads * sizeof(carbon_vec_t));

    carbon_vec_t ofType(ParallelRemoveArg) carrierArgs;
    carbon_vec_create(&carrierArgs, &self->alloc, sizeof(ParallelRemoveArg), num_threads);

    /** prepare thread-local subset of string ids */
    VectorRepreatedPush(&carrierArgs, &empty, num_threads);
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        carbon_vec_create(stringMap + threadId, &self->alloc, sizeof(carbon_string_id_t), approxNumStringsPerThread);
    }

    /** compute subset of string ids per thread  */
    for (size_t i = 0; i < numStrings; i++) {
        carbon_string_id_t globalcarbon_string_id_t = strings[i];
        uint_fast16_t owningThreadId = GET_OWNER(globalcarbon_string_id_t);
        carbon_string_id_t localcarbon_string_id_t = GET_carbon_string_id_t(globalcarbon_string_id_t);
        assert(owningThreadId < num_threads);

        carbon_vec_push(stringMap + owningThreadId, &localcarbon_string_id_t, 1);
    }

    /** schedule remove operation per Carrier */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelRemoveArg *carrierArg = VECTOR_GET(&carrierArgs, threadId, ParallelRemoveArg);
        carrierArg->carrier = carrier;
        carrierArg->localIds = stringMap + threadId;

        pthread_create(&carrier->thread, NULL, parallelRemoveFunction, carrierArg);
    }

    /** synchronize */
    Synchronize(&extra->carriers, num_threads);

    /** cleanup */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        VectorDrop(stringMap + threadId);
    }

    carbon_free(&self->alloc, stringMap);
    VectorData(&carrierArgs);

    thisUnlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "remove operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_locate_safe(carbon_strdic_t *self, carbon_string_id_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation started: %zu strings to locate", num_keys)

    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);

    /** global result output */
    CARBON_MALLOC(carbon_string_id_t, globalOut, num_keys, &self->alloc);
    CARBON_MALLOC(bool, globalFoundMask, num_keys, &self->alloc);

    size_t globalNumNotFound = 0;

    atomic_uint_fast16_t *strCarrierMapping;
    size_t *strCarrierIdxMapping;
    atomic_size_t *carrierNumStrings;

    ParallelLocateArg carrierArgs[num_threads];

    createThreadAssignment(&strCarrierMapping, &carrierNumStrings, &strCarrierIdxMapping,
                           &self->alloc, num_keys, num_threads);

    /** compute which Carrier is responsible for which string */
    computeThreadAssignment(strCarrierMapping, carrierNumStrings, keys, num_keys, num_threads);

    /** prepare to move string subsets to carriers */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelLocateArg *arg = carrierArgs + threadId;
        carbon_vec_create(&arg->keysIn, &self->alloc, sizeof(char *), carrierNumStrings[threadId]);
        assert (&arg->keysIn.base != NULL);
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "computing per-thread string subset for %zu strings", num_keys)
    /** create per-Carrier string subset */
    for (size_t i = 0; i < num_keys; i++) {
        /** get thread responsible for this particular string */
        uint_fast16_t threadId = strCarrierMapping[i];

        /** get the thread-local argument list for the thread that is responsible for this particular string */
        ParallelLocateArg *arg = carrierArgs + threadId;

        /** store local index of string i inside the thread */
        strCarrierIdxMapping[i] = carbon_vec_length(&arg->keysIn);

        /** push that string into the thread-local vector */
        carbon_vec_push(&arg->keysIn, &keys[i], 1);
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "schedule operation to threads to %zu threads...", num_threads)
    /** schedule operation to threads */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelLocateArg *arg = carrierArgs + threadId;
        carrierArgs[threadId].carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallelLocateSafeFunction, arg);
    }

    /** synchronize */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "start syncing %zu threads...", num_threads)
    Synchronize(&extra->carriers, num_threads);
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync.", num_threads)

    /** collect and merge results */
    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "merging results of %zu threads", num_threads)
    for (size_t i = 0; i < num_keys; i++) {
        /** get thread responsible for this particular string, and local position of that string inside the
         * thread storage */
        uint_fast16_t threadId = strCarrierMapping[i];
        size_t localThreadIdx = strCarrierIdxMapping[i];

        /** get the thread-local argument list for the thread that is responsible for this particular string */
        ParallelLocateArg *arg = carrierArgs + threadId;

        /** merge into global result */
        carbon_string_id_t globalcarbon_string_id_t_owner = threadId;
        carbon_string_id_t globalcarbon_string_id_t_localIdx = arg->idsOut[localThreadIdx];
        carbon_string_id_t globalcarbon_string_id_t = MAKE_GLOBAL(globalcarbon_string_id_t_owner, globalcarbon_string_id_t_localIdx);
        globalOut[i] = globalcarbon_string_id_t;
        globalFoundMask[i] = arg->foundMaskOut[localThreadIdx];
    }
    for (size_t threadId = 0; threadId < num_threads; threadId++) {
        /** compute total number of not-found elements */
        ParallelLocateArg *arg = carrierArgs + threadId;
        globalNumNotFound += arg->numNotFoundOut;

        /** cleanup */
        if (CARBON_BRANCH_LIKELY(arg->didWork)) {
            carbon_strdic_free(&arg->carrier->localDictionary, arg->foundMaskOut);
            carbon_strdic_free(&arg->carrier->localDictionary, arg->idsOut);
        }
    }

    CARBON_TRACE(STRING_DIC_ASYNC_TAG, "cleanup%s", "...")

    /** cleanup */
    dropThreadAssignment(&self->alloc, strCarrierMapping, carrierNumStrings, strCarrierIdxMapping);

    for (size_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelLocateArg *arg = carrierArgs + threadId;
        VectorDrop(&arg->keysIn);
    }

    /** return results */
    *out = globalOut;
    *found_mask = globalFoundMask;
    *num_not_found = globalNumNotFound;

    thisUnlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static bool this_locate_fast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t num_keys)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    thisLock(self);

    bool *found_mask;
    size_t num_not_found;
    int result;

    /** use safer but in principle more slower implementation */
    result = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /** cleanup */
    this_free(self, found_mask);

    thisUnlock(self);

    return result;
}

static char **this_extract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t num_ids)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation started: %zu strings to extract", num_ids)

    if (self->tag != CARBON_STRDIC_TYPE_ASYNC) {
        return NULL;
    }

    thisLock(self);

    CARBON_MALLOC(char *, globalResult, num_ids, &self->alloc);

    struct AsyncExtra *extra = (struct AsyncExtra *) self->extra;
    uint_fast16_t num_threads = carbon_vec_length(&extra->carriers);
    size_t approxNumStringsPerThread = CARBON_MAX(1, num_ids / num_threads);

    CARBON_MALLOC(size_t, localThreadIdx, num_ids, &self->alloc);
    CARBON_MALLOC(uint_fast16_t, owningThreadIds, num_ids, &self->alloc);
    CARBON_MALLOC(ParallelExtractArg, threadArgs, num_threads, &self->alloc);

    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelExtractArg *arg = threadArgs + threadId;
        carbon_vec_create(&arg->localIdsIn, &self->alloc, sizeof(carbon_string_id_t), approxNumStringsPerThread);
    }

    /** compute subset of string ids per thread  */
    for (size_t i = 0; i < num_ids; i++) {
        carbon_string_id_t globalcarbon_string_id_t = ids[i];
        owningThreadIds[i] = GET_OWNER(globalcarbon_string_id_t);
        carbon_string_id_t localcarbon_string_id_t = GET_carbon_string_id_t(globalcarbon_string_id_t);
        assert(owningThreadIds[i] < num_threads);

        ParallelExtractArg *arg = threadArgs + owningThreadIds[i];
        localThreadIdx[i] = carbon_vec_length(&arg->localIdsIn);
        carbon_vec_push(&arg->localIdsIn, &localcarbon_string_id_t, 1);
    }

    /** schedule remove operation per Carrier */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelExtractArg *carrierArg = threadArgs + threadId;
        carrierArg->carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallelExtractFunction, carrierArg);
    }

    /** synchronize */
    Synchronize(&extra->carriers, num_threads);

    for (size_t i = 0; i < num_ids; i++) {
        uint_fast16_t owningThreadId = owningThreadIds[i];
        size_t localIdx = localThreadIdx[i];
        ParallelExtractArg *carrierArg = threadArgs + owningThreadId;
        char *extractedString = carrierArg->stringsOut[localIdx];
        globalResult[i] = extractedString;
    }

    /** cleanup */
    for (uint_fast16_t threadId = 0; threadId < num_threads; threadId++) {
        ParallelExtractArg *carrierArg = threadArgs + threadId;
        VectorDrop(&carrierArg->localIdsIn);
        if (CARBON_BRANCH_LIKELY(carrierArg->didWork)) {
            carbon_strdic_free(&carrierArg->carrier->localDictionary, carrierArg->stringsOut);
        }
    }

    CARBON_FREE(localThreadIdx, &self->alloc);
    CARBON_FREE(owningThreadIds, &self->alloc);
    CARBON_FREE(threadArgs, &self->alloc);

    thisUnlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return globalResult;
}

static bool this_free(carbon_strdic_t *self, void *ptr)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    carbon_free(&self->alloc, ptr);
    return true;
}

static bool this_num_distinct(carbon_strdic_t *self, size_t *num)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t numCarriers = carbon_vec_length(&extra->carriers);
    Carrier *carriers = VECTOR_ALL(&extra->carriers, Carrier);
    size_t numDistinct = 0;
    while (numCarriers--) {
        size_t local_distinct;
        carbon_strdic_num_distinct(&local_distinct, &carriers->localDictionary);
        numDistinct += local_distinct;
        carriers++;
    }
    *num = numDistinct;
    thisUnlock(self);
    return true;
}

static bool this_get_contents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                           carbon_vec_t ofType(carbon_string_id_t) * carbon_string_id_ts)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);
    thisLock(self);
    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t numCarriers = carbon_vec_length(&extra->carriers);
    carbon_vec_t ofType (char *) localStringResults;
    carbon_vec_t ofType (carbon_string_id_t) localcarbon_string_id_tResults;
    size_t approxNumDistinctLocalValues;
    this_num_distinct(self, &approxNumDistinctLocalValues);
    approxNumDistinctLocalValues = CARBON_MAX(1, approxNumDistinctLocalValues / extra->carriers.numElems);
    approxNumDistinctLocalValues *= 1.2f;

    carbon_vec_create(&localStringResults, NULL, sizeof(char *), approxNumDistinctLocalValues);
    carbon_vec_create(&localcarbon_string_id_tResults, NULL, sizeof(carbon_string_id_t), approxNumDistinctLocalValues);


    for (size_t threadId = 0; threadId < numCarriers; threadId++)
    {
        VectorClear(&localStringResults);
        VectorClear(&localcarbon_string_id_tResults);

        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);

        carbon_strdic_get_contents(&localStringResults, &localcarbon_string_id_tResults, &carrier->localDictionary);

        assert(localcarbon_string_id_tResults.numElems == localStringResults.numElems);
        for (size_t k = 0; k < localStringResults.numElems; k++) {
            char *string = *VECTOR_GET(&localStringResults, k, char *);
            carbon_string_id_t localcarbon_string_id_t = *VECTOR_GET(&localcarbon_string_id_tResults, k, carbon_string_id_t);
            carbon_string_id_t globalcarbon_string_id_t = MAKE_GLOBAL(threadId, localcarbon_string_id_t);
            carbon_vec_push(strings, &string, 1);
            carbon_vec_push(carbon_string_id_ts, &globalcarbon_string_id_t, 1);
        }
    }

    VectorDrop(&localStringResults);
    VectorDrop(&localcarbon_string_id_tResults);
    thisUnlock(self);
    return true;
}

static bool this_reset_counters(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t num_threads = carbon_vec_length(&extra->carriers);

    for (size_t threadId = 0; threadId < num_threads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        carbon_strdic_reset_counters(&carrier->localDictionary);
    }

    thisUnlock(self);

    return true;
}

static bool this_counters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t num_threads = carbon_vec_length(&extra->carriers);

    CARBON_CHECK_SUCCESS(carbon_strhash_counters_init(counters));

    for (size_t threadId = 0; threadId < num_threads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        carbon_string_hash_counters_t local_counters;
        carbon_strdic_get_counters(&local_counters, &carrier->localDictionary);
        carbon_strhash_counters_add(counters, &local_counters);
    }

    thisUnlock(self);

    return true;
}

typedef struct ParallelCreateCarrierArg
{
    size_t localCapacity;
    size_t localBucketNum;
    size_t localBucketCap;
    const carbon_alloc_t *alloc;
} ParallelCreateCarrierArg;

static void parallelCreateCarrier(const void *restrict start, size_t width, size_t len, void *restrict args,
                                  ThreadId tid)
{
    CARBON_UNUSED(tid);
    CARBON_UNUSED(width);

    Carrier *carrier = (Carrier *) start;
    const ParallelCreateCarrierArg *createArgs = (const ParallelCreateCarrierArg *) args;
    while (len--) {
        carbon_strdic_create_sync(&carrier->localDictionary, createArgs->localCapacity, createArgs->localBucketNum,
                                  createArgs->localBucketCap, 0, createArgs->alloc);
        memset(&carrier->thread, 0, sizeof(pthread_t));
        carrier++;
    }
}

static bool thisSetupCarriers(carbon_strdic_t *self, size_t capacity, size_t num_index_buckets,
                             size_t approxNumUniqueStr, size_t num_threads)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    size_t localBucketNum = CARBON_MAX(1, num_index_buckets / num_threads);
    Carrier new_carrier;

    ParallelCreateCarrierArg createArgs = {
        .localCapacity = CARBON_MAX(1, capacity / num_threads),
        .localBucketNum = localBucketNum,
        .localBucketCap = CARBON_MAX(1, approxNumUniqueStr / num_threads / localBucketNum / SLICE_KEY_COLUMN_MAX_ELEMS),
        .alloc = &self->alloc
    };

    for (size_t threadId = 0; threadId < num_threads; threadId++) {
        new_carrier.id = threadId;
        carbon_vec_push(&extra->carriers, &new_carrier, 1);
    }

    ParallelFor(VECTOR_ALL(&extra->carriers, Carrier), sizeof(Carrier), num_threads, parallelCreateCarrier,
                &createArgs, ThreadingHint_Multi, num_threads);

    return true;
}

static bool thisLock(carbon_strdic_t *self)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    CARBON_CHECK_SUCCESS(carbon_spinlock_acquire(&extra->lock));
    return true;
}

static bool thisUnlock(carbon_strdic_t *self)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    CARBON_CHECK_SUCCESS(carbon_spinlock_release(&extra->lock));
    return true;
}
