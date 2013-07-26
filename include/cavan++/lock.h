#pragma once

/*
 * File:		lock.h
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
#include <cavan/timer.h>

class ILock
{
public:
	virtual int acquire(void) = 0;
	virtual int release(void) = 0;

	virtual int tryLock(void)
	{
		return -ENOENT;
	}

	int get(void)
	{
		return acquire();
	}

	int put(void)
	{
		return release();
	}

	int lock(void)
	{
		return acquire();
	}

	int unlock(void)
	{
		return release();
	}
};

class MutexLock : public ILock
{
private:
	pthread_mutex_t mLock;

public:
	MutexLock(const pthread_mutexattr_t *attr = NULL)
	{
		pthread_mutex_init(&mLock, attr);
	}

	~MutexLock(void)
	{
		pthread_mutex_destroy(&mLock);
	}

	virtual int acquire(void)
	{
		return pthread_mutex_lock(&mLock);
	}

	virtual int release(void)
	{
		return pthread_mutex_unlock(&mLock);
	}

	virtual int tryLock()
	{
		return pthread_mutex_trylock(&mLock);
	}
};

class AutoLock
{
private:
	ILock &mLock;

public:
	AutoLock(ILock &lock) : mLock(lock)
	{
		mLock.acquire();
	}

	~AutoLock()
	{
		mLock.release();
	}
};

class Condition
{
private:
	pthread_mutex_t mLock;
	pthread_cond_t mCond;

public:
	Condition(const pthread_condattr_t *attr = NULL)
	{
		pthread_mutex_init(&mLock, NULL);
		pthread_cond_init(&mCond, attr);
	}

	~Condition(void)
	{
		pthread_cond_destroy(&mCond);
		pthread_mutex_destroy(&mLock);
	}

	int signal(void)
	{
		return pthread_cond_signal(&mCond);
	}

	int broadcast(void)
	{
		return pthread_cond_broadcast(&mCond);
	}

	int notify(void)
	{
		return signal();
	}

	int notifyAll(void)
	{
		return broadcast();
	}

	int wait(void)
	{
		pthread_mutex_lock(&mLock);
		int ret = pthread_cond_wait(&mCond, &mLock);
		pthread_mutex_unlock(&mLock);

		return ret;
	}

	int wait(const struct timespec *abstime)
	{
		pthread_mutex_lock(&mLock);
		int ret = pthread_cond_timedwait(&mCond, &mLock, abstime);
		pthread_mutex_unlock(&mLock);

		return ret;
	}

	int wait(u32 ms)
	{
		struct timespec abstime;

		cavan_timer_set_timespec(&abstime, ms);
		return wait(&abstime);
	}
};
