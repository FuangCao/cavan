/*
 * File:		tee.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-08-31 11:13:36
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
#include <cavan/command.h>

#define FILE_CREATE_DATE "2015-08-31 11:13:36"

static void show_usage(const char *command)
{
	println("Usage: %s [OPTION] [FILE|COMMAND]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("-a, --append\t\tappend to the given FILEs, do not overwrite");
	println("-c, --command\t\twrite to standard input of a command");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			.name = "append",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_APPEND,
		},
		{
			.name = "command",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_COMMAND,
		},
		{
			0, 0, 0, 0
		},
	};
	bool append = false;
	bool command = false;

	while ((c = getopt_long(argc, argv, "vVhHac", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case CAVAN_COMMAND_OPTION_APPEND:
			append = true;
			break;

		case 'c':
		case CAVAN_COMMAND_OPTION_COMMAND:
			command = true;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	return cavan_tee_main(optind < argc ? argv[optind] : NULL, append, command);
}
