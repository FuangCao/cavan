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

#include <huamobile/input.h>
#include <huamobile.h>

#define HUA_TOUCH_POINT_MAX	5

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_SLOT			0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */
#endif

struct huamobile_touch_device
{
	struct huamobile_input_device input_dev;

	int xaxis, yaxis;

	double xscale, yscale;
	double xoffset, yoffset;
};

struct huamobile_multi_touch_device
{
	struct huamobile_touch_device touch_dev;
	struct huamobile_touch_point points[HUA_TOUCH_POINT_MAX];
	int point_count;
	int point_count_old;
};

struct huamobile_single_touch_device
{
	struct huamobile_touch_device touch_dev;
	int pressed;
	struct huamobile_touch_point point;
};

bool huamobile_multi_touch_device_match(uint8_t *abs_bitmask);
bool huamobile_multi_touch_device_matcher(int fd, const char *name, void *data);
bool huamobile_single_touch_device_match(uint8_t *abs_bitmask, uint8_t *key_bitmask);
bool huamobile_single_touch_device_matcher(int fd, const char *name, void *data);
bool huamobile_touch_device_matcher(int fd, const char *name, void *data);

struct huamobile_input_device *huamobile_multi_touch_device_create(void);
struct huamobile_input_device *huamobile_single_touch_device_create(void);
