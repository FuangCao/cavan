/*
 * File:		gsensor.h
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

struct huamobile_gsensor_event
{
	int x, y, z;
};

struct huamobile_gsensor_device
{
	struct huamobile_event_device dev;
	int xmin, xmax;
	int ymin, ymax;
	int zmin, zmax;
	struct huamobile_gsensor_event event;
};

bool huamobile_gsensor_device_matcher(int fd, const char *name, void *data);
struct huamobile_event_device *huamobile_gsensor_create(void *data);

static inline bool huamobile_gsensor_device_match(const uint8_t *abs_bitmask)
{
	return test_bit(ABS_X, abs_bitmask) && test_bit(ABS_Y, abs_bitmask) && test_bit(ABS_Z, abs_bitmask);
}
