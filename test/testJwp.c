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
#include <cavan/timer.h>
#include <cavan/network.h>
#include <cavan/jwp-linux.h>
#include <cavan/jwp_comm.h>

#define TEST_JWP_DEBUG				0
#define TEST_JWP_MEM_DUMP			0
#define TEST_JWP_SHOW_DATA			1
#define TEST_JWP_CLIENT_WATCH		0
#define TEST_JWP_SERVER_WATCH		0

struct jwp_test_data
{
	int hw_fd;
	int data_fd;
	const char *pathname;
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

static void test_jwp_data_received(struct jwp_desc *jwp, const void *data, jwp_size_t size)
{
	struct jwp_test_data *test_data = jwp_get_private_data(jwp);

#if JWP_RX_DATA_QUEUE_ENABLE
#if 1
	while (1)
	{
		ssize_t rdlen, wrlen;
		char buff[JWP_MTU + 1];
		rdlen = jwp_recv_data(jwp, buff, sizeof(buff));
		if (rdlen == 0)
		{
			break;
		}

#if TEST_JWP_SHOW_DATA
		print_ntext(buff, rdlen);
#endif

		wrlen = write(test_data->data_fd, buff, rdlen);
		if (wrlen < 0)
		{
			pr_error_info("write");
		}
	}
#else
	char value;
	jwp_size_t rdlen;
	ssize_t wrlen;

	rdlen = jwp_recv_data(jwp, &value, 1);
	if (rdlen > 0)
	{
		print_ntext(&value, 1);

		wrlen = write(test_data->data_fd, &value, 1);
		if (wrlen < 0)
		{
			pr_error_info("write");
		}
	}

	msleep(1);
#endif
#else
	ssize_t wrlen;

#if TEST_JWP_SHOW_DATA
	print_ntext(data, size);
#endif

	wrlen = write(test_data->data_fd, data, size);
	if (wrlen < 0)
	{
		pr_error_info("write");
	}
#endif

#if TEST_JWP_SHOW_DATA == 0
	println("data received: size = %d", size);
#endif
}

#if TEST_JWP_CLIENT_WATCH || TEST_JWP_SERVER_WATCH
static void *test_jwp_watch_thread(void *data)
{
	struct jwp_linux_desc *jwp_linux = data;
	struct jwp_desc *jwp = &jwp_linux->jwp;
#if JWP_TIMER_ENABLE
	bool timer_fault;
	u32 run_count = 0;
	int fault_count = 0;
	struct cavan_timer_service *timer_service = &jwp_linux->timer_service;
#endif

	while (1)
	{
#if JWP_QUEUE_ENABLE
		int i;
		const struct jwp_queue *p;
#endif

		msleep(2000);

#if JWP_TIMER_ENABLE
		timer_fault = (run_count == timer_service->run_count);
		run_count = timer_service->run_count;
		if (timer_fault)
		{
			fault_count++;
		}
		else
		{
			fault_count = 0;
		}

		println("fault_count = %d", fault_count);
#endif

#if JWP_DEBUG_MEMBER
		println("jwp->line = %d", jwp->line);
#endif

#if JWP_QUEUE_ENABLE
		for (i = 0; i < NELEM(jwp->queues); i++)
		{
			p = jwp->queues + i;

#if JWP_DEBUG_MEMBER
			println("%s[%d]. used = %d, free = %d", p->name, i, jwp_queue_get_used_size(p), jwp_queue_get_free_size(p));
#else
			println("%d. used = %d, free = %d", i, jwp_queue_get_used_size(p), jwp_queue_get_free_size(p));
#endif
		}
#endif

#if JWP_TIMER_ENABLE
		if (timer_fault)
		{
#if JWP_DEBUG_MEMBER
			struct cavan_timer *timer;
			struct jwp_timer *jwp_timer;
			struct double_link_node *node;
			struct double_link *link = &timer_service->link;

			node = double_link_get_first_node(link);
			if (node)
			{
				timer = double_link_get_container(link, node);
				jwp_timer = timer->private_data;

				println("first timer = %s, msec = %d", jwp_timer->name, jwp_timer->msec);
			}

			timer = timer_service->timer_last_run;
			if (timer)
			{
				jwp_timer = timer->private_data;

				println("timer_last_run = %s, msec = %d", jwp_timer->name, jwp_timer->msec);
			}

			timer = timer_service->timer_running;
			if (timer)
			{
				jwp_timer = timer->private_data;

				println("timer_running = %s, msec = %d", jwp_timer->name, jwp_timer->msec);
			}

			timer = timer_service->timer_waiting;
			if (timer)
			{
				jwp_timer = timer->private_data;

				println("timer_waiting = %s, msec = %d", jwp_timer->name, jwp_timer->msec);
			}
#endif

			println("timer_service->thread.state = %d", timer_service->thread.state);
		}
#endif
	}

	return NULL;
}
#endif

static int test_jwp_run(int hw_fd, const char *pathname, bool service)
{
	int data_fd;
	struct jwp_test_data data;
	struct jwp_linux_desc jwp_linux;
	struct jwp_desc *jwp = &jwp_linux.jwp;

	jwp->hw_read = test_jwp_hw_read;
	jwp->hw_write = test_jwp_hw_write;
	jwp->data_received = test_jwp_data_received;
	jwp->send_complete = NULL;
	jwp->command_received = NULL;
	jwp->package_received = NULL;

	println("hw_fd = %d, service = %d, pathname = %s", hw_fd, service, pathname);

	if (!jwp_linux_init(&jwp_linux, &data))
	{
		pr_red_info("jwp_init");
		return -EFAULT;
	}

	data.hw_fd = hw_fd;

	if (!jwp_linux_start(&jwp_linux))
	{
		pr_red_info("jwp_linux_start");
		return -EFAULT;
	}

	if (service)
	{
#if TEST_JWP_SERVER_WATCH
		pthread_t td;
#endif

#if JWP_DEBUG_MEMBER
		jwp->name = "server";
#endif

#if TEST_JWP_SERVER_WATCH
		pthread_create(&td, NULL, test_jwp_watch_thread, &jwp_linux);
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
#if TEST_JWP_CLIENT_WATCH
		pthread_t td;
#endif
		char buff[1024];

#if JWP_DEBUG_MEMBER
		jwp->name = "client";
#endif

#if TEST_JWP_CLIENT_WATCH
		pthread_create(&td, NULL, test_jwp_watch_thread, &jwp_linux);
#endif

		data.data_fd = 0;

		if (pathname)
		{
			int i;

			data_fd = open(pathname, O_RDONLY);
			if (data_fd < 0)
			{
				pr_error_info("open file %s", pathname);
				return data_fd;
			}

			msleep(200);

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
				for (p = buff, p_end = p + rdlen; p < p_end; p += jwp_send_data(jwp, p, p_end - p));
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

			for (i = 0; i < 10; i++)
			{
				pr_green_info("send file complete");
				msleep(10);
			}
		}

		jwp_pr_value("100 = ", 100, 10);
		jwp_pr_value("0x1234 = ", 0x1234, 16);

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
							jwp_queue_get_used_size(p), jwp_queue_get_free_size(p));
				}
			}
#endif

			if (scanf("%s", buff) != 1)
			{
				pr_error_info("scanf");
				continue;
			}

#if 0
			println("send data %s", buff);
			if (jwp_send_data(jwp, buff, strlen(buff)), buff)
			{
				pr_green_info("send data %s complete", buff);
			}
			else
			{
				pr_red_info("Failed to jwp_send_data");
			}
#endif

			println("send command %s", buff);
			if (jwp_send_command(jwp, buff, strlen(buff)))
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

static int do_test_jwp(int argc, char *argv[])
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

#if 1
		test_jwp_run(pair[1], argc > 2 ? argv[2] : "/tmp/jwp.txt", true);
#else
		while (1)
		{
			pr_pos_info();

			msleep(2000);
		}
#endif
	}

	return 0;
}

static int do_test_queue(int argc, char *arv[])
{
#if JWP_QUEUE_ENABLE
	int i;
	int length;
	u8 buff[1024];
	struct jwp_queue queue;
	const char *text = "123456789";

	jwp_queue_init(&queue);

	for (i = 'A'; i < 'Z' - 3; i += 5)
	{
		u8 data[] = { i, i + 1, i + 2, i + 3, i + 4, 0 };

		length = jwp_queue_inqueue(&queue, data, sizeof(data) - 1);
		println("inqueue length = %d, used = %d, free = %d, data = %s",
				length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue), data);
		length = jwp_queue_dequeue(&queue, buff, sizeof(buff));
		buff[length] = 0;
		println("dequeue length = %d, used = %d, free = %d, buff = %s",
				length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue), buff);
		print_sep(60);
	}

	length = jwp_queue_seek(&queue, 10);
	println("seek length = %d, used = %d, free = %d",
			length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue));

	length = jwp_queue_skip(&queue, 1025);
	println("skip length = %d, used = %d, free = %d",
			length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue));

	length = jwp_queue_inqueue(&queue, (jwp_u8 *) text, strlen(text));
	println("inqueue length = %d, used = %d, free = %d, text = %s",
			length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue), text);

	length = jwp_queue_skip(&queue, 5);
	println("skip length = %d, used = %d, free = %d",
			length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue));

	length = jwp_queue_dequeue(&queue, buff, sizeof(buff));
	buff[length] = 0;
	println("dequeue length = %d, used = %d, free = %d, buff = %s",
			length, jwp_queue_get_used_size(&queue), jwp_queue_get_free_size(&queue), buff);
#else
	pr_red_info("jwp queue is not enabled");
#endif

	return 0;
}

static int do_test_comm(int argc, char *argv[])
{
	int ret;
	struct network_client client;
	struct jwp_comm_desc jwp_comm;

	ret = network_client_open2(&client, "unix-tcp:///tmp/COM1", 0);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = jwp_comm_init(&jwp_comm, client.sockfd);
	if (ret < 0)
	{
		pr_red_info("jwp_comm_init");
		goto out_network_client_close;
	}

	while (1)
	{
		pr_pos_info();

		msleep(2000);
	}

out_network_client_close:
	network_client_close(&client);

	return 0;
}

CAVAN_COMMAND_MAP_START
{ "jwp", do_test_jwp },
{ "queue", do_test_queue },
{ "comm", do_test_comm }
CAVAN_COMMAND_MAP_END
