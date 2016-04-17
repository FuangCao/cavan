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

bool cavan_is_android(void)
{
#ifdef CONFIG_ANDROID
	return true;
#else
	static int is_android = -1;

	if (unlikely(is_android < 0)) {
		is_android = file_access_e("/system/framework/framework.jar") && file_access_e("/system/build.prop");
	}

	return is_android;
#endif
}

int android_getprop_command(const char *name, char *buff, size_t size)
{
	if (cavan_is_android()) {
		int ret;
		char *last;
		char command[64];

		snprintf(command, sizeof(command), ANDROID_CMD_GETPROP " \"%s\"", name);

		ret = cavan_popen(command, buff, size, &last);
		if (ret < 0) {
			return ret;
		}

		*last = 0;

		while (last > buff && cavan_isspace(*(last - 1))) {
			*--last = 0;
		}

		return last - buff;
	}

	return -EINVAL;
}

int android_setprop_command(const char *name, const char *value)
{
	if (cavan_is_android()) {
		return cavan_system2(ANDROID_CMD_SETPROP " \"%s\" \"%s\"", name, value);
	}

	return -EINVAL;
}

int android_getprop_format(char *buff, size_t size, const char *name, ...)
{
	va_list ap;
	char name_buff[64];

	va_start(ap, name);
	vsnprintf(name_buff, sizeof(name_buff), name, ap);
	va_end(ap);

	return android_getprop(name_buff, buff, size);
}

int android_setprop_format(const char *value, const char *name, ...)
{
	va_list ap;
	char name_buff[64];

	va_start(ap, name);
	vsnprintf(name_buff, sizeof(name_buff), name, ap);
	va_end(ap);

	return android_setprop(name_buff, value);
}

int32_t android_getprop_int32(const char *name, int32_t def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2value(buff, NULL, 10);
	}

	return def_value;
}

int64_t android_getprop_int64(const char *name, int64_t def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2value(buff, NULL, 10);
	}

	return def_value;
}

uint32_t android_getprop_uint32(const char *name, uint32_t def_value)
{
	int length;
	char buff[1024];

	length = android_getprop(name, buff, sizeof(buff));
	if (length > 0) {
		return text2value_unsigned(buff, NULL, 10);
	}

	return def_value;
}

uint64_t android_getprop_uint64(const char *name, uint64_t def_value)
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

int android_get_wifi_prop(const char *name, char *buff, size_t size)
{
	char ifname[32];

	if (android_getprop("wifi.interface", ifname, sizeof(ifname)) <= 0) {
		return -ENOENT;
	}

	return android_getprop_format(buff, size, "dhcp.%s.%s", ifname, name);
}

int android_get_device_name(char *buff, size_t size)
{
	int length;

	length = android_getprop("ro.product.device", buff, size);
	if (length > 0) {
		return length;
	}

	length = android_getprop("ro.product.name", buff, size);
	if (length > 0) {
		return length;
	}

	return -EINVAL;
}

int android_get_hostname(char *buff, size_t size)
{
	int length;

	length = android_getprop("net.hostname", buff, size);
	if (length > 0) {
		return length;
	}

	return android_get_device_name(buff, size);
}
