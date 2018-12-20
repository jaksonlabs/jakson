// file: strdic_async.c

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "strdic_sync.h"
#include "strdic_async.h"
#include "vector.h"
#include "spinlock.h"
#include "strhash.h"
#include "time.h"
#include "parallel.h"
#include "alloc.h"
#include "slicelist.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

#define STRING_DIC_ASYNC_TAG "strdic_async"

// ---------------------------------------------------------------------------------------------------------------------
//
//  S T A T I C   C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

#define HASH_FUNCTION                  HashSax

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct AsyncExtra AsyncExtra;

typedef struct Carrier
{
    StringDictionary localDictionary;
    pthread_t thread;
    size_t id;
} Carrier;

typedef struct AsyncExtra
{
    Vector ofType(Carrier) carriers;
    Vector ofType(Carrier *) carrierMapping;
    Spinlock lock;
} AsyncExtra;

typedef struct ParallelInsertArg
{
    Vector ofType(char *) strings;
    StringId *out;
    Carrier *carrier;
    bool enableWriteOut;
    bool didWork;
    uint_fast16_t insertNumThreads;
} ParallelInsertArg;

typedef struct ParallelRemoveArg
{
    Vector ofType(StringId) *localIds;
    Carrier *carrier;
    int result;
    bool didWork;
} ParallelRemoveArg;

typedef struct ParallelLocateArg
{
    Carrier *carrier;
    StringId *idsOut;
    bool *foundMaskOut;
    size_t numNotFoundOut;
    Vector ofType(char *) keysIn;
    int result;
    bool didWork;
} ParallelLocateArg;

typedef struct ParallelExtractArg
{
    Vector ofType(StringId) localIdsIn;
    char **stringsOut;
    Carrier *carrier;
    bool didWork;
} ParallelExtractArg;

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define HASHCODE_OF(string)                                                                                            \
    HASH_FUNCTION(strlen(string), string)

#define MAKE_GLOBAL(threadId, localThreadId)                                                                           \
    ((threadId << 54) | localThreadId)

#define GET_OWNER(globalId)                                                                                            \
    (globalId >> 54)

#define GET_STRINGID(globalId)                                                                                         \
    ((~((StringId) 0)) >> 10 & globalStringId);

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int thisDrop(struct StringDictionary *self);
static int thisInsert(struct StringDictionary *self,
                      StringId **out,
                      char *const *strings,
                      size_t numStrings,
                      size_t __numThreads);
static int thisRemove(struct StringDictionary *self, StringId *strings, size_t numStrings);
static int thisLocateSafe(struct StringDictionary *self, StringId **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys);
static int thisLocateFast(struct StringDictionary *self, StringId **out, char *const *keys,
                          size_t numKeys);
static char **thisExtract(struct StringDictionary *self, const StringId *ids, size_t numIds);
static int thisFree(struct StringDictionary *self, void *ptr);

static int thisNumDistinct(struct StringDictionary *self, size_t *num);

static int thisResetCounters(struct StringDictionary *self);
static int thisCounters(struct StringDictionary *self, StringHashCounters *counters);

static int thisLock(struct StringDictionary *self);
static int thisUnlock(struct StringDictionary *self);

static int thisCreateExtra(struct StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                           size_t approxNumUniqueStr, size_t numThreads);
static int thisSetExtra(struct StringDictionary *self, size_t numThreads);

static int thisSetupCarriers(struct StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                             size_t approxNumUniqueStr, size_t numThreads);

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define THIS_EXTRAS(self)                                                                                              \
({                                                                                                                     \
    CHECK_TAG(self->tag, STRING_DIC_ASYNC);                                                                            \
    (AsyncExtra *) self->extra;                                                                                        \
})

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int StringDictionaryCreateAsync(struct StringDictionary *dic, size_t capacity, size_t numIndexBuckets,
                                size_t approxNumUniqueStr, size_t numThreads, const Allocator *alloc)
{
    CHECK_SUCCESS(AllocatorThisOrDefault(&dic->alloc, alloc));

    dic->tag = STRDIC_ASYNC;
    dic->drop = thisDrop;
    dic->insert = thisInsert;
    dic->remove = thisRemove;
    dic->locateSafe = thisLocateSafe;
    dic->locateFast = thisLocateFast;
    dic->extract = thisExtract;
    dic->free = thisFree;
    dic->resetCounters = thisResetCounters;
    dic->counters = thisCounters;
    dic->numDistinct = thisNumDistinct;

    CHECK_SUCCESS(thisCreateExtra(dic, capacity, numIndexBuckets, approxNumUniqueStr, numThreads));
    return STATUS_OK;
}

int StringDictionaryResize(struct StringDictionary *dic, size_t numThreads)
{
  CHECK_SUCCESS(thisSetExtra(dic, numThreads));
  return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static int thisCreateExtra(struct StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                           size_t approxNumUniqueStr, size_t numThreads)
{
    assert(self);

    self->extra = AllocatorMalloc(&self->alloc, sizeof(AsyncExtra));
    AsyncExtra *extra = THIS_EXTRAS(self);
    SpinlockCreate(&extra->lock);
    VectorCreate(&extra->carriers, &self->alloc, sizeof(Carrier), numThreads);
    thisSetupCarriers(self, capacity, numIndexBuckets, approxNumUniqueStr, numThreads);
    VectorCreate(&extra->carrierMapping, &self->alloc, sizeof(Carrier *), capacity);

    return STATUS_OK;
}

static int thisSetExtra(struct StringDictionary *self, size_t numThreads)
{
  assert(self);

  AsyncExtra *extra = THIS_EXTRAS(self);
  CHECK_SUCCESS(VectorGrow(&numThreads, &extra->carriers));
  CHECK_SUCCESS(VectorGrow(&numThreads, &extra->carriers));

  return STATUS_OK;
}

static int thisDrop(struct StringDictionary *self)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);
    AsyncExtra *extra = THIS_EXTRAS(self);
    for (size_t i = 0; i < extra->carriers.numElems; i++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, i, Carrier);
        StringDictionaryDrop(&carrier->localDictionary);
    }
    CHECK_SUCCESS(VectorDrop(&extra->carriers));
    CHECK_SUCCESS(VectorDrop(&extra->carrierMapping));
    CHECK_SUCCESS(AllocatorFree(&self->alloc, extra));
    return STATUS_OK;
}

void *parallelRemoveFunction(void *args)
{
    ParallelRemoveArg *carrierArg = (ParallelRemoveArg *) args;
    StringId len = VectorLength(carrierArg->localIds);
    carrierArg->didWork = len > 0;

    DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for remove task (%zu elements)", carrierArg->carrier->id,
          VectorLength(carrierArg->localIds));
    if (len > 0) {
        struct StringDictionary *dic = &carrierArg->carrier->localDictionary;
        StringId *ids = VECTOR_ALL(carrierArg->localIds, StringId);
        carrierArg->result = StringDictionaryRemove(dic, ids, len);
        DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu task done", carrierArg->carrier->id);
    }
    else {
        carrierArg->result = STATUS_OK;
        WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", carrierArg->carrier->id);
    }

    return NULL;
}

void *parallelInsertFunction(void *args)
{
    ParallelInsertArg *restrict thisArgs = (ParallelInsertArg *restrict) args;
    thisArgs->didWork = thisArgs->strings.numElems > 0;

    TRACE(STRING_DIC_ASYNC_TAG, "thread-local insert function started (thread %zu)", thisArgs->carrier->id);
    DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for insert task (%zu elements)", thisArgs->carrier->id,
          VectorLength(&thisArgs->strings));

    if (thisArgs->didWork) {
        TRACE(STRING_DIC_ASYNC_TAG,
              "thread %zu starts insertion of %zu strings",
              thisArgs->carrier->id,
              VectorLength(&thisArgs->strings));
        char **data = (char **) VectorData(&thisArgs->strings);

        int status = StringDictionaryInsert(&thisArgs->carrier->localDictionary,
                                            thisArgs->enableWriteOut ? &thisArgs->out : NULL,
                                            data, VectorLength(&thisArgs->strings), thisArgs->insertNumThreads);

        PANIC_IF(status != STATUS_OK, "internal error during thread-local string dictionary building process");
        DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

void *parallelLocateSafeFunction(void *args)
{
    ParallelLocateArg *restrict thisArgs = (ParallelLocateArg *restrict) args;
    thisArgs->didWork = VectorLength(&thisArgs->keysIn) > 0;

    TRACE(STRING_DIC_ASYNC_TAG, "thread-local 'locate' function invoked for thread %zu...", thisArgs->carrier->id)

    DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for locate (safe) task (%zu elements)", thisArgs->carrier->id,
          VectorLength(&thisArgs->keysIn));

    if (thisArgs->didWork) {
        thisArgs->result = StringDictionaryLocateSafe(&thisArgs->idsOut,
                                                      &thisArgs->foundMaskOut,
                                                      &thisArgs->numNotFoundOut,
                                                      &thisArgs->carrier->localDictionary,
                                                      VECTOR_ALL(&thisArgs->keysIn, char *),
                                                      VectorLength(&thisArgs->keysIn));

        DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

void *parallelExtractFunction(void *args)
{
    ParallelExtractArg *restrict thisArgs = (ParallelExtractArg *restrict) args;
    thisArgs->didWork = VectorLength(&thisArgs->localIdsIn) > 0;

    DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu spawned for extract task (%zu elements)", thisArgs->carrier->id,
          VectorLength(&thisArgs->localIdsIn));

    if (thisArgs->didWork) {
        thisArgs->stringsOut = StringDictionaryExtract(&thisArgs->carrier->localDictionary,
                                                       VECTOR_ALL(&thisArgs->localIdsIn, StringId),
                                                       VectorLength(&thisArgs->localIdsIn));
        DEBUG(STRING_DIC_ASYNC_TAG, "thread %zu done", thisArgs->carrier->id);
    }
    else {
        WARN(STRING_DIC_ASYNC_TAG, "thread %zu had nothing to do", thisArgs->carrier->id);
    }

    return NULL;
}

static void Synchronize(Vector ofType(Carrier) *carriers, size_t numThreads)
{
    DEBUG(STRING_DIC_ASYNC_TAG, "barrier installed for %d threads", numThreads);

    Timestamp begin = TimeCurrentSystemTime();
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        volatile Carrier *carrier = VECTOR_GET(carriers, threadId, Carrier);
        pthread_join(carrier->thread, NULL);
        DEBUG(STRING_DIC_ASYNC_TAG, "thread %d joined", carrier->id);
    }
    Timestamp end = TimeCurrentSystemTime();
    Timestamp duration = (end - begin);
    UNUSED(duration);

    DEBUG(STRING_DIC_ASYNC_TAG, "barrier passed for %d threads after %f seconds", numThreads, duration / 1000.0f);
}

static void createThreadAssignment(atomic_uint_fast16_t **strCarrierMapping, atomic_size_t **carrierNumStrings,
                                   size_t **strCarrierIdxMapping,
                                   Allocator *alloc, size_t numStrings, size_t numThreads)
{
    /* map string depending on hash value to a particular Carrier */
    *strCarrierMapping = AllocatorMalloc(alloc, numStrings * sizeof(atomic_uint_fast16_t));
    memset(*strCarrierMapping, 0, numStrings * sizeof(atomic_uint_fast16_t));

    /* counters to compute how many strings go to a particular Carrier */
    *carrierNumStrings = AllocatorMalloc(alloc, numThreads * sizeof(atomic_size_t));
    memset(*carrierNumStrings, 0, numThreads * sizeof(atomic_size_t));

    /* an inverted index that contains the i-th position for string k that was assigned to Carrier m.
     * With this, given a (global) string and and its Carrier, one can have directly the position of the
     * string in the carriers "thread-local locate" args */
    *strCarrierIdxMapping = AllocatorMalloc(alloc, numStrings * sizeof(size_t));
}

static void dropThreadAssignment(Allocator *alloc, atomic_uint_fast16_t *strCarrierMapping,
                                 atomic_size_t *carrierNumStrings, size_t *strCarrierIdxMapping)
{
    AllocatorFree(alloc, carrierNumStrings);
    AllocatorFree(alloc, strCarrierMapping);
    AllocatorFree(alloc, strCarrierIdxMapping);
}

typedef struct ParallelComputeThreadAssignmentArg
{
    atomic_uint_fast16_t *strCarrierMapping;
    size_t numThreads;
    atomic_size_t *carrierNumStrings;
    char *const *baseStrings;
} ParallelComputeThreadAssignmentArg;

static void ParallelComputeThreadAssignmentFunction(const void *restrict start, size_t width, size_t len,
                                                    void *restrict args, ThreadId tid)
{
    UNUSED(tid);
    UNUSED(width);

    char *const *strings = (char *const *) start;

    ParallelComputeThreadAssignmentArg *funcArgs = (ParallelComputeThreadAssignmentArg *) args;

    while (len--) {
        size_t i = strings - funcArgs->baseStrings;
        const char *key = *strings;
        /* re-using this hashcode for the thread-local dictionary is more costly than to compute it fresh
         * (due to more I/O with the RAM) */
        size_t threadId = HASHCODE_OF(key) % funcArgs->numThreads;
        atomic_fetch_add(&funcArgs->strCarrierMapping[i], threadId);
        atomic_fetch_add(&funcArgs->carrierNumStrings[threadId], 1);
        strings++;
    }
}

static void computeThreadAssignment(atomic_uint_fast16_t *strCarrierMapping, atomic_size_t *carrierNumStrings,
                                    char *const *strings, size_t numStrings, size_t numThreads)
{
    ParallelComputeThreadAssignmentArg args = {
        .baseStrings = strings,
        .carrierNumStrings = carrierNumStrings,
        .numThreads = numThreads,
        .strCarrierMapping = strCarrierMapping
    };
    ParallelFor(strings,
                sizeof(char *const *),
                numStrings,
                ParallelComputeThreadAssignmentFunction,
                &args,
                ThreadingHint_Multi,
                numThreads);

}

static int thisInsert(struct StringDictionary *self,
                      StringId **out,
                      char *const *strings,
                      size_t numStrings,
                      size_t __numThreads)
{
    Timestamp begin = TimeCurrentSystemTime();
    INFO(STRING_DIC_ASYNC_TAG, "insert operation invoked: %zu strings in total", numStrings)


    CHECK_TAG(self->tag, STRDIC_ASYNC);
    PANIC_IF(__numThreads != 0, "parameter 'numThreads' must be set to 0 for async dictionary")

    thisLock(self);

    AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t numThreads = VectorLength(&extra->carriers);

    atomic_uint_fast16_t *strCarrierMapping;
    size_t *strCarrierIdxMapping;
    atomic_size_t *carrierNumStrings;


    createThreadAssignment(&strCarrierMapping, &carrierNumStrings, &strCarrierIdxMapping,
                           &self->alloc, numStrings, numThreads);

    Vector ofType(ParallelInsertArg *) carrierArgs;
    VectorCreate(&carrierArgs, &self->alloc, sizeof(ParallelInsertArg *), numThreads);

    /* compute which Carrier is responsible for which string */
    computeThreadAssignment(strCarrierMapping, carrierNumStrings, strings, numStrings, numThreads);

    /* prepare to move string subsets to carriers */
    for (uint_fast16_t i = 0; i < numThreads; i++) {
        ParallelInsertArg *entry = AllocatorMalloc(&self->alloc, sizeof(ParallelInsertArg));
        entry->carrier = VECTOR_GET(&extra->carriers, i, Carrier);
        entry->insertNumThreads = numThreads;

        VectorCreate(&entry->strings, &self->alloc, sizeof(char *), MAX(1, carrierNumStrings[i]));
        VectorPush(&carrierArgs, &entry, 1);
        assert (entry->strings.base != NULL);

        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, i, ParallelInsertArg *);
        carrierArg->out = NULL;
    }

    /* create per-Carrier string subset */
    /* parallizing this makes no sense but waste of resources and energy */
    for (size_t i = 0; i < numStrings; i++) {
        uint_fast16_t threadId = strCarrierMapping[i];
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        carrierArg->enableWriteOut = out != NULL;

        /* store local index of string i inside the thread */
        strCarrierIdxMapping[i] = VectorLength(&carrierArg->strings);

        VectorPush(&carrierArg->strings, &strings[i], 1);
    }


    /* schedule insert operation per Carrier */
    TRACE(STRING_DIC_ASYNC_TAG, "schedule insert operation to %zu threads", numThreads)
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        TRACE(STRING_DIC_ASYNC_TAG, "create thread %zu...", threadId)
        pthread_create(&carrier->thread, NULL, parallelInsertFunction, carrierArg);
        TRACE(STRING_DIC_ASYNC_TAG, "thread %zu created", threadId)
    }
    TRACE(STRING_DIC_ASYNC_TAG, "scheduling done for %zu threads", numThreads)

    /* synchronize */
    TRACE(STRING_DIC_ASYNC_TAG, "start synchronizing %zu threads", numThreads)
    Synchronize(&extra->carriers, numThreads);
    TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync", numThreads)

    /* compute string ids; the string id produced by this implementation is a compound identifier encoding
     * both the owning thread id and the thread-local string id. For this, the returned (global) string identifier
     * is split into 10bits encoded the thread (given a maximum of 1024 threads that can be handled by this
     * implementation), and 54bits used to encode the thread-local string id
     *
     * TECHNICAL LIMIT: 1024 threads
     */

    /* optionally, return the created string ids. In case 'out' is NULL, nothing has to be done (especially
     * none of the Carrier threads allocated thread-local 'out's which mean that no cleanup must be done */

    /* parallelizing the following block makes no sense but waste of compute power and energy */
    if (BRANCH_LIKELY(out != NULL)) {
        StringId *totalOut = AllocatorMalloc(&self->alloc, numStrings * sizeof(StringId));
        size_t currentOut = 0;

        for (size_t string_idx = 0; string_idx < numStrings; string_idx++) {
            uint_fast16_t threadId = strCarrierMapping[string_idx];
            size_t localIdx = strCarrierIdxMapping[string_idx];
            ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
            StringId globalStringOwnerId = threadId;
            StringId globalStringLocalId = carrierArg->out[localIdx];
            StringId globalStringId = MAKE_GLOBAL(globalStringOwnerId, globalStringLocalId);
            totalOut[currentOut++] = globalStringId;
        }

        *out = totalOut;
    }

    /* cleanup */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelInsertArg *carrierArg = *VECTOR_GET(&carrierArgs, threadId, ParallelInsertArg *);
        if (carrierArg->didWork) {
            StringDictionaryFree(&carrierArg->carrier->localDictionary, carrierArg->out);
        }
        VectorDrop(&carrierArg->strings);
        AllocatorFree(&self->alloc, carrierArg);
    }

    /* cleanup */
    dropThreadAssignment(&self->alloc, strCarrierMapping, carrierNumStrings, strCarrierIdxMapping);
    VectorDrop(&carrierArgs);

    thisUnlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_ASYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return STATUS_OK;
}

static int thisRemove(struct StringDictionary *self, StringId *strings, size_t numStrings)
{
    Timestamp begin = TimeCurrentSystemTime();
    INFO(STRING_DIC_ASYNC_TAG, "remove operation started: %zu strings to remove", numStrings);

    CHECK_TAG(self->tag, STRDIC_ASYNC);

    thisLock(self);

    ParallelRemoveArg empty;
    struct AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t numThreads = VectorLength(&extra->carriers);
    size_t approxNumStringsPerThread = MAX(1, numStrings / numThreads);
    Vector ofType(StringId) *stringMap = AllocatorMalloc(&self->alloc, numThreads * sizeof(Vector));

    Vector ofType(ParallelRemoveArg) carrierArgs;
    VectorCreate(&carrierArgs, &self->alloc, sizeof(ParallelRemoveArg), numThreads);

    /* prepare thread-local subset of string ids */
    VectorRepreatedPush(&carrierArgs, &empty, numThreads);
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        VectorCreate(stringMap + threadId, &self->alloc, sizeof(StringId), approxNumStringsPerThread);
    }

    /* compute subset of string ids per thread  */
    for (size_t i = 0; i < numStrings; i++) {
        StringId globalStringId = strings[i];
        uint_fast16_t owningThreadId = GET_OWNER(globalStringId);
        StringId localStringId = GET_STRINGID(globalStringId);
        assert(owningThreadId < numThreads);

        VectorPush(stringMap + owningThreadId, &localStringId, 1);
    }

    /* schedule remove operation per Carrier */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelRemoveArg *carrierArg = VECTOR_GET(&carrierArgs, threadId, ParallelRemoveArg);
        carrierArg->carrier = carrier;
        carrierArg->localIds = stringMap + threadId;

        pthread_create(&carrier->thread, NULL, parallelRemoveFunction, carrierArg);
    }

    /* synchronize */
    Synchronize(&extra->carriers, numThreads);

    /* cleanup */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        VectorDrop(stringMap + threadId);
    }

    AllocatorFree(&self->alloc, stringMap);
    VectorData(&carrierArgs);

    thisUnlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_ASYNC_TAG, "remove operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return STATUS_OK;
}

static int thisLocateSafe(struct StringDictionary *self, StringId **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys)
{
    Timestamp begin = TimeCurrentSystemTime();
    INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation started: %zu strings to locate", numKeys)

    CHECK_TAG(self->tag, STRDIC_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    uint_fast16_t numThreads = VectorLength(&extra->carriers);

    /* global result output */
    ALLOCATOR_MALLOC(StringId, globalOut, numKeys, &self->alloc);
    ALLOCATOR_MALLOC(bool, globalFoundMask, numKeys, &self->alloc);

    size_t globalNumNotFound = 0;

    atomic_uint_fast16_t *strCarrierMapping;
    size_t *strCarrierIdxMapping;
    atomic_size_t *carrierNumStrings;

    ParallelLocateArg carrierArgs[numThreads];

    createThreadAssignment(&strCarrierMapping, &carrierNumStrings, &strCarrierIdxMapping,
                           &self->alloc, numKeys, numThreads);

    /* compute which Carrier is responsible for which string */
    computeThreadAssignment(strCarrierMapping, carrierNumStrings, keys, numKeys, numThreads);

    /* prepare to move string subsets to carriers */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelLocateArg *arg = carrierArgs + threadId;
        VectorCreate(&arg->keysIn, &self->alloc, sizeof(char *), carrierNumStrings[threadId]);
        assert (&arg->keysIn.base != NULL);
    }

    TRACE(STRING_DIC_ASYNC_TAG, "computing per-thread string subset for %zu strings", numKeys)
    /* create per-Carrier string subset */
    for (size_t i = 0; i < numKeys; i++) {
        /* get thread responsible for this particular string */
        uint_fast16_t threadId = strCarrierMapping[i];

        /* get the thread-local argument list for the thread that is responsible for this particular string */
        ParallelLocateArg *arg = carrierArgs + threadId;

        /* store local index of string i inside the thread */
        strCarrierIdxMapping[i] = VectorLength(&arg->keysIn);

        /* push that string into the thread-local vector */
        VectorPush(&arg->keysIn, &keys[i], 1);
    }

    TRACE(STRING_DIC_ASYNC_TAG, "schedule operation to threads to %zu threads...", numThreads)
    /* schedule operation to threads */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelLocateArg *arg = carrierArgs + threadId;
        carrierArgs[threadId].carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallelLocateSafeFunction, arg);
    }

    /* synchronize */
    TRACE(STRING_DIC_ASYNC_TAG, "start syncing %zu threads...", numThreads)
    Synchronize(&extra->carriers, numThreads);
    TRACE(STRING_DIC_ASYNC_TAG, "%zu threads in sync.", numThreads)

    /* collect and merge results */
    TRACE(STRING_DIC_ASYNC_TAG, "merging results of %zu threads", numThreads)
    for (size_t i = 0; i < numKeys; i++) {
        /* get thread responsible for this particular string, and local position of that string inside the
         * thread storage */
        uint_fast16_t threadId = strCarrierMapping[i];
        size_t localThreadIdx = strCarrierIdxMapping[i];

        /* get the thread-local argument list for the thread that is responsible for this particular string */
        ParallelLocateArg *arg = carrierArgs + threadId;

        /* merge into global result */
        StringId globalStringId_owner = threadId;
        StringId globalStringId_localIdx = arg->idsOut[localThreadIdx];
        StringId globalStringId = MAKE_GLOBAL(globalStringId_owner, globalStringId_localIdx);
        globalOut[i] = globalStringId;
        globalFoundMask[i] = arg->foundMaskOut[localThreadIdx];
    }
    for (size_t threadId = 0; threadId < numThreads; threadId++) {
        /* compute total number of not-found elements */
        ParallelLocateArg *arg = carrierArgs + threadId;
        globalNumNotFound += arg->numNotFoundOut;

        /* cleanup */
        if (BRANCH_LIKELY(arg->didWork)) {
            StringDictionaryFree(&arg->carrier->localDictionary, arg->foundMaskOut);
            StringDictionaryFree(&arg->carrier->localDictionary, arg->idsOut);
        }
    }

    TRACE(STRING_DIC_ASYNC_TAG, "cleanup%s", "...")

    /* cleanup */
    dropThreadAssignment(&self->alloc, strCarrierMapping, carrierNumStrings, strCarrierIdxMapping);

    for (size_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelLocateArg *arg = carrierArgs + threadId;
        VectorDrop(&arg->keysIn);
    }

    /* return results */
    *out = globalOut;
    *foundMask = globalFoundMask;
    *numNotFound = globalNumNotFound;

    thisUnlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_ASYNC_TAG, "locate (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return STATUS_OK;
}

static int thisLocateFast(struct StringDictionary *self, StringId **out, char *const *keys,
                          size_t numKeys)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);

    thisLock(self);

    bool *foundMask;
    size_t numNotFound;
    int result;

    /* use safer but in principle more slower implementation */
    result = thisLocateSafe(self, out, &foundMask, &numNotFound, keys, numKeys);

    /* cleanup */
    thisFree(self, foundMask);

    thisUnlock(self);

    return result;
}

static char **thisExtract(struct StringDictionary *self, const StringId *ids, size_t numIds)
{
    Timestamp begin = TimeCurrentSystemTime();
    INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation started: %zu strings to extract", numIds)

    if (self->tag != STRDIC_ASYNC) {
        return NULL;
    }

    thisLock(self);

    ALLOCATOR_MALLOC(char *, globalResult, numIds, &self->alloc);

    struct AsyncExtra *extra = (struct AsyncExtra *) self->extra;
    uint_fast16_t numThreads = VectorLength(&extra->carriers);
    size_t approxNumStringsPerThread = MAX(1, numIds / numThreads);

    ALLOCATOR_MALLOC(size_t, localThreadIdx, numIds, &self->alloc);
    ALLOCATOR_MALLOC(uint_fast16_t, owningThreadIds, numIds, &self->alloc);
    ALLOCATOR_MALLOC(ParallelExtractArg, threadArgs, numThreads, &self->alloc);

    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelExtractArg *arg = threadArgs + threadId;
        VectorCreate(&arg->localIdsIn, &self->alloc, sizeof(StringId), approxNumStringsPerThread);
    }

    /* compute subset of string ids per thread  */
    for (size_t i = 0; i < numIds; i++) {
        StringId globalStringId = ids[i];
        owningThreadIds[i] = GET_OWNER(globalStringId);
        StringId localStringId = GET_STRINGID(globalStringId);
        assert(owningThreadIds[i] < numThreads);

        ParallelExtractArg *arg = threadArgs + owningThreadIds[i];
        localThreadIdx[i] = VectorLength(&arg->localIdsIn);
        VectorPush(&arg->localIdsIn, &localStringId, 1);
    }

    /* schedule remove operation per Carrier */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        ParallelExtractArg *carrierArg = threadArgs + threadId;
        carrierArg->carrier = carrier;
        pthread_create(&carrier->thread, NULL, parallelExtractFunction, carrierArg);
    }

    /* synchronize */
    Synchronize(&extra->carriers, numThreads);

    for (size_t i = 0; i < numIds; i++) {
        uint_fast16_t owningThreadId = owningThreadIds[i];
        size_t localIdx = localThreadIdx[i];
        ParallelExtractArg *carrierArg = threadArgs + owningThreadId;
        char *extractedString = carrierArg->stringsOut[localIdx];
        globalResult[i] = extractedString;
    }

    /* cleanup */
    for (uint_fast16_t threadId = 0; threadId < numThreads; threadId++) {
        ParallelExtractArg *carrierArg = threadArgs + threadId;
        VectorDrop(&carrierArg->localIdsIn);
        if (BRANCH_LIKELY(carrierArg->didWork)) {
            StringDictionaryFree(&carrierArg->carrier->localDictionary, carrierArg->stringsOut);
        }
    }

    ALLOCATOR_FREE(localThreadIdx, &self->alloc);
    ALLOCATOR_FREE(owningThreadIds, &self->alloc);
    ALLOCATOR_FREE(threadArgs, &self->alloc);

    thisUnlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_ASYNC_TAG, "extract (safe) operation done: %f seconds spent here", (end - begin) / 1000.0f)

    return globalResult;
}

static int thisFree(struct StringDictionary *self, void *ptr)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);
    AllocatorFree(&self->alloc, ptr);
    return STATUS_OK;
}

static int thisNumDistinct(struct StringDictionary *self, size_t *num)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);
    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t numCarriers = VectorLength(&extra->carriers);
    Carrier *carriers = VECTOR_ALL(&extra->carriers, Carrier);
    size_t numDistinct = 0;
    while (numCarriers--) {
        size_t local_distinct;
        StringDictionaryNumDistinct(&local_distinct, &carriers->localDictionary);
        numDistinct += local_distinct;
        carriers++;
    }
    *num = numDistinct;
    thisUnlock(self);
    return STATUS_OK;
}

static int thisResetCounters(struct StringDictionary *self)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t numThreads = VectorLength(&extra->carriers);

    for (size_t threadId = 0; threadId < numThreads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        StringDictionaryResetCounters(&carrier->localDictionary);
    }

    thisUnlock(self);

    return STATUS_OK;
}

static int thisCounters(struct StringDictionary *self, StringHashCounters *counters)
{
    CHECK_TAG(self->tag, STRDIC_ASYNC);

    thisLock(self);

    struct AsyncExtra *extra = THIS_EXTRAS(self);
    size_t numThreads = VectorLength(&extra->carriers);

    CHECK_SUCCESS(StringHashTableCountersInit(counters));

    for (size_t threadId = 0; threadId < numThreads; threadId++) {
        Carrier *carrier = VECTOR_GET(&extra->carriers, threadId, Carrier);
        StringHashCounters local_counters;
        StringDictionaryGetCounters(&local_counters, &carrier->localDictionary);
        StringHashTableCountersAdd(counters, &local_counters);
    }

    thisUnlock(self);

    return STATUS_OK;
}

typedef struct ParallelCreateCarrierArg
{
    size_t localCapacity;
    size_t localBucketNum;
    size_t localBucketCap;
    const Allocator *alloc;
} ParallelCreateCarrierArg;

static void parallelCreateCarrier(const void *restrict start, size_t width, size_t len, void *restrict args,
                                  ThreadId tid)
{
    UNUSED(tid);
    UNUSED(width);

    Carrier *carrier = (Carrier *) start;
    const ParallelCreateCarrierArg *createArgs = (const ParallelCreateCarrierArg *) args;
    while (len--) {
        StringDicationaryCreateSync(&carrier->localDictionary, createArgs->localCapacity, createArgs->localBucketNum,
                                    createArgs->localBucketCap, 0, createArgs->alloc);
        memset(&carrier->thread, 0, sizeof(pthread_t));
        carrier++;
    }
}

static int thisSetupCarriers(struct StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                             size_t approxNumUniqueStr, size_t numThreads)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    size_t localBucketNum = MAX(1, numIndexBuckets / numThreads);
    Carrier new_carrier;

    ParallelCreateCarrierArg createArgs = {
        .localCapacity = MAX(1, capacity / numThreads),
        .localBucketNum = localBucketNum,
        .localBucketCap = MAX(1, approxNumUniqueStr / numThreads / localBucketNum / SLICE_KEY_COLUMN_MAX_ELEMS),
        .alloc = &self->alloc
    };

    for (size_t threadId = 0; threadId < numThreads; threadId++) {
        new_carrier.id = threadId;
        VectorPush(&extra->carriers, &new_carrier, 1);
    }

    ParallelFor(VECTOR_ALL(&extra->carriers, Carrier), sizeof(Carrier), numThreads, parallelCreateCarrier,
                &createArgs, ThreadingHint_Multi, numThreads);

    return STATUS_OK;
}

static int thisLock(struct StringDictionary *self)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    CHECK_SUCCESS(SpinlockAcquire(&extra->lock));
    return STATUS_OK;
}

static int thisUnlock(struct StringDictionary *self)
{
    AsyncExtra *extra = THIS_EXTRAS(self);
    CHECK_SUCCESS(SpinlockRelease(&extra->lock));
    return STATUS_OK;
}
