/*
 * File:		heart_rate.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-02-14 16:46:38
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
#include <cavan/heart_rate.h>

static void heart_rate_decode_handler(struct heart_rate_decode *decode, int rate)
{
	pr_green_info("rate = %d", rate);
}

int main(int argc, char *argv[])
{
	int fd;
	const char *filename;
	struct heart_rate_decode decode;

	if (argc < 2)
	{
		println("Usage: %s <filename>", argv[0]);
		return -EINVAL;
	}

	filename = argv[1];

	decode.handler = heart_rate_decode_handler;
	heart_rate_decode_init(&decode, NULL);

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file %s", filename);
		return fd;
	}

	while (1)
	{
		u16 value;
		ssize_t rdlen;

		rdlen = read(fd, &value, sizeof(value));
		if (rdlen < (int) sizeof(value))
		{
			break;
		}

		heart_rate_decode_post(&decode, value);
	}

	close(fd);

	return 0;
}
