/*
 * File:		android.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-10-14 16:26:11
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
#include <cavan/command.h>
#include <cavan/android.h>
#include <cavan/calculator.h>

#ifndef CONFIG_ANDROID
int android_getprop(const char *name, char *buff, size_t size)
{
	char *env;

	env = getenv(name);
	if (env == NULL) {
		return -EFAULT;
	}

	return text_ncopy(buff, env, size) - buff;
}

int android_setprop(const char *name, const char *value)
{
	return setenv(name, value, 1);
}

#endif

int android_getprop_int(const char *name, int def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2value_unsigned(buff, NULL, 10);
	}

	return def_value;
}

bool android_getprop_bool(const char *name, bool def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2bool(buff);
	}

	return def_value;
}

double android_getprop_double(const char *name, double def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2double(buff, NULL, NULL, 10);
	}

	return def_value;
}

int android_setprop_int(const char *name, int value)
{
	char buff[1024];

	value2text_simple(value, buff, sizeof(buff), 10);

	return android_setprop(name, buff);
}

int android_setprop_bool(const char *name, bool value)
{
	return android_setprop(name, value ? "1" : "0");
}

int android_setprop_double(const char *name, double value)
{
	char buff[1024];

	double2text(&value, buff, sizeof(buff), 0, 0);

	return android_setprop(name, buff);
}

void android_stop_all(void)
{
#ifdef CONFIG_ANDROID
	int i;

	print("stop android service ");

	for (i = 0; i < 10; i++) {
		cavan_system("stop", 0, NULL);
		print_char('.');
		msleep(200);
	}

	println(" OK");
#endif
}
