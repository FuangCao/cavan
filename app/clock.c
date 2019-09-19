#define CAVAN_CMD_NAME clock

/*
 * File:		clock.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-31 16:07:46
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/timer.h>

int main(int argc, char *argv[])
{
	while (1) {
		struct timespec ts;
		struct tm tm;
		u16 mseconds;

		clock_gettime_real(&ts);
		localtime_r(&ts.tv_sec, &tm);
		mseconds = ts.tv_nsec / 1000000ul;

		print("%02d:%02d:%02d.%03d\r", tm.tm_hour, tm.tm_min, tm.tm_sec, mseconds);

		nsleep(1000000000ul - ts.tv_nsec);
	}

	return 0;
}
