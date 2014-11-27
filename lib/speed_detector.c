/*
 * File:		speed_detector.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-25 11:55:09
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/speed_detector.h>

static int speed_detector_handler(struct cavan_thread *thread, void *data)
{
	struct speed_detector *detector = data;

	cavan_timer_timespec_add(&detector->time_next, detector->interval);
	if (cavan_thread_msleep_until(thread, &detector->time_next) != ETIMEDOUT)
	{
		return 0;
	}

	detector->speed = detector->speed_count;
	detector->speed_count = 0;
	detector->loop_count++;

	if (detector->notify)
	{
		detector->notify(detector, detector->speed);
	}

	return 0;
}

int speed_detector_start(struct speed_detector *detector, u32 interval)
{
	struct cavan_thread *thread = &detector->thread;

	detector->interval = interval;
	detector->loop_count = 0;
	detector->speed = detector->speed_count = 0;

	clock_gettime_real(&detector->time_start);
	detector->time_next = detector->time_start;

	thread->name = "SPEED_DETECTOR";
	thread->handler = speed_detector_handler;
	thread->wake_handker = NULL;

	return cavan_thread_run(thread, detector);
}

u32 speed_detector_get_time_consume(struct speed_detector *detector)
{
	struct timespec time_now;

	clock_gettime_real(&time_now);

	return cavan_timespec_diff(&time_now, &detector->time_start);
}
