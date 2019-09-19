#define CAVAN_CMD_NAME swan_vn

/*
 * File:		swan_vn.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-03-14 14:30:54
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
#include <cavan/net_bridge.h>

#define FILE_CREATE_DATE "2014-03-14 14:30:54"

static void show_usage(const char *command)
{
	println("Usage: %s [option]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
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
	struct cavan_net_bridge bridge;

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

	ret = cavan_net_bridge_init(&bridge);
	if (ret < 0) {
		pr_red_info("cavan_net_bridge_init");
		return ret;
	}

	while (optind < argc) {
		cavan_net_bridge_register_port(&bridge, argv[optind]);
		optind++;
	}

	cavan_thread_join(&bridge.thread);
	cavan_net_bridge_deinit(&bridge);

	return 0;
}
