/*
 * File:		su.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-07-01 16:01:06
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
#include <cavan/adb.h>
#include <cavan/tcp_dd.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2014-07-01 16:01:06"

static void show_usage(const char *command)
{
	println("Usage: %s [option] [username]", command);
	println("--help, -h\t\t\tdisplay this help message and exit");
	println("--version, -v, -V\t\tshow version");
	println("-c, --command COMMAND\t\tpass COMMAND to the invoked shell");
	println("-l, --login\t\t\tmake the shell a login shell");
	println("-m, -p, --preserve-environment\tdo not reset environment variables, and keep the same shell");
	println("-s, --shell SHELL\t\tuse SHELL instead of the default in passwd");
	println("--ip, -i, -I, -H, --host HOST\tserver host address");
	println("--local, -L\t\t\tuse localhost ip");
	println("--port, -P PORT\t\t\tserver port");
	println("--adb, -a, -A\t\t\tuse adb procotol instead of tcp");
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
			.name = "command",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_COMMAND,
		},
		{
			.name = "login",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGIN,
		},
		{
			.name = "preserve-environment",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RESET,
		},
		{
			.name = "shell",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SHELL,
		},
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		},
		{
			0, 0, 0, 0
		},
	};
	struct inet_file_request file_req =
	{
		.hostname = "127.0.0.1",
		.port = TCP_DD_DEFAULT_PORT,
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket
	};

	file_req.command[0] = 0;

	while ((c = getopt_long(argc, argv, "vVhHc:lmps:i:I:P:LaA", long_option, &option_index)) != EOF)
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

		case 'c':
		case CAVAN_COMMAND_OPTION_COMMAND:
			text_copy(file_req.command, optarg);
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

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			file_req.open_connect = adb_create_tcp_link2;
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
			file_req.hostname = optarg;
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			file_req.port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	return tcp_dd_exec_command(&file_req);
}
