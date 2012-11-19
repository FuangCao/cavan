/*
 * File:		cavan.c
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-14
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

int cavan_msleep(useconds_t ms)
{
	int ret;

	while (ms--)
	{
		ret = usleep(1000);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int cavan_ssleep(useconds_t ss)
{
	int ret;

	while (ss--)
	{
		ret = usleep(1000 * 1000);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}
