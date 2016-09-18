/*
 * File:		tcp_repeater.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 16:29:39
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
#include <cavan/tcp_repeater.h>

int main(int argc, char *argv[])
{
	int ret;
	struct cavan_tcp_repeater *repeater;
	struct cavan_dynamic_service *service;

	service = cavan_dynamic_service_create(sizeof(struct cavan_tcp_repeater));
	if (service == NULL) {
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;
	service->verbose = true;

	repeater = cavan_dynamic_service_get_data(service);
	network_url_init(&repeater->url, "tcp", "any", CAVAN_TCP_REPEATER_PORT, network_get_socket_pathname());

	ret = network_url_parse_cmdline(&repeater->url, argc, argv);
	if (ret < 0) {
		pr_red_info("network_url_parse_cmdline: %d", ret);
		network_url_show_usage(argv[0]);
		goto out_cavan_dynamic_service_destroy;
	}

	ret = cavan_tcp_repeater_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}
