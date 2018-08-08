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

#ifndef _NG5_ASYNC
#define _NG5_ASYNC

#include <common.h>
#include <stdx/vector.h>

struct spinlock
{
  atomic_flag lock;
  bool        owns_lock;
} ;

int spinlock_create(struct spinlock *spinlock);

int spinlock_lock(struct spinlock *spinlock);

int spinlock_unlock(struct spinlock *spinlock);


#endif
