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

static bool tcp_repeater_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int tcp_repeater_keepalive_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	if (repeater->head) {
		u64 time = clock_gettime_mono_ms();
		struct cavan_tcp_repeater_conn *conn, *head;

		conn = head = repeater->head;

		while (1) {
			u64 idle = time - conn->alive_time;

			if (idle < TCP_REPEATER_KEEP_ALIVE_OVERTIME) {
				int ret = network_client_send(&conn->client, TCP_REPEATER_KEEP_ALIVE_COMMAND "\n", sizeof(TCP_REPEATER_KEEP_ALIVE_COMMAND));
				if (ret < 0) {
					network_client_close(&conn->client);
				}
			} else {
				network_client_close(&conn->client);
			}

			conn = conn->next;
			if (conn == head) {
				break;
			}
		};
	}

	return TCP_REPEATER_KEEP_ALIVE_DELAY;
}

static int tcp_repeater_run_handler(struct cavan_dynamic_service *service, void *_conn)
{
	int ret;
	struct cavan_fifo fifo;
	struct cavan_tcp_repeater_conn *head;
	struct cavan_tcp_repeater_conn *conn = _conn;
	struct cavan_tcp_repeater *repeater = cavan_dynamic_service_get_data(service);

	ret = cavan_fifo_init(&fifo, 1024, &conn->client);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init: %d", ret);
		return ret;
	}

	fifo.read = network_client_fifo_read;

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

	while (1) {
		char *p;
		char buff[1024];
		int rdlen, wrlen;

		conn->alive_time = clock_gettime_mono_ms();

		cavan_dynamic_service_unlock(service);
		p = cavan_fifo_read_line(&fifo, buff, sizeof(buff) - 1);
		cavan_dynamic_service_lock(service);

		if (p == NULL) {
			break;
		}

		*p = 0;
		rdlen = p - buff;

		pd_info("tcp_repeater[%d] = %s", rdlen, buff);

		if (text_lhcmp(TCP_REPEATER_KEEP_ALIVE_COMMAND, buff) == 0) {
			continue;
		}

		for (head = conn->next; head != conn; head = head->next) {
			wrlen = network_client_send(&head->client, buff, rdlen);
			if (wrlen < rdlen) {
				network_client_close(&head->client);
			}
		}
	}

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
	cavan_fifo_deinit(&fifo);

	return 0;
}

int cavan_tcp_repeater_run(struct cavan_dynamic_service *service)
{
	service->name = "TCP_REPEATER";
	service->conn_size = sizeof(struct cavan_tcp_repeater_conn);
	service->start = tcp_repeater_start_handler;
	service->stop = tcp_repeater_stop_handler;
	service->run = tcp_repeater_run_handler;
	service->keepalive = tcp_repeater_keepalive_handler;
	service->open_connect = tcp_repeater_open_connect;
	service->close_connect = tcp_repeater_close_connect;

	return cavan_dynamic_service_run(service);
}
