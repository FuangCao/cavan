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
	if (acquire && MutexLock::acquire() == 0)
	{
		mOwner = pthread_self();
		mHeldCount = 1;
	}
	else
	{
		mOwner = 0;
		mHeldCount = 0;
	}
}

int ThreadLock::acquire(bool trylock)
{
	pthread_t owner;

	owner = pthread_self();
	if (isHeldBy(owner))
	{
		mHeldCount++;
	}
	else
	{
		int ret = trylock ? MutexLock::tryLock() : MutexLock::acquire();

		if (ret < 0)
		{
			return ret;
		}

		mOwner = owner;
		mHeldCount = 1;
	}

	return 0;
}

int ThreadLock::release(void)
{
	if (!isHeld() || --mHeldCount > 0)
	{
		return 0;
	}

	if (mHeldCount < 0)
	{
		pr_red_info("unbalanced %s %d", __FUNCTION__, mHeldCount);
		mHeldCount = 0;
	}

	mOwner = 0;

	return MutexLock::release();
}
