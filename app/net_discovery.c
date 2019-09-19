#define CAVAN_CMD_NAME net_discovery

/*
 * File:		net_discovery.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-04-06 19:13:31
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
#include <cavan/tcp_dd.h>
#include <cavan/network.h>

static u16 net_discovery_get_port(int argc, char *argv[], u16 port_def)
{
	if (argc > 1) {
		return text2value_unsigned(argv[1], NULL, 10);
	} else {
		return CAVAN_DISCOVERY_PORT;
	}
}

static int do_discovery_udp(int argc, char *argv[])
{
	int count;
	u16 port = net_discovery_get_port(argc, argv, CAVAN_DISCOVERY_PORT);

	count = udp_discovery_client_run(port, NULL, NULL);
	if (count <= 0) {
		pr_red_info("No service found");
	}

	return 0;
}

static int do_discovery_tcp(int argc, char *argv[])
{
	int count;
	struct tcp_discovery_client client;

	client.handler = NULL;
	client.port = net_discovery_get_port(argc, argv, CAVAN_DISCOVERY_PORT);

	count = tcp_discovery_client_run(&client, NULL);
	if (count <= 0) {
		pr_red_info("No service found");
	}

	return 0;
}

static int do_discovery_tcp_dd(int argc, char *argv[])
{
	int count;
	struct tcp_dd_discovery_client client;

	client.handler = NULL;
	client.client.port = net_discovery_get_port(argc, argv, TCP_DD_DEFAULT_PORT);

	count = tcp_dd_discovery(&client, NULL);
	if (count <= 0) {
		pr_red_info("No service found");
	}

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "udp", do_discovery_udp },
	{ "tcp", do_discovery_tcp },
	{ "tcp_dd", do_discovery_tcp_dd },
} CAVAN_COMMAND_MAP_END;
