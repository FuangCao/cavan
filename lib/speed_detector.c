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
	u32 count;
	struct speed_detector *detector = data;

	for (count = detector->interval; count; count--)
	{
		if (thread->state != CAVAN_THREAD_STATE_RUNNING)
		{
			return 0;
		}

		msleep(1);
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

	thread->name = "SPEED_DETECTOR";
	thread->handler = speed_detector_handler;
	thread->wake_handker = NULL;

	return cavan_thread_run(thread, detector);
}

s64 speed_detector_get_time_consume_ms(struct speed_detector *detector)
{
	struct timespec time_now;

	clock_gettime_real(&time_now);

	return cavan_timespec_sub_ms(&time_now, &detector->time_start);
}

s64 speed_detector_get_time_consume_us(struct speed_detector *detector)
{
	struct timespec time_now;

	clock_gettime_real(&time_now);

	return cavan_timespec_sub_us(&time_now, &detector->time_start);
}

s64 speed_detector_get_time_consume_ns(struct speed_detector *detector)
{
	struct timespec time_now;

	clock_gettime_real(&time_now);

	return cavan_timespec_sub_ns(&time_now, &detector->time_start);
}
