/*
 * File:			jwp-linux.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-30 11:41:56
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
#include <cavan/jwp-linux.h>

#define JWP_LINUX_DEBUG			0

const char *jwp_device_to_string(jwp_device_t device)
{
	switch (device)
	{
	case JWP_DEVICE_LOCAL:
		return "Local";

	case JWP_DEVICE_REMOTE:
		return "Remote";

	default:
		return "unknown";
	}
}

#if JWP_TIMER_ENABLE
static int jwp_linux_timer_handler(struct cavan_timer *cavan_timer, void *data)
{
	struct jwp_timer *timer = data;

#if JWP_DEBUG_MEMBER && JWP_LINUX_DEBUG
	println("run timer %s, msec = %d", timer->name, timer->msec);
#endif

	jwp_timer_run(timer);

	return 0;
}

static jwp_bool jwp_linux_create_timer(struct jwp_timer *timer)
{
	struct cavan_timer *cavan_timer;
	struct jwp_linux_desc *jwp_linux = (struct jwp_linux_desc *) timer->jwp;

#if JWP_DEBUG_MEMBER && JWP_LINUX_DEBUG
	println("create timer %s, msec = %d", timer->name, timer->msec);
#endif

	if (timer->handle == NULL)
	{
		cavan_timer = malloc(sizeof(struct cavan_timer));
		if (cavan_timer == NULL)
		{
			return false;
		}

		cavan_timer_init(cavan_timer, timer);
		cavan_timer->handler = jwp_linux_timer_handler;

		timer->handle = cavan_timer;
	}
	else
	{
		cavan_timer = timer->handle;
	}

	cavan_timer_insert(&jwp_linux->timer_service, cavan_timer, timer->msec);

	return true;
}

static void jwp_linux_delete_timer(struct jwp_timer *timer)
{
#if JWP_DEBUG_MEMBER && JWP_LINUX_DEBUG
	println("delete timer %s, msec = %d", timer->name, timer->msec);
#endif

	if (timer->handle != NULL)
	{
		struct jwp_linux_desc *jwp_linux = (struct jwp_linux_desc *) timer->jwp;

		cavan_timer_remove(&jwp_linux->timer_service, timer->handle);
	}
}
#endif

// ============================================================

static jwp_size_t jwp_linux_hw_read(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	pr_pos_info();

	msleep(2000);

	return 0;
}

static void jwp_linux_send_complete(struct jwp_desc *jwp)
{
#if JWP_LINUX_DEBUG
	pr_green_pos();
#endif
}

static void jwp_linux_remote_not_response(struct jwp_desc *jwp)
{
	pr_red_pos();
}

static void jwp_linux_data_received(struct jwp_desc *jwp, const void *data, jwp_size_t size)
{
	println("data received: size = %d", size);

#if JWP_RX_DATA_QUEUE_ENABLE
	jwp_queue_clear(jwp_get_queue(jwp, JWP_QUEUE_RX_DATA));
#endif
}

static void jwp_linux_command_received(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	((char *) command)[size] = 0;

	pr_green_info("command = %s", (char *) command);
}

static void jwp_linux_package_received(struct jwp_desc *jwp, const struct jwp_header *hdr)
{
	pr_pos_info();

	jwp_header_dump(hdr);
}

static void jwp_linux_log_received(struct jwp_desc *jwp, jwp_device_t device, const char *log, jwp_size_t size)
{
	print("%s: %s", jwp_device_to_string(device), log);
}

// ============================================================

#if JWP_TX_DATA_LOOP_ENABLE
static void *jwp_linux_tx_data_loop_thread(void *data)
{
	jwp_tx_data_loop(data);

	return NULL;
}
#endif

#if JWP_TX_LOOP_ENABLE
static void *jwp_linux_tx_loop_thread(void *data)
{
	jwp_tx_loop(data);

	return NULL;
}
#endif

#if JWP_RX_PKG_LOOP_ENABLE
static void *jwp_linux_rx_package_loop_thread(void *data)
{
	jwp_rx_package_loop(data);

	return NULL;
}
#endif

#if JWP_RX_LOOP_ENABLE
static void *jwp_linux_rx_loop_thread(void *data)
{
	jwp_rx_loop(data);

	return NULL;
}
#endif

jwp_bool jwp_linux_init(struct jwp_linux_desc *jwp_linux, void *data)
{
	struct jwp_desc *jwp = &jwp_linux->jwp;

#if JWP_TIMER_ENABLE
	jwp->create_timer = jwp_linux_create_timer;
	jwp->delete_timer = jwp_linux_delete_timer;
#endif

	return jwp_init(jwp, data);
}

jwp_bool jwp_linux_start(struct jwp_linux_desc *jwp_linux)
{
	pthread_t td;
	struct jwp_desc *jwp = &jwp_linux->jwp;

	if (jwp->hw_write == NULL)
	{
		pr_red_info("jwp->hw_write == NULL\n");
		return false;
	}

	if (jwp->hw_read == NULL)
	{
		jwp->hw_read = jwp_linux_hw_read;
	}

	if (jwp->data_received == NULL)
	{
		jwp->data_received = jwp_linux_data_received;
	}

	if (jwp->command_received == NULL)
	{
		jwp->command_received = jwp_linux_command_received;
	}

	if (jwp->package_received == NULL)
	{
		jwp->package_received = jwp_linux_package_received;
	}

	if (jwp->send_complete == NULL)
	{
		jwp->send_complete = jwp_linux_send_complete;
	}

	if (jwp->remote_not_response == NULL)
	{
		jwp->remote_not_response = jwp_linux_remote_not_response;
	}

#if JWP_WRITE_LOG_ENABLE
	if (jwp->log_received == NULL)
	{
		jwp->log_received = jwp_linux_log_received;
	}
#endif

#if JWP_TIMER_ENABLE
	if (cavan_timer_service_start(&jwp_linux->timer_service) < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return false;
	}
#endif

#if JWP_TX_DATA_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_linux_tx_data_loop_thread, jwp_linux);
#endif

#if JWP_TX_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_linux_tx_loop_thread, jwp_linux);
#endif

#if JWP_RX_PKG_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_linux_rx_package_loop_thread, jwp_linux);
#endif

#if JWP_RX_LOOP_ENABLE
	pthread_create(&td, NULL, jwp_linux_rx_loop_thread, jwp_linux);
#endif

	return true;
}
