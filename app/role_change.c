/*
 * File:		role_change.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-15 19:08:16
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/role_change.h>

static void role_change_service_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] URL", command);
	println("-H, -h, --help\t\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t\t%s", cavan_help_message_verbose);
	println("-D, -d, --daemon\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t%s", cavan_help_message_daemon_max);
	println("-L, -l, --log\t\t\t%s", cavan_help_message_logfile);
}

static int role_change_service_main(int argc, char *argv[])
{
	struct cavan_dynamic_service *service;
	struct role_change_service *role;
	int ret;
	int c;
	int option_index;
	struct option long_option[] = {
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
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		}, {
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		}, {
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		}, {
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		}, {
			0, 0, 0, 0
		},
	};

	service = cavan_dynamic_service_create(sizeof(struct role_change_service));
	if (service == NULL) {
		pr_err_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;

	role = cavan_dynamic_service_get_data(service);

	while ((c = getopt_long(argc, argv, "vVhHm:c:M:C:dDl:L:", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			service->verbose = true;
			break;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			role_change_service_show_usage(argv[0]);
			return 0;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = true;
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			role_change_service_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(optind < argc);

	if (network_url_parse(&role->url, argv[optind++]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	ret = role_change_service_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

static void role_change_client_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] URL [NAME]", command);
	println("-H, -h, --help\t\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t\t%s", cavan_help_message_verbose);
	println("-D, -d, --daemon\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t%s", cavan_help_message_daemon_max);
	println("-L, -l, --log\t\t\t%s", cavan_help_message_logfile);
	println("-N, -n, --name\t\t\t%s", cavan_help_message_service_name);
}

static int role_change_client_main(int argc, char *argv[])
{
	struct cavan_dynamic_service *service;
	struct role_change_client *role;
	int ret;
	int c;
	int option_index;
	const char *name = NULL;
	struct option long_option[] = {
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
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		}, {
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		}, {
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		}, {
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		}, {
			.name = "name",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_NAME,
		}, {
			0, 0, 0, 0
		},
	};

	service = cavan_dynamic_service_create(sizeof(struct role_change_client));
	if (service == NULL) {
		pr_err_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 2;
	service->max = 1000;

	role = cavan_dynamic_service_get_data(service);

	while ((c = getopt_long(argc, argv, "vVhHm:c:M:C:dDl:L:n:N:", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			service->verbose = true;
			break;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			role_change_client_show_usage(argv[0]);
			return 0;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = true;
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'n':
		case 'N':
		case CAVAN_COMMAND_OPTION_NAME:
			name = optarg;
			break;

		default:
			role_change_client_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(optind < argc);

	if (network_url_parse(&role->url, argv[optind++]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	if (name != NULL) {
		strcpy(role->name, name);
	} else if (optind < argc) {
		strcpy(role->name, argv[optind++]);
	} else {
		ret = gethostname(role->name, sizeof(role->name));
		if (ret < 0) {
			pr_red_info("gethostname");
			return ret;
		}
	}

	ret = role_change_client_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

static void role_change_proxy_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] URL_LOCAL URL_REMOTE URL [NAME]", command);
	println("-H, -h, --help\t\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t\t%s", cavan_help_message_verbose);
	println("-D, -d, --daemon\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t%s", cavan_help_message_daemon_max);
	println("-L, -l, --log\t\t\t%s", cavan_help_message_logfile);
	println("-B, -b, --burrow\t\t%s", cavan_help_message_burrow);
}

static int role_change_proxy_main(int argc, char *argv[])
{
	struct cavan_dynamic_service *service;
	struct role_change_proxy *proxy;
	int ret;
	int c;
	int option_index;
	struct option long_option[] = {
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
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		}, {
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		}, {
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		}, {
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		}, {
			.name = "burrow",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BURROW,
		}, {
			0, 0, 0, 0
		},
	};

	service = cavan_dynamic_service_create(sizeof(struct role_change_client));
	if (service == NULL) {
		pr_err_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;

	proxy = cavan_dynamic_service_get_data(service);

	while ((c = getopt_long(argc, argv, "vVhHm:c:M:C:dDl:L:Bb", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			service->verbose = true;
			break;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			role_change_proxy_show_usage(argv[0]);
			return 0;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = true;
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'b':
		case 'B':
		case CAVAN_COMMAND_OPTION_BURROW:
			proxy->burrow = true;
			break;

		default:
			role_change_proxy_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(optind + 2 < argc);

	if (network_url_parse(&proxy->url_local, argv[optind++]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	if (network_url_parse(&proxy->url_remote, argv[optind++]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	proxy->url = argv[optind++];

	if (optind < argc) {
		proxy->name = argv[optind++];
	} else {
		proxy->burrow = false;
		proxy->name = NULL;
	}

	ret = role_change_proxy_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

static int role_change_list_main(int argc, char *argv[])
{
	int ret;
	char buff[4096];
	struct network_client client;

	assert(argc > 1);

	ret = network_client_open2(&client, argv[1], 0);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = network_client_send_packet(&client, "list", 4);
	if (ret < 0) {
		pr_red_info("network_client_send_packet");
		goto out_network_client_close;
	}

	ret = network_client_recv_packet(&client, buff, sizeof(buff));
	buff[ret] = 0;
	puts(buff);

out_network_client_close:
	network_client_close(&client);
	return ret;
}

CAVAN_COMMAND_MAP_START {
	{ "service", role_change_service_main },
	{ "client", role_change_client_main },
	{ "proxy", role_change_proxy_main },
	{ "list", role_change_list_main },
} CAVAN_COMMAND_MAP_END;
