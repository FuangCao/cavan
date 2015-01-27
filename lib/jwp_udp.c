/*
 * File:		jwp_udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-27 20:06:25
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/service.h>
#include <cavan/jwp_udp.h>

static jwp_size_t jwp_udp_hw_read(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	ssize_t rdlen;
	struct jwp_udp_client *client = jwp_get_private_data(jwp);

	rdlen = network_client_recv(&client->client, buff, size);
	if (rdlen < 0)
	{
		pr_error_info("network_client_recv");
		return 0;
	}

	return rdlen;
}

static jwp_size_t jwp_udp_hw_write(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	ssize_t wrlen;
	struct jwp_udp_client *client = jwp_get_private_data(jwp);

	wrlen = network_client_send(&client->client, buff, size);
	if (wrlen < 0)
	{
		pr_error_info("write");
		return 0;
	}

	return wrlen;
}

static void jwp_udp_send_complete(struct jwp_desc *jwp)
{
	pr_pos_info();
}

static void jwp_udp_data_received(struct jwp_desc *jwp, const void *data, jwp_size_t size)
{
	println("data received: size = %d", size);
}

static void jwp_udp_command_received(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	char *text = (char *) command;

	text[size] = 0;
	pr_green_info("command = %s", text);
}

static void jwp_udp_package_received(struct jwp_desc *jwp, const struct jwp_header *hdr)
{
	pr_pos_info();
	jwp_header_dump(hdr);
}

#if JWP_TIMER_ENABLE
static int jwp_udp_timer_handler(struct cavan_timer *timer, void *data)
{
	struct jwp_timer *jwp_timer = data;

	println("%s run timer %s, msec = %d", jwp_timer->jwp->name, jwp_timer->name, jwp_timer->msec);

	jwp_timer_run(jwp_timer);

	return 0;
}

static jwp_bool jwp_udp_create_timer(struct jwp_timer *timer)
{
	struct cavan_timer *cavan_timer;
	struct jwp_udp_service *service = jwp_get_private_data(timer->jwp);

	println("%s create timer %s, msec = %d", timer->jwp->name, timer->name, timer->msec);

	if (timer->handle == NULL)
	{
		cavan_timer = malloc(sizeof(struct cavan_timer));
		if (cavan_timer == NULL)
		{
			return false;
		}

		cavan_timer_init(cavan_timer, timer);
		cavan_timer->handler = jwp_udp_timer_handler;

		timer->handle = cavan_timer;
	}
	else
	{
		cavan_timer = timer->handle;
	}

	cavan_timer_insert(&service->timer_service, cavan_timer, timer->msec);

	return true;
}

static void jwp_udp_delete_timer(struct jwp_timer *timer)
{
	println("%s delete timer %s, msec = %d" , timer->jwp->name, timer->name, timer->msec);

	if (timer->handle != NULL)
	{
		struct jwp_udp_service *service = jwp_get_private_data(timer->jwp);

		cavan_timer_remove(&service->timer_service, timer->handle);
	}
}
#endif

#if JWP_TX_DATA_LOOP_ENABLE
static void *jwp_udp_tx_data_loop_thread(void *data)
{
	jwp_tx_data_loop(data);

	return NULL;
}
#endif

#if JWP_TX_LOOP_ENABLE
static void *jwp_udp_tx_loop_thread(void *data)
{
	jwp_tx_loop(data);

	return NULL;
}
#endif

#if JWP_RX_PKG_LOOP_ENABLE
static void *jwp_udp_rx_package_loop_thread(void *data)
{
	jwp_rx_package_loop(data);

	return NULL;
}
#endif

#if JWP_RX_LOOP_ENABLE == 0
#error "must enable rx loop"
#endif

static void *jwp_udp_rx_loop_thread(void *data)
{
	jwp_rx_loop(data);

	return NULL;
}

int jwp_udp_client_init(struct jwp_udp_client *client)
{
	int ret;
	pthread_t td;
	struct jwp_desc *jwp = &client->jwp;

	jwp->hw_read = jwp_udp_hw_read,
	jwp->hw_write = jwp_udp_hw_write,
	jwp->send_complete = jwp_udp_send_complete,
	jwp->data_received = jwp_udp_data_received,
	jwp->command_received = jwp_udp_command_received,
	jwp->package_received = jwp_udp_package_received,
#if JWP_TIMER_ENABLE
	jwp->create_timer = jwp_udp_create_timer,
	jwp->delete_timer = jwp_udp_delete_timer,
#endif

	ret = jwp_init(jwp, client);
	if (ret < 0)
	{
		pr_red_info("jwp_init");
		return ret;
	}

#if JWP_TX_DATA_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_udp_tx_data_loop_thread, jwp);
#endif

#if JWP_TX_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_udp_tx_loop_thread, jwp);
#endif

#if JWP_RX_PKG_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_udp_rx_package_loop_thread, jwp);
#endif

	pthread_create(&td, NULL, jwp_udp_rx_loop_thread, jwp);

	return 0;
}

static int jwp_udp_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	struct jwp_udp_client *jwp_udp_conn = conn;
	struct jwp_udp_service *jwp_udp = cavan_dynamic_service_get_data(service);

	ret = jwp_udp->service.accept(&jwp_udp->service, &jwp_udp_conn->client);
	if (ret < 0)
	{
		pr_red_info("jwp_udp->service.accept");
		return ret;
	}

	ret = jwp_udp_client_init(jwp_udp_conn);
	if (ret < 0)
	{
		pr_red_info("jwp_udp_client_init");
		goto out_network_client_close;
	}

	return 0;

out_network_client_close:
	network_client_close(&jwp_udp_conn->client);
	return ret;
}

static void jwp_udp_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int jwp_udp_service_start_handler(struct cavan_dynamic_service *service)
{
	int ret;
	struct jwp_udp_service *jwp_udp = cavan_dynamic_service_get_data(service);

	ret = network_service_open(&jwp_udp->service, &jwp_udp->url, 0);
	if (ret < 0)
	{
		pr_red_info("network_service_open2");
		return ret;
	}

	ret = cavan_timer_service_start(&jwp_udp->timer_service);
	if (ret < 0)
	{
		pr_red_info("jwp_udp_service_init");
		goto out_network_service_close;
	}

	return 0;

out_network_service_close:
	network_service_close(&jwp_udp->service);
	return ret;
}

static void jwp_udp_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct jwp_udp_service *jwp_udp = cavan_dynamic_service_get_data(service);

	cavan_timer_service_stop(&jwp_udp->timer_service);
	network_service_close(&jwp_udp->service);
}

static int jwp_udp_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	struct jwp_udp_client *client = conn;
	struct jwp_desc *jwp = &client->jwp;

	while (1)
	{
		char buff[1024];

		if (scanf("%s", buff) < 1)
		{
			pr_error_info("scanf");
		}

		println("buff = %s", buff);

		if (!jwp_send_command(jwp, buff, strlen(buff)))
		{
			pr_red_info("jwp_send_command failed!");
		}
	}

	return 0;
}

int jwp_udp_service_run(struct cavan_dynamic_service *service)
{
	service->name = "JWP_UDP";
	service->conn_size = sizeof(struct jwp_udp_client);

	service->start = jwp_udp_service_start_handler;
	service->stop = jwp_udp_service_stop_handler;
	service->run = jwp_udp_service_run_handler;
	service->open_connect = jwp_udp_service_open_connect;
	service->close_connect = jwp_udp_service_close_connect;

	return cavan_dynamic_service_run(service);
}
