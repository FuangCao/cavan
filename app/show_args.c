#define CAVAN_CMD_NAME show_args

/*
 * File:		show_args.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-09-21 16:28:12
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

int main(int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; i++) {
		println("argv[%d] = %s", i, argv[i]);
	}

	return 0;
}
