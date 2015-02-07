#pragma once

/*
 * File:		watchdog.h
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
#include <cavan/thread.h>

#define wd_set_pos(watchdog) \
	cavan_watchdog_flush(watchdog, __FILE__, __LINE__)

struct cavan_watchdog
{
	struct cavan_thread thread;

	u32 msec;
	int count;
	void *private_data;

	u32 line;
	const char *file;

	void (*handler)(struct cavan_watchdog *watchdog);
};

int cavan_watchdog_init(struct cavan_watchdog *watchdog, u32 msec, void *data);
void cavan_watchdog_deinit(struct cavan_watchdog *watchdog);
int cavan_watchdog_start(struct cavan_watchdog *watchdog);
int cavan_watchdog_run(struct cavan_watchdog *watchdog, u32 msec, void *data);
void cavan_watchdog_flush(struct cavan_watchdog *watchdog, const char *file, u32 line);

static inline void cavan_watchdog_set_data(struct cavan_watchdog *watchdog, void *data)
{
	watchdog->private_data = data;
}

static inline void *cavan_watchdog_get_data(struct cavan_watchdog *watchdog)
{
	return watchdog->private_data;
}

static inline void cavan_watchdog_stop(struct cavan_watchdog *watchdog)
{
	cavan_thread_stop(&watchdog->thread);
}
