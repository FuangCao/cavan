/*
 * File:		touchscreen.h
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-17
 * Description:	HUAMOBILE LIBRARY
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

#pragma once

#include <huamobile/event.h>
#include <huamobile.h>

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#endif

struct huamobile_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
	int released;
};

struct huamobile_multi_touch_device
{
	struct huamobile_event_device dev;

	double xscale, yscale;
	int xoffset, yoffset;

	struct huamobile_touch_point points[10];
	int point_count;
	int point_count_old;
};

struct huamobile_single_touch_device
{
	struct huamobile_event_device dev;

	double xscale, yscale;
	int xoffset, yoffset;

	int pressed;
	struct huamobile_touch_point point;
};

bool huamobile_multi_touch_device_matcher(int fd, const char *name, void *data);
bool huamobile_single_touch_device_matcher(int fd, const char *name, void *data);
bool huamobile_touch_device_matcher(int fd, const char *name, void *data);

struct huamobile_event_device *huamobile_multi_touch_device_create(void *data);
struct huamobile_event_device *huamobile_single_touch_device_create(void *data);

static inline bool huamobile_multi_touch_device_match(const uint8_t *abs_bitmask)
{
	return test_bit(ABS_MT_POSITION_X, abs_bitmask) && test_bit(ABS_MT_POSITION_Y, abs_bitmask);
}

static inline bool huamobile_single_touch_device_match(const uint8_t *abs_bitmask, const uint8_t *key_bitmask)
{
	return test_bit(BTN_TOUCH, key_bitmask) && test_bit(ABS_X, abs_bitmask) && test_bit(ABS_Y, abs_bitmask) && test_bit(ABS_Z, abs_bitmask) == 0;
}
