#pragma once

/*
 * File:		android.h
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
#include <cavan/file.h>

#define ANDROID_CMD_GETPROP		"/system/bin/getprop"
#define ANDROID_CMD_SETPROP		"/system/bin/setprop"

__BEGIN_DECLS

bool cavan_is_android(void);
int android_getprop_command(const char *name, char *buff, size_t size);
int android_setprop_command(const char *name, const char *value);

int32_t android_getprop_int32(const char *name, int32_t def_value);
int64_t android_getprop_int64(const char *name, int64_t def_value);
uint32_t android_getprop_uint32(const char *name, uint32_t def_value);
uint64_t android_getprop_uint64(const char *name, uint64_t def_value);
bool android_getprop_bool(const char *name, bool def_value);
double android_getprop_double(const char *name, double def_value);

int android_setprop_int(const char *name, int value);
int android_setprop_bool(const char *name, bool value);
int android_setprop_double(const char *name, double value);
void android_stop_all(void);

__printf_format_34__ int android_getprop_format(char *buff, size_t size, const char *name, ...);
__printf_format_23__ int android_setprop_format(const char *value, const char *name, ...);

int android_get_wifi_prop(const char *name, char *buff, size_t size);
int android_get_device_name(char *buff, size_t size);

static inline int android_get_wifi_ipaddress(char *buff, size_t size)
{
	return android_get_wifi_prop("ipaddress", buff, size);
}

static inline int android_get_wifi_gateway(char *buff, size_t size)
{
	return android_get_wifi_prop("gateway", buff, size);
}

static inline int android_get_wifi_mask(char *buff, size_t size)
{
	return android_get_wifi_prop("mask", buff, size);
}

#ifdef CONFIG_ANDROID
int android_getprop(const char *name, char *buff, size_t size);
int android_setprop(const char *name, const char *value);
#else
static inline int android_getprop(const char *name, char *buff, size_t size)
{
	return android_getprop_command(name, buff, size);
}

static inline int android_setprop(const char *name, const char *value)
{
	return android_setprop_command(name, value);
}
#endif

__END_DECLS
