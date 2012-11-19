/*
 * File:		gsensor.h
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

#include <cavan/input.h>
#include <cavan.h>

struct cavan_gsensor_device
{
	struct cavan_input_device input_dev;
	struct cavan_gsensor_event event;
	int xmin, xmax;
	int ymin, ymax;
	int zmin, zmax;
};

bool cavan_gsensor_device_match(uint8_t *abs_bitmask);
bool cavan_gsensor_device_matcher(struct cavan_event_matcher *matcher, void *data);
struct cavan_input_device *cavan_gsensor_create(void);
