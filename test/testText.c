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
#if 0
	int i;
	int ret;
	char *texts[10];

	assert(argc > 1);

	ret = text_split_by_space(argv[1], texts, NELEM(texts));
	for (i = 0; i < ret; i++)
	{
		println("text[%d] = %s", i, texts[i]);
	}
#else
	int i;
	char buff[1024] = "----\n";

	println("buff = %p", buff);

	for (i = 5; i < 288 + 5; i++)
	{
		buff[i] = '8';
	}

	mem_move(buff + 8, buff + 4, 288);
	for (i = 0; i < 8; i++)
	{
		buff[i] = '0';
	}
	println("buff = %s", buff);
#endif

	return 0;
}
