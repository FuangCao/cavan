/*
 * File:		mouse.h
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

#include <cavan/event.h>
#include <cavan/touchscreen.h>
#include <cavan.h>

struct cavan_mouse_device
{
	struct cavan_input_device input_dev;
	struct cavan_touch_point point;
	int xold, xmax;
	int yold, ymax;
};

bool cavan_mouse_device_match(uint8_t *key_bitmask, uint8_t *rel_bitmask);
bool cavan_mouse_device_matcher(struct cavan_event_matcher *matcher, void *data);
struct cavan_input_device *cavan_mouse_create(void);
