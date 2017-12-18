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

static int role_change_service_main(int argc, char *argv[])
{
	struct cavan_dynamic_service *service;
	struct role_change_service *role;
	int ret;

	assert(argc > 1);

	service = cavan_dynamic_service_create(sizeof(struct role_change_service));
	if (service == NULL) {
		pr_err_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->verbose = true;
	role = cavan_dynamic_service_get_data(service);

	if (network_url_parse(&role->url, argv[1]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	ret = role_change_service_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

static int role_change_client_main(int argc, char *argv[])
{
	struct cavan_dynamic_service *service;
	struct role_change_client *role;
	int ret;

	assert(argc > 2);

	service = cavan_dynamic_service_create(sizeof(struct role_change_client));
	if (service == NULL) {
		pr_err_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->verbose = true;
	role = cavan_dynamic_service_get_data(service);

	ret = gethostname(role->name, sizeof(role->name));
	if (ret < 0) {
		pr_red_info("gethostname");
		return ret;
	}

	if (network_url_parse(&role->url_local, argv[1]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	if (network_url_parse(&role->url_remote, argv[2]) == NULL) {
		pr_red_info("network_url_parse");
		ret = -EINVAL;
		goto out_cavan_dynamic_service_destroy;
	}

	if (argc > 4) {
		role->proxy_name = argv[3];
		role->proxy_url = argv[4];
	} else {
		role->proxy_name = NULL;
		role->proxy_url = NULL;
	}

	ret = role_change_client_run(service);

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
	{ "list", role_change_list_main },
} CAVAN_COMMAND_MAP_END;
