/*
 * File:		web_proxy.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-08-22 14:10:51
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/web_proxy.h>

#define FILE_CREATE_DATE "2013-08-22 14:10:51"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_PORT,
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_MIN,
	LOCAL_COMMAND_OPTION_MAX,
	LOCAL_COMMAND_OPTION_VERBOSE,
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] proxy_port", command);
	println("--help, -h, -H\t\t\tshow this help");
	println("--version, -v, -V\t\tshow version");
	println("--port, -p, -P\t\t\tserver port");
	println("--adb, -a, -A\t\t\tuse adb procotol instead of tcp");
	println("--daemon, -d, -D\t\trun as a daemon");
	println("--min, -m, -c\t\t\tmin daemon count");
	println("--max, -M, -C\t\t\tmax daemon count");
	println("--verbose\t\t\tshow log message");
}

int main(int argc, char *argv[])
{
	int ret;
	int c;
	int option_index;
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
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PORT,
		},
		{
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON,
		},
		{
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MIN,
		},
		{
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MAX,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
		{
			0, 0, 0, 0
		},
	};
	struct cavan_dynamic_service *service;
	struct web_proxy_service *proxy_service;

	service = cavan_dynamic_service_create(sizeof(struct web_proxy_service));
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;
	service->as_daemon = 0;
	service->show_verbose = 0;
	service->super_permission = 0;

	proxy_service = cavan_dynamic_service_get_data(service);
	proxy_service->port = 9090;

	while ((c = getopt_long(argc, argv, "vVhHp:P:c:C:m:M:dD", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			ret = 0;
			goto out_cavan_dynamic_service_destroy;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			ret = 0;
			goto out_cavan_dynamic_service_destroy;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PORT:
			proxy_service->port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			service->as_daemon = 1;
			break;

		case 'c':
		case LOCAL_COMMAND_OPTION_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case LOCAL_COMMAND_OPTION_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case LOCAL_COMMAND_OPTION_VERBOSE:
			service->show_verbose = 1;
			break;

		default:
			show_usage(argv[0]);
			ret = -EINVAL;
			goto out_cavan_dynamic_service_destroy;
		}
	}

	if (optind < argc)
	{
		proxy_service->port = text2value_unsigned(argv[optind], NULL, 10);
	}

	ret = web_proxy_service_run(service);
	if (ret < 0)
	{
		pr_red_info("tcp_proxy_service_run");
	}

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

