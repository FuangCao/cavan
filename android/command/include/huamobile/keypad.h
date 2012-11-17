/*
 * File:		keypad.h
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

struct huamobile_keypad_device
{
	struct huamobile_event_device dev;
};

bool huamobile_keypad_device_match(const uint8_t *key_bitmask, size_t size);
bool huamobile_keypad_device_matcher(int fd, const char *name, void *data);
struct huamobile_event_device *huamobile_keypad_create(void *data);
