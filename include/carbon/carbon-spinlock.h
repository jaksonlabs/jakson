/*
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

#ifndef CARBON_SPINLOCK_H
#define CARBON_SPINLOCK_H

#include "carbon-common.h"
#include "carbon-vector.h"

CARBON_BEGIN_DECL

typedef struct carbon_spinlock
{
    atomic_flag lock;
    pthread_t owner;
} carbon_spinlock_t;

CARBON_EXPORT(bool)
carbon_spinlock_init(carbon_spinlock_t *spinlock);

CARBON_EXPORT(bool)
carbon_spinlock_acquire(carbon_spinlock_t *spinlock);

CARBON_EXPORT(bool)
carbon_spinlock_release(carbon_spinlock_t *spinlock);

CARBON_END_DECL

#endif
