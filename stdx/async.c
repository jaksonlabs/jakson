#include <stdx/async.h>

int spinlock_create(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    atomic_flag_clear(&spinlock->lock);
    spinlock->owns_lock = false;
    return STATUS_OK;
}

int spinlock_lock(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    if (!spinlock->owns_lock) {
        while(atomic_flag_test_and_set(&spinlock->lock))
            ;
        spinlock->owns_lock = true;
    }
    return STATUS_OK;
}

int spinlock_unlock(struct spinlock *spinlock)
{
    check_non_null(spinlock)
    atomic_flag_clear(&spinlock->lock);
    spinlock->owns_lock = false;
    return STATUS_OK;
}