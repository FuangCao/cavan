/*
 * File:		watchdog.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-02-08 00:21:47
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/watchdog.h>

static int cavan_watchdog_thread_handler(struct cavan_thread *thread, void *data)
{
	int ret;
	struct cavan_watchdog *watchdog = data;

	watchdog->count = 0;

	ret = cavan_thread_msleep(thread, watchdog->msec);

	if (ret == ETIMEDOUT && watchdog->count == 0)
	{
		watchdog->handler(watchdog);
	}

	return 0;
}

int cavan_watchdog_init(struct cavan_watchdog *watchdog, u32 msec, void *data)
{
	int ret;
	struct cavan_thread *thread = &watchdog->thread;

	thread->name = "WATCHDOG";
	thread->wake_handker = NULL;
	thread->handler = cavan_watchdog_thread_handler;

	ret = cavan_thread_init(thread, watchdog);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		return ret;
	}

	watchdog->msec = msec;
	watchdog->private_data = data;
	watchdog->file = __FILE__;
	watchdog->line = __LINE__;

	return 0;
}

void cavan_watchdog_deinit(struct cavan_watchdog *watchdog)
{
	cavan_thread_deinit(&watchdog->thread);
}

static void cavan_watchdog_handler_dummy(struct cavan_watchdog *watchdog)
{
	println("watchdog trigger: %s +%d", watchdog->file, watchdog->line);
}

int cavan_watchdog_start(struct cavan_watchdog *watchdog)
{
	if (watchdog->handler == NULL)
	{
		watchdog->handler = cavan_watchdog_handler_dummy;
	}

	return cavan_thread_start(&watchdog->thread);
}

int cavan_watchdog_run(struct cavan_watchdog *watchdog, u32 msec, void *data)
{
	int ret;

	ret = cavan_watchdog_init(watchdog, msec, data);
	if (ret < 0)
	{
		pr_red_info("cavan_watchdog_init");
		return ret;
	}

	return cavan_watchdog_start(watchdog);
}

void cavan_watchdog_flush(struct cavan_watchdog *watchdog, const char *file, u32 line)
{
	watchdog->file = file;
	watchdog->line = line;

	watchdog->count++;
}
