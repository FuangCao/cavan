/*
 * File:		test_discovery.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-04-06 11:17:35
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
#include <cavan/network.h>

static int test_discovery_service_main(int argc, char *argv[])
{
	int ret;
	struct network_discovery_service service;

	assert(argc > 1);

	if (argc > 2) {
		service.port = text2value_unsigned(argv[2], NULL, 10);
	} else {
		service.port = 8888;
	}

	service.delay = 2000;

	ret = network_discovery_service_start(&service, argv[1]);
	if (ret < 0) {
		pr_red_info("network_discovery_service_start: %d", ret);
		return ret;
	}

	cavan_thread_join(&service.thread);

	return 0;
}

static int test_discovery_client_main(int argc, char *argv[])
{
	u16 port;

	if (argc > 1) {
		port = text2value_unsigned(argv[1], NULL, 10);
	} else {
		port = 8888;
	}

	return network_discovery_client_run(port);
}


CAVAN_COMMAND_MAP_START {
	{ "service", test_discovery_service_main },
	{ "client", test_discovery_client_main },
} CAVAN_COMMAND_MAP_END;
