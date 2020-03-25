/*
 * File:		serial.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2020-03-25 12:30:44
 *
 * Copyright (c) 2020 Fuang.Cao <cavan.cfa@gmail.com>
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

#define FILE_CREATE_DATE "2020-03-25 12:30:44"

static void show_usage(const char *command)
{
	println("Usage: %s [option]", command);
	println("-h, -H, --help\t\tshow this help");
	println("-v, -V, --version\tshow version");
}

int main(int argc, char *argv[])
{
	int c;
	int fd;
	int rate;
	int option_index;
	const char *pathname;
	const char *line_end;
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
			.name = "rate",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RATE,
		}, {
			.name = "line-end",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LINE_END,
		}, {
			0, 0, 0, 0
		},
	};

	rate = 115200;
	line_end = "\n";
	pathname = "/dev/ttyUSB0";

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

		case CAVAN_COMMAND_OPTION_RATE:
			rate = atoi(optarg);
			break;

		case CAVAN_COMMAND_OPTION_LINE_END:
			line_end = optarg;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc) {
		pathname = argv[optind++];
	}

	fd = serial_open(pathname, rate);
	if (fd < 0) {
		pr_err_info("serial_open: %d", fd);
		return -EFAULT;
	}

	serial_cmdline(fd, line_end);
	close(fd);

	return 0;
}
