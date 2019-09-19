#define CAVAN_CMD_NAME tcp_bridge

/*
 * File:		tcp_bridge.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-21 17:48:56
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
#include <cavan/tcp_bridge.h>

static void show_usage(const char *command)
{
	println("Usage: %s [OPTION] <URL1> <URL2>", command);
	println("--daemon, -d\t%s", cavan_help_message_daemon);
	println("--loop, -l\t%s", cavan_help_message_loop);
}

int main(int argc, char *argv[])
{
	int c;
	const char *url1;
	const char *url2;
	int option_index;
	bool as_loop = false;
	bool as_daemon = false;
	static const struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			.name = "loop",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOOP,
		}, {
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "hHdl", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return -EFAULT;

		case 'd':
		case CAVAN_COMMAND_OPTION_DAEMON:
			as_daemon = true;
			break;

		case 'l':
		case CAVAN_COMMAND_OPTION_LOOP:
			as_loop = true;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind + 1 < argc) {
		url1 = argv[optind];
		url2 = argv[optind + 1];
	} else {
		show_usage(argv[0]);
		return -EINVAL;
	}

	if (as_daemon) {
		int ret = daemon(0, 0);
		if (ret < 0) {
			pr_err_info("daemon");
			return ret;
		}
	}

	while (1) {
		cavan_tcp_bridge_run(url1, url2);

		if (as_loop) {
			msleep(2000);
		} else {
			break;
		}
	}

	return 0;
}
