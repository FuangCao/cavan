/*
 * File:		testFile.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-09-01 00:28:55
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
#include <cavan/file.h>

int main(int argc, char *argv[])
{
	int fd;
	char buff[1024];

	assert(argc > 1);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file %s", argv[1]);
		return fd;
	}

	while (1)
	{
		int ret = file_read_line(fd, buff, sizeof(buff));
		if (ret <= 0)
		{
			break;
		}

		println("line = \"%s\"", buff);
	}

	close(fd);

	return 0;
}
