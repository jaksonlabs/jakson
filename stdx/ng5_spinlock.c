#include <stdx/ng5_spinlock.h>
#include <stdx/ng5_time.h>

#define SPINLOCK_TAG "spinlock"

int spinlock_create(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    atomic_flag_clear(&spinlock->lock);

    memset(&spinlock->owning_thread, 0, sizeof(pthread_t));

    return STATUS_OK;
}

int spinlock_lock(struct spinlock *spinlock)
{
    timestamp_t begin = time_current_time_ms();
    check_non_null(spinlock)
    if (!pthread_equal(spinlock->owning_thread, pthread_self())) {
        while(atomic_flag_test_and_set(&spinlock->lock))
            ;
        /* remeber the thread that aquires this lock */
        spinlock->owning_thread = pthread_self();
    }
    timestamp_t end = time_current_time_ms();
    float duration = (end-begin)/1000.0f;
    if (duration > 0.01f) {
        warn(SPINLOCK_TAG, "spin lock acquisition takes exceptionally long: %f seconds", duration);
    }

    return STATUS_OK;
}

int spinlock_unlock(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    atomic_flag_clear(&spinlock->lock);
    memset(&spinlock->owning_thread, 0, sizeof(pthread_t));
    return STATUS_OK;
}