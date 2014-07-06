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
	struct network_client *client;

	client = network_client_open2(url, CAVAN_NET_FLAG_UDP_TALK);
	if (client == NULL)
	{
		pr_red_info("network_client_open");
		return -EFAULT;
	}

	ret = client->send(client, buff, strlen(buff));
	if (ret < 0)
	{
		pr_red_info("client.send");
		goto out_network_client_close;
	}

	ret = client->recv(client, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("client.recv");
		goto out_network_client_close;
	}

	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

out_network_client_close:
	network_client_close(client);
	return ret;
}

static int network_service_test(const char *url)
{
	int ret;
	char buff[1024];
	struct network_client *client;
	struct network_service service;

	ret = network_service_open2(&service, url);
	if (ret < 0)
	{
		pr_red_info("network_service_open");
		return ret;
	}

	client = network_service_alloc_client(&service);
	if (client == NULL)
	{
		pr_error_info("network_service_alloc_client");
		goto out_network_service_close;
	}

	ret = service.accept(&service, client);
	if (ret < 0)
	{
		pr_red_info("service.accept");
		goto out_free_client;
	}

	ret = client->recv(client, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("client.recv");
		goto out_client_close;
	}

	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

	ret = client->send(client, "8888", 4);
	if (ret < 0)
	{
		pr_red_info("client.send");
		goto out_client_close;
	}

	msleep(500);

out_client_close:
	client->close(client);
out_free_client:
	free(client);
out_network_service_close:
	network_service_close(&service);
	return ret;
}

static int network_url_test(const char *_url)
{
	struct network_url url;

	if (network_url_parse(&url, _url) == NULL)
	{
		pr_red_info("web_proxy_parse_url");
	}
	else
	{
		println("protocol = %s", url.protocol);
		println("hostname = %s", url.hostname);
		println("port = %d", url.port);
		println("pathname = %s", url.pathname);
		println("url = %s", network_url_tostring(&url, NULL, 0, NULL));
	}

	return 0;
}

int main(int argc, char *argv[])
{
	assert(argc > 2);

	if (strcmp(argv[1], "client") == 0)
	{
		return network_client_test(argv[2]);
	}
	else if (strcmp(argv[1], "service") == 0)
	{
		return network_service_test(argv[2]);
	}
	else if (strcmp(argv[1], "url") == 0)
	{
		return network_url_test(argv[2]);
	}
	else
	{
		pr_red_info("unknown command %s", argv[1]);
	}

	return 0;
}
