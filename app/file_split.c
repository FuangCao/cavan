/*
 * File:		file_split.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-01-16 10:45:13
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

#define FILE_CREATE_DATE "2014-01-16 10:45:13"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_COUNT,
	LOCAL_COMMAND_OPTION_SIZE,
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] input_file output_directory", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--count, -c, -C\t\tfile count");
	println("--size, -s, -S\t\tevery file size");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	size_t size = 0;
	int count = 0;
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
			.name = "count",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_COUNT,
		},
		{
			.name = "size",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_COUNT,
		},
		{
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "vVhHc:C:s:S:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
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

		case 'c':
		case 'C':
		case LOCAL_COMMAND_OPTION_COUNT:
			count = text2value_unsigned(optarg, NULL, 10);
			break;

		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SIZE:
			size = text2size(optarg, NULL);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind + 2 > argc)
	{
		pr_red_info("Please give input_file and output_directory");
		show_usage(argv[0]);
		return -EINVAL;
	}

	return file_split(argv[optind], argv[optind + 1], size, count);
}
