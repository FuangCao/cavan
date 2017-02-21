/*
 * File:		http.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-02-13 17:52:12
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
#include <cavan/http.h>

static int cavan_http_client_on_read(struct cavan_epoll_service *service, struct cavan_epoll_client *client)
{
	int rdlen;
	char buff[1024];
	struct cavan_http_client *http_client = (struct cavan_http_client *) client;

	rdlen = network_client_recv(&http_client->network, buff, sizeof(buff));
	if (rdlen <= 0) {
		pr_red_info("network_client_recv");
		return -EFAULT;
	}

	buff[rdlen] = 0;
	pr_red_info("buff[%d] = %s", rdlen, buff);

	return 0;
}

static void cavan_http_client_on_close(struct cavan_epoll_service *service, struct cavan_epoll_client *client)
{
	struct cavan_http_client *http_client = (struct cavan_http_client *) client;

	network_client_close(&http_client->network);
}

static int cavan_http_file_service_on_read(struct cavan_epoll_service *service, struct cavan_epoll_client *client)
{
	int ret;
	struct cavan_http_file_service *http = cavan_epoll_service_get_data(service);
	struct cavan_http_client *http_client;

	http_client = malloc(sizeof(struct cavan_http_client));
	if (http_client == NULL) {
		pr_err_info("malloc");
		return -ENOMEM;
	}

	ret = network_service_accept(&http->service, &http_client->network);
	if (ret < 0) {
		pr_red_info("network_service_accept: %d", ret);
		goto out_free;
	}

	http_client->epoll.fd = http_client->network.sockfd;
	http_client->epoll.on_read = cavan_http_client_on_read;
	http_client->epoll.on_close = cavan_http_client_on_close;

	ret = cavan_epoll_service_add(service, &http_client->epoll);
	if (ret < 0) {
		pr_red_info("cavan_epoll_service_add: %d", ret);
		goto out_network_client_close;
	}

out_network_client_close:
	network_client_close(&http_client->network);
out_free:
	free(http_client);
	return ret;
}

int cavan_http_file_service_run(struct cavan_epoll_service *service)
{
	int ret;
	struct cavan_epoll_client client;
	struct cavan_http_file_service *http = cavan_epoll_service_get_data(service);

	ret = network_service_open(&http->service, &http->url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open: %d", ret);
		return ret;
	}

	service->name = "HTTP_FILE";

	ret = cavan_epoll_service_init(service);
	if (ret < 0) {
		pr_red_info("cavan_epoll_service_init: %d", ret);
		goto out_network_service_close;
	}

	client.fd = http->service.sockfd;
	client.on_read = cavan_http_file_service_on_read;
	cavan_epoll_service_add(service, &client);

	cavan_epoll_service_run(service);
	cavan_epoll_service_deinit(service);

out_network_service_close:
	network_service_close(&http->service);
	return ret;
}
