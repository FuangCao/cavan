/*
 * File:			testJwp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-22 10:12:11
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
#include <cavan/jwp.h>
#include <cavan/timer.h>
#include <cavan/network.h>

#define TEST_JWP_DEBUG		0
#define TEST_JWP_MEM_DUMP	1

struct jwp_test_data
{
	int hw_fd;
	int data_fd;
	const char *pathname;
	struct cavan_timer_service timer_service;
};

static jwp_size_t test_jwp_hw_read(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	ssize_t rdlen;
	struct jwp_test_data *data = jwp_get_private_data(jwp);

	rdlen = read(data->hw_fd, buff, size);
	if (rdlen < 0)
	{
		pr_error_info("read");
		return 0;
	}

#if TEST_JWP_MEM_DUMP
#if JWP_DEBUG
	print("%s read >> ", jwp->name);
#endif
	print_mem(buff, rdlen);
#endif

	return rdlen;
}

static jwp_size_t test_jwp_hw_write(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	ssize_t wrlen;
	struct jwp_test_data *data = jwp_get_private_data(jwp);

#if TEST_JWP_MEM_DUMP
#if JWP_DEBUG
	print("%s write << ", jwp->name);
#endif
	print_mem(buff, size);
#endif

	wrlen = write(data->hw_fd, buff, size);
	if (wrlen < 0)
	{
		pr_error_info("write");
		return 0;
	}

	return wrlen;
}

static void test_jwp_send_complete(struct jwp_desc *jwp)
{
#if TEST_JWP_DEBUG
	pr_pos_info();
#endif
}

static void test_jwp_data_received(struct jwp_desc *jwp, const void *data, jwp_size_t size)
{
	struct jwp_test_data *test_data = jwp_get_private_data(jwp);

#if JWP_RX_DATA_QUEUE_ENABLE
	while (1)
	{
		ssize_t rdlen, wrlen;
		char buff[JWP_MTU + 1];
		rdlen = jwp_recv_data(jwp, buff, sizeof(buff));
		if (rdlen == 0)
		{
			break;
		}

		print_ntext(buff, rdlen);

		wrlen = write(test_data->data_fd, buff, rdlen);
		if (wrlen < 0)
		{
			pr_error_info("write");
		}
	}
#else
	ssize_t wrlen;

	print_ntext(data, size);

	wrlen = write(test_data->data_fd, data, size);
	if (wrlen < 0)
	{
		pr_error_info("write");
	}
#endif
}

static void test_jwp_command_received(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	char *text = (char *) command;

	text[size] = 0;
	pr_green_info("command = %s", text);
}

static void test_jwp_package_received(struct jwp_desc *jwp, const struct jwp_header *hdr)
{
	pr_pos_info();
	jwp_header_dump(hdr);
}

static int test_jwp_timer_handler(struct cavan_timer *timer, void *data)
{
	struct jwp_timer *jwp_timer = data;

#if JWP_DEBUG
	println("run timer %s, msec = %d", jwp_timer->name, jwp_timer->msec);
#endif

	jwp_timer_run(jwp_timer);

	return 0;
}

static jwp_bool test_jwp_create_timer(struct jwp_timer *timer)
{
	struct cavan_timer *cavan_timer;
	struct jwp_test_data *data = jwp_get_private_data(timer->jwp);

#if JWP_DEBUG
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
		cavan_timer->handler = test_jwp_timer_handler;

		timer->handle = cavan_timer;
	}
	else
	{
		cavan_timer = timer->handle;
	}

	cavan_timer_insert(&data->timer_service, cavan_timer, timer->msec);

	return true;
}

static void test_jwp_delete_timer(struct jwp_timer *timer)
{
#if JWP_DEBUG
	println("delete timer %s, msec = %d", timer->name, timer->msec);
#endif

	if (timer->handle != NULL)
	{
		struct jwp_test_data *data = jwp_get_private_data(timer->jwp);

		cavan_timer_remove(&data->timer_service, timer->handle);
	}
}

#if JWP_TX_DATA_LOOP_ENABLE
static void *test_jwp_tx_data_loop_thread(void *data)
{
	jwp_tx_data_loop(data);

	return NULL;
}
#endif

#if JWP_TX_LOOP_ENABLE
static void *test_jwp_tx_loop_thread(void *data)
{
	jwp_tx_loop(data);

	return NULL;
}
#endif

#if JWP_RX_LOOP_ENABLE
static void *test_jwp_rx_loop_thread(void *data)
{
	jwp_rx_loop(data);

	return NULL;
}
#endif

#if JWP_RX_PKG_LOOP_ENABLE
static void *test_jwp_rx_package_loop_thread(void *data)
{
	jwp_rx_package_loop(data);

	return NULL;
}
#endif


static int test_jwp_run(int hw_fd, const char *pathname, bool service)
{
	int data_fd;
#if JWP_TX_TIMER_ENABLE
	int ret;
#endif
	pthread_t td;
	struct jwp_test_data data;
	struct jwp_desc jwp =
	{
		.hw_read = test_jwp_hw_read,
		.hw_write = test_jwp_hw_write,
		.send_complete = test_jwp_send_complete,
		.data_received = test_jwp_data_received,
		.command_received = test_jwp_command_received,
		.package_received = test_jwp_package_received,
		.create_timer = test_jwp_create_timer,
		.delete_timer = test_jwp_delete_timer,
	};

	println("hw_fd = %d, service = %d, pathname = %s", hw_fd, service, pathname);

#if JWP_TX_TIMER_ENABLE
	ret = cavan_timer_service_start(&data.timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return ret;
	}
#endif

	if (jwp_init(&jwp, &data) == false)
	{
		pr_red_info("jwp_init");
		return -EFAULT;
	}

	data.hw_fd = hw_fd;

#if JWP_RX_LOOP_ENABLE
	pthread_create(&td, NULL, test_jwp_rx_loop_thread, &jwp);
#endif

#if JWP_TX_DATA_LOOP_ENABLE
	pthread_create(&td, NULL, test_jwp_tx_data_loop_thread, &jwp);
#endif

#if JWP_TX_LOOP_ENABLE
	pthread_create(&td, NULL, test_jwp_tx_loop_thread, &jwp);
#endif

#if JWP_RX_PKG_LOOP_ENABLE
	pthread_create(&td, NULL, test_jwp_rx_package_loop_thread, &jwp);
#endif

	if (service)
	{
#if JWP_DEBUG
		jwp.name = "server";
#endif

		data_fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (data_fd < 0)
		{
			pr_error_info("open file %s", pathname);
			return data_fd;
		}

		data.data_fd = data_fd;

		while (1)
		{
			msleep(2000);
		}
	}
	else
	{
		char buff[1024];

#if JWP_DEBUG
		jwp.name = "client";
#endif

		data.data_fd = 0;

		if (pathname)
		{
			data_fd = open(pathname, O_RDONLY);
			if (data_fd < 0)
			{
				pr_error_info("open file %s", pathname);
				return data_fd;
			}

			while (1)
			{
				ssize_t rdlen;
				char *p, *p_end;

				rdlen = read(data_fd, buff, sizeof(buff));
				if (rdlen <= 0)
				{
					if (rdlen < 0)
					{
						pr_error_info("read")
					}

					break;
				}

#if 1
				for (p = buff, p_end = p + rdlen; p < p_end; p += jwp_send_data(&jwp, p, p_end - p));
#else
				for (p = buff, p_end = p + rdlen; p < p_end; p++)
				{
					while (jwp_send_data(&jwp, p, 1) != 1)
					{
#if TEST_JWP_DEBUG
						pr_red_info("jwp_send_data");
#endif
						msleep(1);
					}
				}
#endif
			}

			close(data_fd);

			pr_green_info("send file complete");
		}

		println("please input command");

		while (1)
		{
			char buff[1024];

#if TEST_JWP_DEBUG
			if (JWP_QUEUE_COUNT > 0)
			{
				struct jwp_queue *p, *p_end;

				for (p = jwp.queues, p_end = p + JWP_QUEUE_COUNT; p < p_end; p++)
				{
					println("%d. fill = %d, free = %d", p - jwp.queues,
							jwp_queue_get_fill_size(p), jwp_queue_get_free_size(p));
				}
			}
#endif

			if (scanf("%s", buff) != 1)
			{
				pr_error_info("scanf");
				continue;
			}

			println("send data %s", buff);
			if (jwp_send_data(&jwp, buff, strlen(buff)), buff)
			{
				pr_green_info("send data %s complete", buff);
			}
			else
			{
				pr_red_info("Failed to jwp_send_data");
			}

			println("send command %s", buff);
			if (jwp_send_command(&jwp, buff, strlen(buff)))
			{
				pr_green_info("send command %s complete", buff);
			}
			else
			{
				pr_red_info("Failed to jwp_send_command");
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	pid_t pid;
	int pair[2];

	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
	if (ret < 0)
	{
		pr_error_info("socketpair");
		return ret;
	}

	pid = fork();
	if (pid == 0)
	{
		close(pair[1]);

		test_jwp_run(pair[0], argc > 1 ? argv[1] : NULL, false);
	}
	else
	{
		close(pair[0]);

		test_jwp_run(pair[1], argc > 2 ? argv[2] : "/tmp/jwp.txt", true);
	}

	return 0;
}
