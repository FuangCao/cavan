#pragma once

/*
 * File:			printf.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-06-04 19:05:27
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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

typedef __builtin_va_list cavan_va_list;

#define cavan_va_start(ap, args)			__builtin_va_start(ap, args)
#define cavan_va_end(ap)					__builtin_va_end(ap)
#define cavan_va_arg(ap, type)				__builtin_va_arg(ap, type)
#define cavan_printf(fmt, args ...)			cavan_fdprintf(stdout_fd, fmt, ##args)

#define CAVAN_PRINTF_TEN_LOWER_CASE			'a'
#define CAVAN_PRINTF_TEN_UPPER_CASE			'A'
#define CAVAN_PRINTF_PREFIX_ENABLE(flags)	((flags) & (CAVAN_PRINTF_PREFIX | CAVAN_PRINTF_PREFIX_FORCE))

enum {
	CAVAN_PRINTF_SIGN = 1 << 0,
	CAVAN_PRINTF_PLUS = 1 << 1,
	CAVAN_PRINTF_LEFT = 1 << 2,
	CAVAN_PRINTF_PREFIX = 1 << 3,
	CAVAN_PRINTF_PREFIX_FORCE = 1 << 4,
	CAVAN_PRINTF_REVERSE = 1 << 5,
	CAVAN_PRINTF_NEGATIVE = 1 << 6,
};

enum {
	CAVAN_PRINTF_TYPE_INVALID = -1,
	CAVAN_PRINTF_TYPE_NONE,
	CAVAN_PRINTF_TYPE_WIDTH,
	CAVAN_PRINTF_TYPE_PRECISION,
	CAVAN_PRINTF_TYPE_CHAR,
	CAVAN_PRINTF_TYPE_STR,
	CAVAN_PRINTF_TYPE_PTR,			// 5
	CAVAN_PRINTF_TYPE_PERCENT,
	CAVAN_PRINTF_TYPE_ULLONG,
	CAVAN_PRINTF_TYPE_LLONG,
	CAVAN_PRINTF_TYPE_ULONG,
	CAVAN_PRINTF_TYPE_LONG,			// 10
	CAVAN_PRINTF_TYPE_UBYTE,
	CAVAN_PRINTF_TYPE_BYTE,
	CAVAN_PRINTF_TYPE_USHORT,
	CAVAN_PRINTF_TYPE_SHORT,
	CAVAN_PRINTF_TYPE_UINT,			// 15
	CAVAN_PRINTF_TYPE_INT,
	CAVAN_PRINTF_TYPE_STR_LEN,
	CAVAN_PRINTF_TYPE_SIZE,
	CAVAN_PRINTF_TYPE_SSIZE,
	CAVAN_PRINTF_TYPE_MEMORY,
};

struct cavan_printf_spec {
	int type;
	int base;
	int width;
	int flags;
	int qualifier;
	int precision;
	char ten;
	char fill;
	char buff_prefix[4];
	const char *prefix;
	char *buff;
	char *buff_end;
};

__printf_format_30__ int cavan_vsnprintf(char *buff, size_t size, const char *fmt, cavan_va_list ap);
__printf_format_34__ int cavan_snprintf(char *buff, size_t size, const char *fmt, ...);
__printf_format_23__ int cavan_fdprintf(int fd, const char *fmt, ...);
