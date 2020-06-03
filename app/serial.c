#define CAVAN_CMD_NAME serial

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
	int option_index;
	int rate = 115200;
	const char *pathname = NULL;
	struct cavan_serial_desc serial;
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
			.name = "crlf",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CRLF,
		}, {
			.name = "cr",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CR,
		}, {
			.name = "at",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CR,
		}, {
			.name = "lf",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LF,
		}, {
			.name = "rk",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RK,
		}, {
			0, 0, 0, 0
		},
	};

	cavan_serial_init(&serial);

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
			rate = text2clock(optarg);
			break;

		case CAVAN_COMMAND_OPTION_LINE_END:
			serial.line_end = optarg;
			break;

		case CAVAN_COMMAND_OPTION_CRLF:
			serial.line_end = "\r\n";
			break;

		case CAVAN_COMMAND_OPTION_CR:
		case CAVAN_COMMAND_OPTION_AT:
			serial.line_end = "\r";
			break;

		case CAVAN_COMMAND_OPTION_LF:
			serial.line_end = "\n";
			break;

		case CAVAN_COMMAND_OPTION_RK:
			rate = 1500000;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc) {
		pathname = argv[optind++];
	}

	fd = serial_open(pathname, rate, serial.line_end);
	if (fd < 0) {
		pr_err_info("serial_open: %d", fd);
		return -EFAULT;
	}

	serial.fd = fd;
	cavan_serial_cmdline(&serial);

	close(fd);
	cavan_serial_deinit(&serial);

	return 0;
}
