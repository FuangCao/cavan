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
#include <cavan/thread.h>

static void *network_client_recv_handler(void *data)
{
	struct network_client *client = data;

	while (1) {
		u32 value;
		ssize_t rdlen;

		println("client recv -");

		rdlen = client->recv(client, &value, sizeof(value));
		if (rdlen != sizeof(value)) {
			pr_red_info("client->recv");
			break;
		}

		println("client recv %d", value);
	}

	return NULL;
}

static int network_client_test_base(struct network_client *client)
{
	int ret;
	u32 value;
	pthread_t thread_recv;

	cavan_pthread_create(&thread_recv, network_client_recv_handler, client, true);

	for (value = 0; value < 2000; value++) {
		println("client send %d", value);

		ret = client->send(client, &value, sizeof(value));
		if (ret != sizeof(value)) {
			pr_red_info("client->send");
			goto out_network_client_close;
		}

		msleep(10);
	}

	msleep(1000);

	ret = 0;
out_network_client_close:
	network_client_close(client);
	pthread_join(thread_recv, NULL);

	return ret;
}

static int do_test_client(int argc, char *argv[])
{
	int ret;
	const char *url;
	struct network_client client;

	assert(argc > 1);

	url = argv[1];

	ret = network_client_open2(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0) {
		pr_red_info("network_client_open");
		return -EFAULT;
	}

	return network_client_test_base(&client);
}

static int do_test_service(int argc, char *argv[])
{
	int ret;
	const char *url;
	struct network_client client;
	struct network_service service;

	assert(argc > 1);

	url = argv[1];

	ret = network_service_open2(&service, url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open");
		return ret;
	}

	ret = service.accept(&service, &client);
	if (ret < 0) {
		pr_red_info("service.accept");
		goto out_network_service_close;
	}

	ret = network_client_test_base(&client);

out_network_service_close:
	network_service_close(&service);
	return ret;
}

static int do_test_url(int argc, char *argv[])
{
	const char *url_text;
	struct network_url url;

	assert(argc > 1);

	url_text = argv[1];

	if (network_url_parse(&url, url_text) == NULL) {
		pr_red_info("web_proxy_parse_url");
	} else {
		println("protocol = %s", url.protocol);
		println("hostname = %s", url.hostname);
		println("port = %d", url.port);
		println("pathname = %s", url.pathname);
		println("url = %s", network_url_tostring(&url, NULL, 0, NULL));
	}

	return 0;
}

static int do_test_dump(int argc, char *argv[])
{
	int ret;
	const char *url;
	struct network_client client;
	struct network_service service;

	assert(argc > 1);

	url = argv[1];

	ret = network_service_open2(&service, url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open2");
		return ret;
	}

	while (1) {
		ret = network_service_accept(&service, &client);
		if (ret < 0) {
			pr_red_info("network_service_accept");
			goto out_network_service_close;
		}

		while (1) {
			char buff[1024];

			ret = client.recv(&client, buff, sizeof(buff));
			if (ret <= 0) {
				pr_red_info("client->recv");
				break;
			}

			print_ntext(buff, ret);
		}

		network_client_close(&client);
	}

out_network_service_close:
	network_service_close(&service);
	return ret;
}

static int do_test_send(int argc, char *argv[])
{
	int ret;
	const char *url;
	const char *pathname;
	struct network_client client;

	assert(argc > 2);

	url = argv[1];
	pathname = argv[2];

	ret = network_client_open2(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0) {
		pr_red_info("network_client_open");
		return -EFAULT;
	}

	ret = network_client_send_file2(&client, pathname, 0);
	if (ret < 0) {
		pr_red_info("network_client_send_file2");
	} else {
		char buff[1024];

		ret = client.recv(&client, buff, sizeof(buff));
		if (ret > 0) {
			print_ntext(buff, ret);
		} else {
			pr_red_info("client.recv");
		}
	}

	network_client_close(&client);

	return ret;
}

static int do_test_ifconfig(int argc, char *argv[])
{
	int i;
	int count;
	struct cavan_inet_ifconfig configs[8];

	count = cavan_inet_get_ifconfig_list2(configs, NELEM(configs));
	if (count < 0) {
		pr_red_info("cavan_network_get_device_list");
		return count;
	}

	for (i = 0; i < count; i++) {
		cavan_inet_ifconfig_dump(configs + i);
		print_sep(60);
	}

	return 0;
}

static int do_test_route(int argc, char *argv[])
{
	int i;
	int ret;
	int count;
	struct cavan_inet_route routes[16];
	struct cavan_inet_route def_route;

	count = cavan_inet_get_route_table(routes, NELEM(routes));
	if (count < 0) {
		pr_red_info("cavan_network_get_device_list");
		return count;
	}

	for (i = 0; i < count; i++) {
		cavan_inet_route_dump(routes + i);
		print_sep(60);
	}

	ret = cavan_inet_get_default_route(&def_route);
	if (ret < 0) {
		pr_red_info("cavan_inet_get_default_route: %d", ret);
		return ret;
	}

	pr_green_info("default route is:");
	cavan_inet_route_dump(&def_route);

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "client", do_test_client },
	{ "service", do_test_service },
	{ "url", do_test_url },
	{ "dump", do_test_dump },
	{ "send", do_test_send },
	{ "ifconfig", do_test_ifconfig },
	{ "route", do_test_route },
} CAVAN_COMMAND_MAP_END;
