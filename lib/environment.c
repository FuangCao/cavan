/*
 * File:			environment.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-04-22 09:49:45
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/environment.h>

int cavan_getenv_int(const char *name, int *value)
{
	char *p;

	p = getenv(name);
	if (p == NULL) {
		pr_error_info("getenv(%s)", name);
		return -EINVAL;
	}

	*value = text2value(p, NULL, 10);

	return 0;
}
