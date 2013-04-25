/*
 * File:		mkdir.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-04-25 19:01:40
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

#define FILE_CREATE_DATE "2013-04-25 19:01:40"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_MODE,
	LOCAL_COMMAND_OPTION_PARENTS,
	LOCAL_COMMAND_OPTION_VERBOSE,
	LOCAL_COMMAND_OPTION_CONTEXT
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] pathname", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -V\t\tshow version");
	println("-m, --mode=MODE\t\tset file mode (as in chmod), not a=rwx - umask");
	println("-p, --parents\t\tno error if existing, make parent directories as needed");
	println("-v, --verbose\t\tprint a message for each created directory");
	println("-Z, --context=CTX\tset the SELinux security context of each created directory to CTX");
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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
		{
			.name = "parents",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PARENTS,
		},
		{
			.name = "mode",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
		{
			.name = "context",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_CONTEXT,
		},
		{
			0, 0, 0, 0
		},
	};
	struct cavan_mkdir_command_option option =
	{
		.mode = 0777,
		.verbose = false,
		.parents = false
	};

	while ((c = getopt_long(argc, argv, "vVhHm:pZ:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'v':
		case LOCAL_COMMAND_OPTION_VERBOSE:
			option.verbose = true;
			break;

		case 'p':
		case LOCAL_COMMAND_OPTION_PARENTS:
			option.parents = true;
			break;

		case 'm':
		case LOCAL_COMMAND_OPTION_MODE:
			option.mode = file_mode2value(optarg);
			break;

		case 'Z':
		case LOCAL_COMMAND_OPTION_CONTEXT:
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind >= argc)
	{
		pr_red_info("Please give least one pathname");
		return -EINVAL;
	}

	while (optind < argc)
	{
		int ret = cavan_mkdir_main(argv[optind], &option);
		if (ret < 0)
		{
			return ret;
		}

		optind++;
	}

	return 0;
}
