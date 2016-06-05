/*
 * File:		test_printf.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-06-04 19:08:33
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

int main(int argc, char *argv[])
{
	int a = 12345678;
	int b = 0x1122aabb;
	int c = 'z';
	int d = -100;
	const char *text = "aabbccddeeff";

	println("text = %p", text);

	cavan_printf("a = %d = %0*d\n", a, 10, a);
	cavan_printf("b = %d = %#08x = %#08X, %011o\n", b, b, b, b);
	cavan_printf("c = %c = %8c = %*c\n", c, c, 10, c);
	cavan_printf("d = %d@2 = %08d\n", d, d);
	cavan_printf("text = %p, %s\n", text, text);

	return 0;
}
