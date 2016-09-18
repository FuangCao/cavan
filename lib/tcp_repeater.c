/*
 * File:		tcp_repeater.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 16:30:05
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

static int tcp_repeater_start_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	return network_service_open(&repeater->service, &repeater->url, 0);
}

static void tcp_repeater_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	network_service_close(&repeater->service);
}

static int tcp_repeater_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	return network_service_accept(&repeater->service, conn);
}

static void tcp_repeater_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int cavan_tcp_repeater_run_handler(struct cavan_dynamic_service *service, void *_conn)
{
	char buff[1024];
	struct cavan_tcp_repeater_conn *head;
	struct cavan_tcp_repeater_conn *conn = _conn;
	struct network_client *client = &conn->client;
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	cavan_dynamic_service_lock(service);

	if (repeater->head == NULL) {
		conn->prev = conn->next = conn;
		repeater->head = conn;
	} else {
		head = repeater->head;

		conn->next = head->next;
		head->next = conn;

		conn->next->prev = conn;
		conn->prev = head;
	}

	cavan_dynamic_service_unlock(service);

	while (1) {
		int length;

		length = client->recv(client, buff, sizeof(buff));
		if (length <= 0) {
			break;
		}

		buff[length] = 0;
		println("buff[%d] = %s", length, buff);

		cavan_dynamic_service_lock(service);

		for (head = conn->next; head != conn; head = head->next) {
			head->client.send(&head->client, buff, length);
		}

		cavan_dynamic_service_unlock(service);
	}

	cavan_dynamic_service_lock(service);

	if (conn == conn->next) {
		repeater->head = NULL;
	} else {
		if (conn == repeater->head) {
			repeater->head = conn->next;
		}

		conn->next->prev = conn->prev;
		conn->prev->next = conn->next;
	}

	cavan_dynamic_service_unlock(service);

	return 0;
}

int cavan_tcp_repeater_run(struct cavan_dynamic_service *service)
{
	service->name = "TCP_REPEATER";
	service->conn_size = sizeof(struct cavan_tcp_repeater_conn);
	service->start = tcp_repeater_start_handler;
	service->stop = tcp_repeater_stop_handler;
	service->run = cavan_tcp_repeater_run_handler;
	service->open_connect = tcp_repeater_open_connect;
	service->close_connect = tcp_repeater_close_connect;

	return cavan_dynamic_service_run(service);
}
