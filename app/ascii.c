/*
 * File:		ascii.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-18 11:34:42
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
	const char *text;
	char c;

	assert(argc > 1);

	for (text = argv[1]; (c = *text); text++) {
		println("'%c' = 0x%02x = 0%o = %d", c, c, c, c);
	}

	return 0;
}
