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

#define CAVAN_LOCK_INITIALIZER \
	{ .mutex = PTHREAD_MUTEX_INITIALIZER, .lock = PTHREAD_MUTEX_INITIALIZER, .owner = 0, .count = 0 }

typedef struct cavan_lock {
	pthread_mutex_t mutex;
	pthread_mutex_t lock;
	pthread_t owner;
	int count;
} cavan_lock_t;

void cavan_lock_init(cavan_lock_t *lock);
void cavan_lock_deinit(cavan_lock_t *lock);
void cavan_lock_acquire(cavan_lock_t *lock);
void cavan_lock_release(cavan_lock_t *lock);
