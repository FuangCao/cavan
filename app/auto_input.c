/*
 * File:		auto_input.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-08-22 16:23:15
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
#include <cavan/command.h>

int main(int argc, char *argv[])
{
	int i;
	int fd;
	int ret;
	pid_t pid;

	if (argc < 2)
	{
		println("Usage: %s [command] <options ...>", argv[0]);
		return -EINVAL;
	}

	fd = cavan_exec_redirect_stdio_popen(argv[1], 0xFFFF, 0xFFFFF, &pid, 0x01);
	if (fd < 0)
	{
		pr_red_info("cavan_exec_redirect_stdio_popen");
		return fd;
	}

	for (i = 2; i < argc; i++)
	{
		ret = write(fd, argv[i], strlen(argv[i]));
		if (ret < 0)
		{
			goto out_close_fd;
		}

		ret = write(fd, "\n", 1);
		if (ret < 0)
		{
			goto out_close_fd;
		}
	}

	ret = cavan_exec_waitpid(pid);

out_close_fd:
	close(fd);
	return ret;
}
