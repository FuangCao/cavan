/*
 * File:		test_lz77.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-15 14:38:33
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
#include <cavan/codec.h>

int main(int argc, char *argv[])
{
	int ret;

	assert(argc > 3);

	if (strcmp(argv[1], "encode") == 0) {
		ret = cavan_lz77_encode2(argv[2], argv[3]);
		if (ret < 0) {
			return ret;
		}
	} else if (strcmp(argv[1], "decode") == 0) {
		ret = cavan_lz77_decode(argv[2], argv[3]);
		if (ret < 0) {
			return ret;
		}
	} else {
		pr_red_info("invalid command %s", argv[1]);
	}

	return 0;
}
