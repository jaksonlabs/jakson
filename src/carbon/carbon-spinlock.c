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

#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-time.h"

#define SPINLOCK_TAG "spinlock"

bool carbon_spinlock_init(carbon_spinlock_t *spinlock)
{
    CARBON_NON_NULL_OR_ERROR(spinlock)
#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear(&spinlock->lock);
#else
    spinlock->mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

    memset(&spinlock->owner, 0, sizeof(pthread_t));


    return true;
}

bool carbon_spinlock_acquire(carbon_spinlock_t *spinlock)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_NON_NULL_OR_ERROR(spinlock)
    if (!pthread_equal(spinlock->owner, pthread_self())) {

#ifndef __STDC_NO_ATOMICS__
        while (atomic_flag_test_and_set(&spinlock->lock));
#else
        pthread_mutex_lock(&spinlock->mutex);
#endif
        /** remember the thread that aquires this lock */
        spinlock->owner = pthread_self();
    }
    carbon_timestamp_t end = carbon_time_now_wallclock();
    float duration = (end - begin) / 1000.0f;
    if (duration > 0.01f) {
        CARBON_WARN(SPINLOCK_TAG, "spin lock acquisition took exceptionally long: %f seconds", duration);
    }

    return true;
}

bool carbon_spinlock_release(carbon_spinlock_t *spinlock)
{
    CARBON_NON_NULL_OR_ERROR(spinlock)

#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear(&spinlock->lock);
#else
    pthread_mutex_lock(&mutex);
#endif

    memset(&spinlock->owner, 0, sizeof(pthread_t));
    return true;
}