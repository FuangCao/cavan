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

int cavan_lock_init(cavan_lock_t *lock)
{
	pthread_mutexattr_t attr;
	int ret;

	ret = pthread_mutexattr_init(&attr);
	if (ret < 0) {
		pr_err_info("pthread_mutexattr_init: %d", ret);
		return ret;
	}

	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	if (ret < 0) {
		pr_err_info("pthread_mutexattr_settype: %d", ret);
		goto out_pthread_mutexattr_destroy;
	}

	ret = pthread_mutex_init(lock, &attr);
	if (ret < 0) {
		pr_err_info("pthread_mutexattr_init: %d", ret);
		goto out_pthread_mutexattr_destroy;
	}

out_pthread_mutexattr_destroy:
	pthread_mutexattr_destroy(&attr);
	return ret;
}
