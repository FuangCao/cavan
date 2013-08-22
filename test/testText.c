/*
 * File:		testText.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-08-16 16:29:20
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	println("%c", text_get_char("123456", 1));

	return 0;
}
