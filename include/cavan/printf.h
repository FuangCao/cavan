#pragma once

/*
 * File:		printf.h
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

int cavan_vsnprintf(char *buff, size_t size, const char *fmt, cavan_va_list ap);
int cavan_snprintf(char *buff, size_t size, const char *fmt, ...);
int cavan_fdprintf(int fd, const char *fmt, ...);
