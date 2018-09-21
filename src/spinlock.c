// file: spinlock.c

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

#include "spinlock.h"
#include "time.h"

#define SPINLOCK_TAG "spinlock"

int SpinlockCreate(Spinlock *spinlock)
{
    CHECK_NON_NULL(spinlock)
    atomic_flag_clear(&spinlock->lock);

    memset(&spinlock->owner, 0, sizeof(pthread_t));

    return STATUS_OK;
}

int SpinlockAcquire(Spinlock *spinlock)
{
    Timestamp begin = TimeCurrentSystemTime();
    CHECK_NON_NULL(spinlock)
    if (!pthread_equal(spinlock->owner, pthread_self())) {
        while (atomic_flag_test_and_set(&spinlock->lock));
        /* remeber the thread that aquires this lock */
        spinlock->owner = pthread_self();
    }
    Timestamp end = TimeCurrentSystemTime();
    float duration = (end - begin) / 1000.0f;
    if (duration > 0.01f) {
        WARN(SPINLOCK_TAG, "spin lock acquisition took exceptionally long: %f seconds", duration);
    }

    return STATUS_OK;
}

int SpinlockRelease(Spinlock *spinlock)
{
    CHECK_NON_NULL(spinlock)
    atomic_flag_clear(&spinlock->lock);
    memset(&spinlock->owner, 0, sizeof(pthread_t));
    return STATUS_OK;
}