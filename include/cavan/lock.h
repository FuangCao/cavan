#pragma once

/*
 * File:		lock.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-01-12 11:55:24
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>

#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER		PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#endif

#define CAVAN_LOCK_INITIALIZER					PTHREAD_RECURSIVE_MUTEX_INITIALIZER

typedef pthread_mutex_t cavan_lock_t;

int cavan_lock_init(cavan_lock_t *lock);

static inline void cavan_lock_deinit(cavan_lock_t *lock)
{
	pthread_mutex_destroy(lock);
}

static inline void cavan_lock_acquire(cavan_lock_t *lock)
{
	pthread_mutex_lock(lock);
}

static inline void cavan_lock_release(cavan_lock_t *lock)
{
	pthread_mutex_unlock(lock);
}
