/*
 * File:		network.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 17:43:29
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
#include <cavan/service.h>

struct app_network_service {
	struct network_service service;
	const char *url;
};

static void *app_network_receive_thread(void *data)
{
	struct network_client *client = data;

	while (1) {
		int length;
		char buff[1024];

		length = client->recv(client, buff, sizeof(buff));
		if (length <= 0) {
			break;
		}

		buff[length] = 0;
		println("receive[%d]: %s", length, buff);
	}

	return NULL;
}

static int app_network_cmdline(struct network_client *client)
{
	int ret;
	int length;
	char buff[1024];
	pthread_t thread;

	ret = cavan_pthread_create(&thread, app_network_receive_thread, client, true);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create: %d", ret);
		return ret;
	}

	while (1) {
		print_ntext("> ", 2);

		if (fgets(buff, sizeof(buff), stdin) == NULL) {
			pr_err_info("fgets");
			break;
		}

		length = strlen(buff);
		buff[length] = 0;

		println("send[%d]: %s", length, buff);

		length = client->send(client, buff, length);
		if (length <= 0) {
			break;
		}
	}

	cavan_pthread_join(thread);

	return 0;
}

static int app_network_client_main(int argc, char *argv[])
{
	int ret;
	struct network_client client;

	assert(argc > 1);

	ret = network_client_open2(&client, argv[1], CAVAN_NET_FLAG_WAIT);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = app_network_cmdline(&client);

	network_client_close(&client);

	return ret;
}

static int app_network_start_handler(struct cavan_dynamic_service *service)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	return network_service_open2(&app_service->service, app_service->url, 0);
}

static void app_network_stop_handler(struct cavan_dynamic_service *service)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	network_service_close(&app_service->service);
}

static int app_network_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	return network_service_accept(&app_service->service, conn);
}

static bool app_network_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int app_network_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	return app_network_cmdline(conn_data);
}

static int app_network_service_main(int argc, char *argv[])
{
	int ret;
	struct cavan_dynamic_service *service;
	struct app_network_service *app_service;

	assert(argc > 1);

	service = cavan_dynamic_service_create(sizeof(struct app_network_service));
	if (service == NULL) {
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;
	service->verbose = true;
	service->name = "APP_NETWORK";
	service->conn_size = sizeof(struct network_client);
	service->start = app_network_start_handler;
	service->stop = app_network_stop_handler;
	service->run = app_network_run_handler;
	service->open_connect = app_network_open_connect;
	service->close_connect = app_network_close_connect;

	app_service = cavan_dynamic_service_get_data(service);
	app_service->url = argv[1];

	ret = cavan_dynamic_service_run(service);

	cavan_dynamic_service_destroy(service);

	return ret;
}


CAVAN_COMMAND_MAP_START {
	{ "client", app_network_client_main },
	{ "service", app_network_service_main },
} CAVAN_COMMAND_MAP_END;
