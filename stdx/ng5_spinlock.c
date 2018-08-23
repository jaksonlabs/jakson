#include <stdx/ng5_spinlock.h>

int spinlock_create(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    atomic_flag_clear(&spinlock->lock);

    memset(&spinlock->owning_thread, 0, sizeof(pthread_t));

    return STATUS_OK;
}

int spinlock_lock(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    if (!pthread_equal(spinlock->owning_thread, pthread_self())) {
        while(atomic_flag_test_and_set(&spinlock->lock))
            ;
        /* remeber the thread that aquires this lock */
        spinlock->owning_thread = pthread_self();
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