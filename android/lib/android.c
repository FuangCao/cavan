/*
 * File:		Android.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-11 11:55:09
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/android.h>

int android_getprop(const char *name, char *buff, size_t size)
{
	int length;
    char value[PROPERTY_VALUE_MAX];

	length = property_get(name, value, NULL);
	if (length < 0) {
		return length;
	}

	strncpy(buff, value, size);

	return length;
}

int android_setprop(const char *name, const char *value)
{
	return property_set(name, value);
}
