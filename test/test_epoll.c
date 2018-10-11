/*
 * File:		test_epoll.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-10-10 16:05:51
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/service.h>

struct network_epoll_service {
	struct cavan_epoll_client ep_client;
	struct cavan_epoll_service ep_service;
	struct network_service net_service;
};

struct network_epoll_client {
	struct cavan_epoll_client ep_client;
	struct network_client net_client;
};

static bool network_epoll_service_do_poll(struct cavan_epoll_client *client)
{
	struct network_epoll_service *service = (struct network_epoll_service *) client;
	struct network_epoll_client *conn = malloc(service->ep_service.conn_size);
	pr_pos_info();

	if (conn == NULL) {
		return true;
	}

	pr_pos_info();

	if (service->net_service.accept(&service->net_service, &conn->net_client, 0) < 0) {
		pr_pos_info();
		client->events &= ~EPOLLIN;
		goto out_free_conn;
	}
	pr_pos_info();

	cavan_epoll_client_init(&conn->ep_client, conn->net_client.sockfd);
	pr_pos_info();

	if (cavan_epoll_service_add(&service->ep_service, &conn->ep_client) < 0) {
		goto out_free_conn;
	}
	pr_pos_info();

	return true;

out_free_conn:
	free(conn);
	return true;
}

static int network_epoll_service_open(struct network_epoll_service *service, const char *url)
{
	int ret;
	struct cavan_epoll_client *client;

	ret = network_service_open2(&service->net_service, url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open2: %d", ret);
		return ret;
	}

	ret = cavan_epoll_service_init(&service->ep_service);
	if (ret < 0) {
		pr_red_info("cavan_epoll_service_init: %d", ret);
		return ret;
	}

	client = &service->ep_client;
	cavan_epoll_client_init(client, service->net_service.sockfd);
	client->do_poll = network_epoll_service_do_poll;

	ret = cavan_epoll_service_add(&service->ep_service, client);
	if (ret < 0) {
		pr_red_info("cavan_epoll_service_add: %d", ret);
		return ret;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct network_epoll_service service;
	int ret;

	assert(argc > 1);

	service.ep_service.max = 200;
	service.ep_service.conn_size = sizeof(struct network_epoll_client);

	ret = network_epoll_service_open(&service, argv[1]);
	if (ret < 0) {
		pr_red_info("network_epoll_service_open: %d", ret);
		return ret;
	}

	cavan_epoll_service_run(&service.ep_service);

	return 0;
}