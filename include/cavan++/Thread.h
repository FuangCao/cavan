#pragma once

/*
 * File:		Thread.h
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
#include <cavan++/Lock.h>

class CavanThread;

typedef int (*cavan_thread_handler_t)(CavanThread *thread);

class CavanThread
{
private:
	void *mData;
	cavan_thread_handler_t mHandler;
	struct cavan_thread mThread;

	static int CavanThreadHandler(struct cavan_thread *_thread, void *data);

protected:
	virtual int run(void);

public:
	CavanThread(const char *name, cavan_thread_handler_t handler = NULL);
	virtual ~CavanThread(void);

	void *getData(void)
	{
		return mData;
	}

	void setData(void *data)
	{
		mData = data;
	}

	int sendEvent(u32 event)
	{
		return cavan_thread_send_event(&mThread, event);
	}

	int recvEvent(u32 *event)
	{
		return cavan_thread_recv_event(&mThread, event);
	}

	int recvEventTimeout(u32 *event, u32 msec)
	{
		return cavan_thread_recv_event_timeout(&mThread, event, msec);
	}

	int waitEvent(u32 msec)
	{
		return cavan_thread_wait_event(&mThread, msec);
	}

	int start(void)
	{
		return cavan_thread_start(&mThread);
	}

	void stop(void)
	{
		cavan_thread_stop(&mThread);
	}

	void suspend(void)
	{
		cavan_thread_suspend(&mThread);
	}

	void resume(void)
	{
		cavan_thread_resume(&mThread);
	}

	int msleepUntil(struct timespec *time)
	{
		return cavan_thread_msleep_until(&mThread, time);
	}

	int msleep(u32 msec)
	{
		return cavan_thread_msleep(&mThread, msec);
	}

	void setState(cavan_thread_state_t state)
	{
		cavan_thread_set_state(&mThread, state);
	}

	void shouldStop(void)
	{
		cavan_thread_should_stop(&mThread);
	}

	int join(void)
	{
		return cavan_thread_join(&mThread);
	}

	int wakeup(void)
	{
		return cavan_thread_wakeup(&mThread);
	}
};
