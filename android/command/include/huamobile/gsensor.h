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

#include <huamobile/input.h>
#include <huamobile.h>

struct huamobile_gsensor_device
{
	struct huamobile_input_device input_dev;
	struct huamobile_gsensor_event event;
	int xmin, xmax;
	int ymin, ymax;
	int zmin, zmax;
};

bool huamobile_gsensor_device_match(uint8_t *abs_bitmask);
bool huamobile_gsensor_device_matcher(int fd, const char *name, void *data);
struct huamobile_input_device *huamobile_gsensor_create();
