/*
 * File:		lock.c
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
#include <cavan/lock.h>

void cavan_lock_init(cavan_lock_t *lock)
{
	pthread_mutex_init(&lock->mutex, NULL);
	pthread_mutex_init(&lock->lock, NULL);
	lock->owner = 0;
	lock->count = 0;
}

void cavan_lock_deinit(cavan_lock_t *lock)
{
	pthread_mutex_destroy(&lock->mutex);
	pthread_mutex_destroy(&lock->lock);
}

void cavan_lock_acquire(cavan_lock_t *lock)
{
	pthread_t owner = pthread_self();

	pthread_mutex_lock(&lock->lock);

	if (lock->count > 0 && pthread_equal(owner, lock->owner)) {
		lock->count++;
	} else {
		pthread_mutex_unlock(&lock->lock);
		pthread_mutex_lock(&lock->mutex);
		pthread_mutex_lock(&lock->lock);
		lock->owner = owner;
		lock->count = 1;
	}

	pthread_mutex_unlock(&lock->lock);
}

void cavan_lock_release(cavan_lock_t *lock)
{
	pthread_t owner = pthread_self();

	pthread_mutex_lock(&lock->lock);

	if (pthread_equal(owner, lock->owner) && --lock->count == 0) {
		pthread_mutex_unlock(&lock->mutex);
	}

	pthread_mutex_unlock(&lock->lock);
}
