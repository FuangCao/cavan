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
#endif

int android_getprop_int(const char *name, int def_value)
{
	int ret;
	char buff[1024];

	ret = android_getprop(name, buff, sizeof(buff));
	if (ret < 0) {
		return def_value;
	}

	return text2value_unsigned(buff, NULL, 10);
}

bool android_getprop_bool(const char *name, bool def_value)
{
	int ret;
	char buff[1024];

	ret = android_getprop(name, buff, sizeof(buff));
	if (ret < 0) {
		return def_value;
	}

	return text2bool(buff);
}

double android_getprop_double(const char *name, double def_value)
{
	int ret;
	char buff[1024];

	ret = android_getprop(name, buff, sizeof(buff));
	if (ret < 0) {
		return def_value;
	}

	return text2double(buff, NULL, NULL, 10);
}

void android_stop_all(void)
{
#ifdef CONFIG_ANDROID
	int i;

	print("stop android service ");

	for (i = 0; i < 10; i++) {
		cavan_system("stop");
		print_char('.');
		msleep(200);
	}

	println(" OK");
#endif
}
