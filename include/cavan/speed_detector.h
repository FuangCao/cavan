#pragma once

/*
 * File:		speed_detector.h
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
#include <cavan/thread.h>

struct speed_detector
{
	struct cavan_thread thread;
	u32 speed;
	u32 speed_count;
	u32 interval;
	u32 loop_count;

	struct timespec time_start;
	struct timespec time_next;

	void (*notify)(struct speed_detector *detector, u32 speed);
};

int speed_detector_start(struct speed_detector *detector, u32 interval);
u32 speed_detector_get_time_consume(struct speed_detector *detector);

static inline void speed_detector_stop(struct speed_detector *detector)
{
	cavan_thread_exit(&detector->thread);
}

static inline void speed_detector_post(struct speed_detector *detector, u32 value)
{
	detector->speed_count += value;
}

static inline double speed_detector_get_speed(struct speed_detector *detector, u32 unit)
{
	return ((double) detector->speed) * unit / detector->interval;
}

static inline void speed_detector_set_interval(struct speed_detector *detector, u32 interval)
{
	detector->interval = interval;
}
