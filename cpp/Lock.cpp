/*
 * File:		Lock.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-23 14:51:20
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan++/Lock.h>

ThreadLock::ThreadLock(bool acquire)
{
	if (acquire && MutexLock::acquire() == 0) {
		mOwner = pthread_self();
	} else {
		mOwner = 0;
	}
}

int ThreadLock::acquire(bool trylock)
{
	int ret;
	pthread_t owner;

	owner = pthread_self();
	if (isHeldBy(owner))
	{
		return 0;
	}

	ret = trylock ? MutexLock::tryLock() : MutexLock::acquire();
	if (ret == 0) {
		mOwner = owner;
	}

	return ret;
}

int ThreadLock::release(void)
{
	if (isHeld())
	{
		mOwner = 0;

		return MutexLock::release();
	}

	return 0;
}
