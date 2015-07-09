/*
 * File:		testProgressBar.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-06-17 14:49:28
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
#include <cavan/progress.h>

int main(int argc, char *argv[])
{
	int i;
	struct progress_bar bar;

	println("%ld", sizeof("123456789"));

	progress_bar_init(&bar, MB(100));

	for (i = 0; i < 100; i++)
	{
		progress_bar_add(&bar, MB(1));
		msleep(500);
		progress_bar_add(&bar, 0);
		msleep(500);
		progress_bar_add(&bar, 0);
		msleep(500);
		progress_bar_add(&bar, 0);
		msleep(500);
	}

	progress_bar_finish(&bar);

	return 0;
}
