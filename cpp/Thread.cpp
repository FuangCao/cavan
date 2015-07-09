/*
 * File:		CavanThread.cpp
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
#include <cavan++/Thread.h>

#define CAVAN_THREAD_DEBUG 0

int CavanThread::CavanThreadHandler(struct cavan_thread *_thread, void *data)
{
	CavanThread *thread = (CavanThread *) data;

	return thread->run();
}

CavanThread::CavanThread(const char *name, cavan_thread_handler_t handler)
{
	mHandler = handler;
	mThread.name = name;
	mThread.wake_handker = NULL;
	mThread.handler = CavanThreadHandler;
	cavan_thread_init(&mThread, this);
}

CavanThread::~CavanThread(void)
{
	cavan_thread_stop(&mThread);
	cavan_thread_deinit(&mThread);
}

int CavanThread::run(void)
{
	if (mHandler)
	{
		return mHandler(this);
	}

	return 0;
}
