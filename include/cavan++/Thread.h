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

typedef void (*cavan_thread_handler_t)(CavanThread *thread);

class CavanThread {
private:
	void *mData;
	cavan_thread_handler_t mHandler;
	struct cavan_thread mThread;

	static int CavanThreadHandler(struct cavan_thread *thread, void *data);

protected:
	virtual void run(void);

public:
	CavanThread(const char *name, cavan_thread_handler_t handler = NULL, int flags = 0);
	virtual ~CavanThread(void);

	void *getData(void) {
		return mData;
	}

	void setData(void *data) {
		mData = data;
	}

	virtual int sendEvent(u32 event) {
		return cavan_thread_send_event(&mThread, event);
	}

	virtual int recvEvent(u32 *event) {
		return cavan_thread_recv_event(&mThread, event);
	}

	virtual int recvEventTimeout(u32 *event, u32 msec) {
		return cavan_thread_recv_event_timeout(&mThread, event, msec);
	}

	virtual int waitEvent(u32 msec) {
		return cavan_thread_wait_event(&mThread, msec);
	}

	virtual int start(void) {
		return cavan_thread_start(&mThread);
	}

	virtual void stop(void) {
		cavan_thread_stop(&mThread);
	}

	virtual void suspend(void) {
		cavan_thread_suspend(&mThread);
	}

	virtual void resume(void) {
		cavan_thread_resume(&mThread);
	}

	virtual int msleepUntil(struct timespec *time) {
		return cavan_thread_sleep_until(&mThread, time);
	}

	virtual int msleep(u32 msec) {
		return cavan_thread_msleep(&mThread, msec);
	}

	virtual void setState(cavan_thread_state_t state) {
		cavan_thread_set_state(&mThread, state);
	}

	virtual void shouldStop(void) {
		cavan_thread_should_stop(&mThread);
	}

	virtual int join(void) {
		return cavan_thread_join(&mThread);
	}

	virtual int wakeup(void) {
		return cavan_thread_wakeup(&mThread);
	}
};

class SimpleThread {
private:
	pthread_t mThread;

public:
	SimpleThread(void) : mThread(-1) {}
	virtual ~SimpleThread() {}

private:
	static void SigKillHandler(int signum)
	{
		pd_bold_info("signum = %d", signum);
		pthread_exit(0);
	}

	static void *ThreadHandler(void *data) {
		SimpleThread *thread = (SimpleThread *) data;

		signal(SIGUSR1, SigKillHandler);

		thread->onStarted();
		thread->run();
		thread->onStopped();
		thread->detach();

		return NULL;
	}

public:
	virtual int start(void) {
		return pthread_create(&mThread, NULL, ThreadHandler, this);
	}

	virtual int kill(void) {
		return pthread_kill(mThread, SIGUSR1);
	}

	virtual int join(void) {
		return pthread_join(mThread, NULL);
	}

	virtual int detach(void) {
		return pthread_detach(mThread);
	}

protected:
	virtual void run(void) = 0;
	virtual void onStarted(void) {}
	virtual void onStopped(void) {}
};
