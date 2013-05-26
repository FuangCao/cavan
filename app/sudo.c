/*
 * File:		sudo.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-05-26 16:06:02
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

int main(int argc, char *argv[])
{
	int ret;
	const char *shell_command = "sh";

	ret = setuid(0);
	if (ret < 0)
	{
		pr_error_info("Set user to super failed");
		return ret;
	}

	ret = setgid(0);
	if (ret < 0)
	{
		pr_error_info("Set group to super failed");
		return ret;
	}

	if (argc > 1)
	{
		char command[1024];

		text_join_by_char(argv + 1, argc - 1, ' ', command, sizeof(command));
		ret = execlp(shell_command, shell_command, "-c", command, NULL);
	}
	else
	{
		ret = execlp(shell_command, shell_command, "-", NULL);
	}

	return ret;
}
