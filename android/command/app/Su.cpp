/*
 * File:		su_server.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-18 11:55:09
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

#include <binder/ProcessState.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include "SuService.h"

using namespace android;

static void show_usage(const char *command)
{
	println("Usage: %s [option] [username]", command);
	println("--help, -h\t\t\t\t%s", cavan_help_message_help);
	println("--version, -V, -v\t\t\t%s", cavan_help_message_version);
	println("-c, --command COMMAND\t\t\t%s", cavan_help_message_command);
	println("-l, --login\t\t\t\t%s", cavan_help_message_login);
	println("-m, -p, --preserve-environment\t\t%s", cavan_help_message_preserve_environment);
	println("-s, --shell SHELL\t\t\t%s", cavan_help_message_shell);
	println("-d, --daemon, --service\t\t\t%s", cavan_help_message_daemon);
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
			.name = "command",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_COMMAND,
		}, {
			.name = "login",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGIN,
		}, {
			.name = "preserve-environment",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RESET,
		}, {
			.name = "shell",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SHELL,
		}, {
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			.name = "service",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			0, 0, 0, 0
		},
	};
	bool daemon = false;
	const char *command = NULL;

	while ((c = getopt_long(argc, argv, "vVhH:c:lmps:i:I:P:LaAd", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			return 0;

		case 'h':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'c':
		case CAVAN_COMMAND_OPTION_COMMAND:
			command = optarg;
			break;

		case 'l':
		case CAVAN_COMMAND_OPTION_LOGIN:
			pr_pos_info();
			break;

		case 'm':
		case 'p':
		case CAVAN_COMMAND_OPTION_RESET:
			pr_pos_info();
			break;

		case 's':
		case CAVAN_COMMAND_OPTION_SHELL:
			pr_pos_info();
			break;

		case 'd':
		case CAVAN_COMMAND_OPTION_DAEMON:
			daemon = true;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (daemon) {
		ProcessState::self()->setThreadPoolMaxThreadCount(0);
		SuService::publishAndJoinThreadPool();

		return 0;
	}

	sp<ISuService> su = ISuService::getService();
	if (su == NULL) {
		fprintf(stderr, "Failed to SuService::getService()\n");
		return -EFAULT;
	}

	int ret = su->popen(command, 0);
	if (ret < 0) {
		pr_red_info("su->popen: %d\n", ret);
		return ret;
	}

	return su->redirectSlaveStdio();
}
