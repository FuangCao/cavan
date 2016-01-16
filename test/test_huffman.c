/*
 * File:		test_huffman.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-16 17:01:32
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
	const char *command;

	assert(argc > 3);

	command = argv[1];

	if (strcmp(command, "encode") == 0) {
		ret = cavan_huffman_encode_file(argv[2], argv[3]);
		if (ret < 0) {
			pr_red_info("cavan_huffman_encode_file");
			return ret;
		}
	} else if (strcmp(command, "decode") == 0) {
		ret = cavan_huffman_decode_file(argv[2], argv[3]);
		if (ret < 0) {
			pr_red_info("cavan_huffman_encode_file");
			return ret;
		}
	} else {
		pr_red_info("invalid command %s", command);
		return -EINVAL;
	}

	return 0;
}
