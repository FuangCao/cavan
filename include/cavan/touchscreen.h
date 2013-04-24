/*
 * File:		touchscreen.h
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-17
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

#include <cavan.h>
#include <cavan/input.h>

#define CAVAN_TOUCH_POINT_MAX	5

struct cavan_touch_device
{
	struct cavan_input_device input_dev;

	int xaxis, yaxis;

	double xscale, yscale;
	double xoffset, yoffset;
};

struct cavan_multi_touch_device
{
	struct cavan_touch_device touch_dev;
	struct cavan_input_message_point points[CAVAN_TOUCH_POINT_MAX];
	int point_count;
	int point_count_old;
};

struct cavan_single_touch_device
{
	struct cavan_touch_device touch_dev;
	int pressed;
	struct cavan_input_message_point point;
};

bool cavan_multi_touch_device_match(uint8_t *abs_bitmask);
bool cavan_multi_touch_device_matcher(struct cavan_event_matcher *matcher, void *data);
bool cavan_single_touch_device_match(uint8_t *abs_bitmask, uint8_t *key_bitmask);
bool cavan_single_touch_device_matcher(struct cavan_event_matcher *matcher, void *data);
bool cavan_touch_device_matcher(struct cavan_event_matcher *matcher, void *data);

struct cavan_input_device *cavan_multi_touch_device_create(void);
struct cavan_input_device *cavan_single_touch_device_create(void);
