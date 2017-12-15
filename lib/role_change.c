/*
 * File:		role_change.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-15 19:07:42
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
#include <cavan/tcp_proxy.h>
#include <cavan/role_change.h>

static int role_change_service_start_handler(struct cavan_dynamic_service *service)
{
	int ret;
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	ret = network_service_open(&role->service, &role->url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open");
		return ret;
	}

	ret = network_service_open(&role->proxy, &role->url_proxy, 0);
	if (ret < 0) {
		pr_red_info("network_service_open");
		goto out_network_service_close;
	}

	return 0;

out_network_service_close:
	network_service_close(&role->service);
	return ret;
}

static void role_change_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	network_service_close(&role->proxy);
	network_service_close(&role->service);
}

static int role_change_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	return network_service_accept(&role->service, conn);
}

static void role_change_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int role_change_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	struct network_client proxy;
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	while (true) {
		ret = network_service_accept_timed(&role->proxy, &proxy, 60000);
		if (ret < 0) {
			if (ret != -ETIMEDOUT) {
				pr_err_info("network_service_accept_timed");
				return ret;
			}

			ret = network_client_send_packet(conn, "keepalive", 9);
			if (ret < 0) {
				pr_err_info("network_service_accept_timed");
				return ret;
			}
		} else {
			break;
		}
	}

	ret = network_client_send_packet(conn, "link", 4);
	if (ret < 0) {
		pr_err_info("network_service_accept_timed");
		goto out_network_client_close;
	}

	tcp_proxy_main_loop(conn, &proxy);

out_network_client_close:
	network_client_close(&proxy);
	return ret;
}

int role_change_service_run(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	pd_bold_info("URL = %s", network_url_tostring(&role->url, NULL, 0, NULL));
	pd_bold_info("PROXY_URL = %s", network_url_tostring(&role->url_proxy, NULL, 0, NULL));

	service->name = "ROLE_CHANGE";
	service->conn_size = sizeof(struct network_client);
	service->start = role_change_service_start_handler;
	service->stop = role_change_service_stop_handler;
	service->run = role_change_service_run_handler;
	service->open_connect = role_change_service_open_connect;
	service->close_connect = role_change_service_close_connect;

	return cavan_dynamic_service_run(service);
}
