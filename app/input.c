#define CAVAN_CMD_NAME input

/*
 * File:		input.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-11 00:03:56
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
#include <cavan/input.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2016-01-11 00:03:56"

static void show_usage(const char *command)
{
	println("Usage: %s [option]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		}, {
			0, 0, 0, 0
		},
	};
	int fd;
	int type, code, value;
	const char *pathname;

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF) {
		switch (c) {
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

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(argc > optind + 3);

	pathname = argv[optind++];

	if (argc > 4) {
		type = cavan_input_type2value(argv[optind++]);
	} else {
		type = EV_KEY;
	}

	code = text2value_unsigned(argv[optind++], NULL, 10);
	value = text2value_unsigned(argv[optind++], NULL, 10);

	println("pathname = %s", pathname);
	println("type = %d, code = %d, value = %d", type, code, value);

	fd = open(pathname, O_WRONLY);
	if (fd < 0) {
		pr_err_info("open `%s': %d", pathname, fd);
		return fd;
	}

	cavan_input_event2(fd, type, code, value);
	cavan_input_sync(fd);

	return 0;
}
