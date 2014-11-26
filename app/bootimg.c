/*
 * File:		bootimg.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-24 13:35:23
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
#include <cavan/bootimg.h>

#define FILE_CREATE_DATE "2014-11-24 13:35:23"

static void show_usage_unpack(const char *command)
{
	println("Usage: %s [option] boot.img [output]", command);
	println("--help, -h, -H\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t%s", cavan_help_message_version);
}

static int cavan_bootimg_unpack(int argc, char *argv[])
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
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
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
			show_usage_unpack(argv[0]);
			return 0;

		default:
			show_usage_unpack(argv[0]);
			return -EINVAL;
		}
	}

	argv += optind;
	argv -= optind;

	if (argc < 2)
	{
		show_usage_unpack(argv[0]);
		return -EINVAL;
	}

	return bootimg_unpack(argv[1], argc > 2 ? argv[2] : ".");
}

static int cavan_bootimg_repack(int argc, char *argv[])
{
	pr_pos_info();

	return 0;
}

static int cavan_bootimg_pack(int argc, char *argv[])
{
	pr_pos_info();

	return 0;
}

static struct cavan_command_map cmd_map[] =
{
	{"unpack", cavan_bootimg_unpack},
	{"repack", cavan_bootimg_repack},
	{"pack", cavan_bootimg_pack}
};

FIND_EXEC_COMMAND_MAIN(cmd_map);
