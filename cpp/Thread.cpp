/*
 * File:		Thread.cpp
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
#include <cavan++/thread.h>

#define CAVAN_THREAD_DEBUG 0

static void ThreadSignal(int signum)
{
	pr_bold_info("signum = %d", signum);

	if (signum == SIGUSR1)
	{
		exit(0);
	}
}

static void *ThreadMain(void *data)
{
	signal(SIGUSR1, ThreadSignal);
	((Thread *)data)->mainLoop();

	return NULL;
}

Thread::Thread(const char *name, cavan_thread_handler_t handler) : mLock(), mCond()
{
	mName = name;
	mPending = false;
	mHandler = handler;
	mState = CAVAN_THREAD_STATE_NONE;
}

int Thread::run(void)
{
	if (mHandler == NULL)
	{
		pr_red_info("mHandler == NULL");
		return -EINVAL;
	}

	return mHandler(this);
}

void Thread::mainLoop(void)
{
	AutoLock lock(mLock);

	mState = CAVAN_THREAD_STATE_RUNNING;

	while (1)
	{
		switch (mState)
		{
		case CAVAN_THREAD_STATE_RUNNING:
#if CAVAN_THREAD_DEBUG
			pr_bold_info("Thread %s running", mName);
#endif
			do {
				mLock.release();
				int ret = run();
				mLock.acquire();
				if (ret < 0)
				{
					goto out_thread_stopped;
				}
			} while (mState == CAVAN_THREAD_STATE_RUNNING);
			break;

		case CAVAN_THREAD_STATE_SUSPEND:
			if (mPending)
			{
#if CAVAN_THREAD_DEBUG
				pr_bold_info("Thread %s pending", mName);
#endif
				mState = CAVAN_THREAD_STATE_RUNNING;
				mPending = false;
				break;
			}

#if CAVAN_THREAD_DEBUG
			pr_bold_info("Thread %s suspend", mName);
#endif

			do {
				mLock.release();
				wait();
				mLock.acquire();
			} while (mState == CAVAN_THREAD_STATE_SUSPEND);
			break;

		case CAVAN_THREAD_STATE_STOPPPING:
#if CAVAN_THREAD_DEBUG
			pr_bold_info("Thread %s stopping", mName);
#endif
		case CAVAN_THREAD_STATE_STOPPED:
			goto out_thread_stopped;

		default:
			pr_red_info("invalid state = %d", mState);
		}
	}

out_thread_stopped:
#if CAVAN_THREAD_DEBUG
	pr_red_info("Thread %s stopped", mName);
#endif
	mState = CAVAN_THREAD_STATE_STOPPED;
}

int Thread::start(void)
{
	AutoLock lock(mLock);

	mState = CAVAN_THREAD_STATE_IDLE;

	int ret = pthread_create(&mThread, NULL, ThreadMain, this);
	if (ret < 0)
	{
		pr_error_info("pthread_create");
		mState = CAVAN_THREAD_STATE_NONE;
		return ret;
	}

	do {
		mLock.release();
		msleep(10);
		mLock.acquire();
	} while (mState == CAVAN_THREAD_STATE_IDLE);

	return 0;
}

int Thread::stop(void)
{
	AutoLock lock(mLock);

	switch (mState)
	{
	case CAVAN_THREAD_STATE_RUNNING:
	case CAVAN_THREAD_STATE_SUSPEND:
	case CAVAN_THREAD_STATE_IDLE:
		mState = CAVAN_THREAD_STATE_STOPPPING;
	case CAVAN_THREAD_STATE_STOPPPING:
		for (int i = 100; mState != CAVAN_THREAD_STATE_STOPPED && i > 0; i--)
		{
			mLock.release();
			wakeup();
			msleep(10);
			mLock.acquire();
		}

		if (mState != CAVAN_THREAD_STATE_STOPPED)
		{
			pthread_kill(mThread, SIGUSR1);
			mState = CAVAN_THREAD_STATE_STOPPED;
		}
		break;

	default:
		pr_green_info("Thread %s is suppend", mName);
	}

	return 0;
}

int Thread::suspend(void)
{
	AutoLock lock(mLock);

	if (mState == CAVAN_THREAD_STATE_RUNNING)
	{
		mState = CAVAN_THREAD_STATE_SUSPEND;
	}

	return 0;
}

int Thread::resume(void)
{
	AutoLock lock(mLock);

	if (mState == CAVAN_THREAD_STATE_SUSPEND)
	{
		mPending = true;
		mState = CAVAN_THREAD_STATE_RUNNING;
		wakeup();
	}

	return 0;
}
