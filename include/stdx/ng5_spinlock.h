// file: asnyc.h

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

#pragma once

#ifndef _NG5_SPINLOCK
#define _NG5_SPINLOCK

#include <ng5_common.h>
#include <stdx/ng5_vector.h>

typedef struct ng5_spinlock
{
  atomic_flag lock;
  pthread_t owning_thread;
} ng5_spinlock_t;

int ng5_spinlock_create(struct ng5_spinlock* spinlock);

int ng5_spinlock_lock(struct ng5_spinlock* spinlock);

int ng5_spinlock_unlock(struct ng5_spinlock* spinlock);


#endif
