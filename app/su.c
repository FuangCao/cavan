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
	println("--help, -h\t\t\t\t%s", cavan_help_message_help);
	println("--version, -V, -v\t\t\t%s", cavan_help_message_version);
	println("--ip, -I, -i IP\t\t\t\t%s", cavan_help_message_ip);
	println("--host, -H HOSTNAME\t\t\t%s", cavan_help_message_hostname);
	println("--local, -L\t\t\t\t%s", cavan_help_message_local);
	println("--port, -P PORT\t\t\t\t%s", cavan_help_message_port);
	println("--adb, -A, -a\t\t\t\t%s", cavan_help_message_adb);
	println("--udp\t\t\t\t\t%s", cavan_help_message_udp);
	println("--tcp\t\t\t\t\t%s", cavan_help_message_tcp);
	println("--unix, --unix-tcp, -U, -u [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t\t%s", cavan_help_message_unix_udp);
	println("--url [URL]\t\t\t\t%s", cavan_help_message_url);
	println("-c, --command COMMAND\t\t\t%s", cavan_help_message_command);
	println("-l, --login\t\t\t\t%s", cavan_help_message_login);
	println("-m, -p, --preserve-environment\t\t%s", cavan_help_message_preserve_environment);
	println("-s, --shell SHELL\t\t\t%s", cavan_help_message_shell);
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
			.name = "udp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UDP,
		},
		{
			.name = "tcp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TCP,
		},
		{
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HOST,
		},
		{
			.name = "unix",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX,
		},
		{
			.name = "unix-tcp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_TCP,
		},
		{
			.name = "unix-udp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_UDP,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		},
		{
			0, 0, 0, 0
		},
	};
	const char *command;
	struct network_url url;

	command = NULL;
	network_url_init(&url, "unix-tcp", NULL, TCP_DD_DEFAULT_PORT, network_get_socket_pathname());

	while ((c = getopt_long(argc, argv, "vVhH:c:lmps:i:I:P:LaA", long_option, &option_index)) != EOF)
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

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			url.protocol = "adb";
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case 'H':
		case CAVAN_COMMAND_OPTION_IP:
		case CAVAN_COMMAND_OPTION_HOST:
			url.hostname = optarg;
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			url.protocol = "udp";
			break;

		case 't':
		case 'T':
		case CAVAN_COMMAND_OPTION_TCP:
			url.protocol = "tcp";
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			url.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url.protocol = "unix-tcp";
			if (optarg)
			{
				url.pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url.protocol = "unix-udp";
			if (optarg)
			{
				url.pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(&url, optarg) == NULL)
			{
				pr_red_info("Invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	return tcp_dd_exec_command(&url, command);
}
