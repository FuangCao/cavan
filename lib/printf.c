/*
 * File:		printf.c
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
#include <cavan/printf.h>

int cavan_vsnprintf(char *buff, size_t size, const char *fmt, cavan_va_list ap)
{
	return 0;
}

int cavan_snprintf(char *buff, size_t size, const char *fmt, ...)
{
	int length;
	cavan_va_list ap;

	cavan_va_start(ap, fmt);
	length = cavan_vsnprintf(buff, size, fmt, ap);
	cavan_va_end(ap);

	return length;
}

int cavan_fdprintf(int fd, const char *fmt, ...)
{
	int length;
	char buff[1024];
	cavan_va_list ap;

	cavan_va_start(ap, fmt);
	length = cavan_vsnprintf(buff, sizeof(buff), fmt, ap);
	cavan_va_end(ap);

	return ffile_write(fd, buff, length);
}
