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

static void test_printf(const char *fmt, ...)
{
	int length;
	va_list ap;
	cavan_va_list cavan_ap;
	char buff[1024];

	va_start(ap, fmt);
	length = vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	// ffile_puts(stdout_fd, "std printf:\n");
	ffile_write(stdout_fd, buff, length);

	cavan_va_start(cavan_ap, fmt);
	length = cavan_vsnprintf(buff, sizeof(buff), fmt, cavan_ap);
	va_end(cavan_ap);

	// ffile_puts(stdout_fd, "cavan printf:\n");
	ffile_write(stdout_fd, buff, length);
}

int main(int argc, char *argv[])
{
	int a = 12345678;
	int b = -100;
	int x = 0x1122aabb;
	int o = 011223344;
	int c = 'z';
	long sl = -100;
	ulong ul = -100;
	sllong sll = -100;
	ullong ull = -100;
	const char *text = "aabbccddeeff";

	println("text = %p", text);

	test_printf("a = %d = %*d = %0*d = %10d = %010d\n", a, 10, a, 10, a, a, a);
	test_printf("b = %d = %08d\n", b, b);
	test_printf("x = %d = %x = %X = %#x = %#X, %8x = %08x = %#8x = %#08x\n", x, x, x, x, x, x, x, x, x);
	test_printf("o = %d = %o = %#o = %8o = %08o = %#8o = %#08o\n", o, o, o, o, o, o, o);
	test_printf("c = %c = %8c = %*c\n", c, c, 10, c);
	test_printf("text = %p = %s\n", text, text);
	test_printf("sl = %ld\n", sl);
	test_printf("sll = %lld = %Ld\n", sll, sll);
	test_printf("ul = %lu\n", ul);
	test_printf("ull = %llu = %Lu\n", ull, ull);

	return 0;
}
