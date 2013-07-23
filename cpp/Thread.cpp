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

Thread::Thread(cavan_thread_handler_t handler, const char *name) : mLock(), mCond()
{
	mName = name;
	mHandler = handler;
	mState = CAVAN_THREAD_STATE_NONE;
}

int Thread::handler(void)
{
	if (mHandler == NULL)
	{
		pr_red_info("mHandler == NULL");
		return -EINVAL;
	}

	return mHandler(this);
}

int Thread::mainLoop(void)
{
	if (getState() != CAVAN_THREAD_STATE_IDEL)
	{
		setState(CAVAN_THREAD_STATE_STOPPED);
		return 0;
	}

	setState(CAVAN_THREAD_STATE_RUNNING);

	while (1)
	{
		switch (getState())
		{
		case CAVAN_THREAD_STATE_RUNNING:
			while (getState() == CAVAN_THREAD_STATE_RUNNING)
			{
				int ret = handler();
				if (ret < 0)
				{
					setState(CAVAN_THREAD_STATE_STOPPED);
					return ret;
				}
			}
			break;

		case CAVAN_THREAD_STATE_SUSPEND:
			if (isPending())
			{
				setPending(false);
				break;
			}

			while (getState() == CAVAN_THREAD_STATE_SUSPEND)
			{
				wait();
			}
			break;

		case CAVAN_THREAD_STATE_STOPPPING:
			setState(CAVAN_THREAD_STATE_STOPPED);
		case CAVAN_THREAD_STATE_STOPPED:
			return 0;

		default:
			pr_red_info("invalid state = %d", mState);
		}
	}

	setState(CAVAN_THREAD_STATE_STOPPED);

	return -1;
}

int Thread::start(void)
{
	setState(CAVAN_THREAD_STATE_IDEL);

	int ret = pthread_create(&mThread, NULL, ThreadMain, this);
	if (ret < 0)
	{
		pr_error_info("pthread_create");
		setState(CAVAN_THREAD_STATE_NONE);
	}

	return ret;
}

int Thread::stop(void)
{
	switch (getState())
	{
	case CAVAN_THREAD_STATE_RUNNING:
	case CAVAN_THREAD_STATE_SUSPEND:
	case CAVAN_THREAD_STATE_IDEL:
		setState(CAVAN_THREAD_STATE_STOPPPING);
	case CAVAN_THREAD_STATE_STOPPPING:
		for (int i = 10; getState() != CAVAN_THREAD_STATE_STOPPED; i--)
		{
			if (i > 0)
			{
				msleep(100);
			}
			else
			{
				pthread_kill(mThread, SIGUSR1);
				setState(CAVAN_THREAD_STATE_STOPPED);
				break;
			}
		}
		break;

	default:
		pr_green_info("Don't need stop");
	}

	return 0;
}

int Thread::suspend(void)
{
	return 0;
}

int Thread::resume(void)
{
	return 0;
}
