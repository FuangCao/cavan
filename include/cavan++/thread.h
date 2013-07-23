#pragma once

/*
 * File:		thread.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-23 14:51:57
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
#include <cavan/thread.h>
#include <cavan++/lock.h>

class Thread;

typedef int (*cavan_thread_handler_t)(Thread *thread);

class Thread
{
private:
	void *mData;
	const char *mName;
	bool mPending;
	pthread_t mThread;
	MutexLock mLock;
	Condition mCond;
	cavan_thread_state_t mState;
	cavan_thread_handler_t mHandler;

public:
	Thread(cavan_thread_handler_t handler = NULL, const char *name = "None");
	int start(void);
	int stop(void);
	int suspend(void);
	int resume(void);
	int mainLoop(void);

	virtual int handler(void);

	int wait()
	{
		return mCond.wait();
	}

	int timedwait(u32 ms)
	{
		return mCond.timedwait(ms);
	}

	int join(void **retval = NULL)
	{
		return pthread_join(mThread, retval);
	}

	int wakeup(void)
	{
		return mCond.signal();
	}

	void setHandler(cavan_thread_handler_t handler)
	{
		mHandler = handler;
	}

	cavan_thread_state_t getState(void)
	{
		AutoLock lock(mLock);
		return mState;
	}

	void setState(cavan_thread_state_t state)
	{
		AutoLock lock(mLock);
		mState = state;
		mCond.signal();
	}

	void *getData(void)
	{
		return mData;
	}

	void setData(void *data)
	{
		mData = data;
	}

	bool isPending(void)
	{
		AutoLock lock(mLock);
		return mPending;
	}

	void setPending(bool pending = true)
	{
		mPending = pending;
	}
};
