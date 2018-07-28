#include <stdx/asnyc.h>

void spinlock_create(struct spinlock *spinlock)
{
    atomic_flag_clear(&spinlock->lock);
    spinlock->owns_lock = false;
}

void spinlock_lock(struct spinlock *spinlock)
{
    if (!spinlock->owns_lock) {
        while(atomic_flag_test_and_set(&spinlock->lock))
            ;
        spinlock->owns_lock = true;
    }
}

void spinlock_unlock(struct spinlock *spinlock)
{
    atomic_flag_clear(&spinlock->lock);
    spinlock->owns_lock = false;
}