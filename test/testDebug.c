/*
 * File:		testDebug.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-04-10 09:39:36
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

static void function1(void)
{
	dump_stack();
}

static void function2(void)
{
	function1();
}

int main(int argc, char *argv[])
{
	char buff[1024];

	function2();
	if (address_to_symbol(printf, buff, sizeof(buff)))
	{
		println("symbol = %s", buff);
	}

	catch_sigsegv();
	*(int *) 0 = 100;

	return 0;
}
