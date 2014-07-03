/*
 * File:		testNetwork.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-08-22 09:48:48
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
#include <cavan/tcp_proxy.h>
#include <cavan/network.h>

static int network_client_test(const char *url)
{
	int ret;
	char buff[1024] = "123456789";
	struct network_client client;

	ret = network_client_open2(&client, url);
	if (ret < 0)
	{
		pr_red_info("network_client_open");
		return ret;
	}

	ret = client.send(&client, buff, strlen(buff));
	if (ret < 0)
	{
		pr_red_info("client.send");
		goto out_network_client_close;
	}

	ret = client.recv(&client, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("client.recv");
		goto out_network_client_close;
	}

	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

out_network_client_close:
	network_client_close(&client);
	return ret;
}

static int network_service_test(const char *url)
{
	int ret;
	char buff[1024];
	struct network_connect conn;
	struct network_service service;

	ret = network_service_open2(&service, url);
	if (ret < 0)
	{
		pr_red_info("network_service_open");
		return ret;
	}

	ret = service.accept(&service, &conn);
	if (ret < 0)
	{
		pr_red_info("service->accept");
		goto out_network_service_close;
	}

	ret = conn.recv(&conn, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("conn.recv");
		goto out_conn_close;
	}

	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

	ret = conn.send(&conn, "8888", 4);
	if (ret < 0)
	{
		pr_red_info("conn.send");
		goto out_conn_close;
	}

	msleep(500);

out_conn_close:
	conn.close(&conn);
out_network_service_close:
	network_service_close(&service);
	return ret;
}

int main(int argc, char *argv[])
{
#if 0
	struct network_url url;

	assert(argc > 1);

	if (network_parse_url(argv[1], &url) == NULL)
	{
		pr_red_info("web_proxy_parse_url");
	}
	else
	{
		println("%s", network_url_tostring(&url, NULL, 0, NULL));
	}
#else
	assert(argc > 2);

	if (strcmp(argv[1], "client") == 0)
	{
		return network_client_test(argv[2]);
	}
	else if (strcmp(argv[1], "service") == 0)
	{
		return network_service_test(argv[2]);
	}
	else
	{
		pr_red_info("unknown command %s", argv[1]);
	}
#endif

	return 0;
}
