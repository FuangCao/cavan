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

#define cavan_va_start(ap, args)		__builtin_va_start(ap, args)
#define cavan_va_end(ap)				__builtin_va_end(ap)
#define cavan_va_arg(ap, type)			__builtin_va_arg(ap, type)
#define cavan_printf(fmt, args ...)		cavan_fdprintf(stdout_fd, fmt, ##args)

enum {
	CAVAN_PRINTF_SIGN = 1 << 0,
	CAVAN_PRINTF_PLUS = 1 << 1,
	CAVAN_PRINTF_LEFT = 1 << 2,
	CAVAN_PRINTF_PREFIX = 1 << 3,
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
	CAVAN_PRINTF_TYPE_LONG_LONG,
	CAVAN_PRINTF_TYPE_ULONG,
	CAVAN_PRINTF_TYPE_LONG,
	CAVAN_PRINTF_TYPE_UBYTE,		// 10
	CAVAN_PRINTF_TYPE_BYTE,
	CAVAN_PRINTF_TYPE_USHORT,
	CAVAN_PRINTF_TYPE_SHORT,
	CAVAN_PRINTF_TYPE_UINT,
	CAVAN_PRINTF_TYPE_INT,			// 15
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
	char fill;
	char first_letter;
	const char *prefix;

	union {
		char letter;
		ulonglong value;
		const uchar *mem;
		const char *text;
	};
};

int cavan_vsnprintf(char *buff, size_t size, const char *fmt, cavan_va_list ap);
int cavan_snprintf(char *buff, size_t size, const char *fmt, ...);
int cavan_fdprintf(int fd, const char *fmt, ...);
