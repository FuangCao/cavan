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
#include <cavan/speed_detector.h>

static int speed_detector_handler(struct cavan_thread *thread, void *data)
{
	struct speed_detector *detector = data;

	detector->count = 0;

	if (cavan_thread_msleep(thread, detector->interval) != ETIMEDOUT)
	{
		return 0;
	}

	detector->speed = detector->count;

	if (detector->times_consume > 0)
	{
		detector->speed_avg = (detector->speed_avg + detector->speed) >> 1;
	}
	else
	{
		detector->speed_avg = detector->speed;
	}

	detector->times_consume += detector->interval;

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
	detector->times_consume = 0;
	detector->speed = detector->speed_avg = 0;

	thread->name = "SPEED_DETECTOR";
	thread->handler = speed_detector_handler;
	thread->wake_handker = NULL;

	return cavan_thread_run(thread, detector);
}
