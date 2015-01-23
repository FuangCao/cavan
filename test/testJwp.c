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

struct jwp_test_data
{
	int dev_fd;
	int data_fd;
	const char *pathname;
	struct cavan_timer_service timer_service;
};

struct jwp_test_timer
{
	struct cavan_timer timer;
	void (*handler)(struct jwp_desc *desc, jwp_timer timer);
};

static jwp_size_t test_jwp_hw_read(struct jwp_desc *desc, void *buff, jwp_size_t size)
{
	struct jwp_test_data *data = jwp_get_private_data(desc);

	return read(data->dev_fd, buff, size);
}

static jwp_size_t test_jwp_hw_write(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
	struct jwp_test_data *data = jwp_get_private_data(desc);

	return write(data->dev_fd, buff, size);
}

static void test_jwp_send_complete(struct jwp_desc *desc)
{
	// pr_pos_info();
}

static void test_jwp_data_received(struct jwp_desc *desc, const void *data, jwp_size_t size)
{
	struct jwp_test_data *test_data = jwp_get_private_data(desc);

#if JWP_RX_DATA_QUEUE
	while (1)
	{
		ssize_t rdlen, wrlen;
		char buff[JWP_MTU + 1];
		rdlen = jwp_recv_data(desc, buff, sizeof(buff));
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

static void test_jwp_package_received(struct jwp_desc *desc, struct jwp_package *pkg)
{
	pr_pos_info();
	jwp_package_dump(pkg);
}

#if JWP_USE_TIMER
static int test_jwp_timer_handler(struct cavan_timer *_timer, void *data)
{
	struct jwp_test_timer *timer = (struct jwp_test_timer *) _timer;

	timer->handler(_timer->private_data, (jwp_timer) timer);

	return 0;
}

static jwp_timer test_jwp_create_timer(struct jwp_desc *desc, jwp_timer _timer, jwp_u32 ms, void (*handler)(struct jwp_desc *desc, jwp_timer timer))
{
	struct jwp_test_data *data = jwp_get_private_data(desc);
	struct jwp_test_timer *timer;

	if (_timer == JWP_TIMER_INVALID)
	{
		timer = malloc(sizeof(struct jwp_test_timer));
		cavan_timer_init(&timer->timer, desc);
	}
	else
	{
		timer = (struct jwp_test_timer *) _timer;
	}

	timer->timer.handler = test_jwp_timer_handler;
	timer->handler = handler;

	cavan_timer_insert(&data->timer_service, &timer->timer, ms);

	return (jwp_timer) timer;
}

static void test_jwp_delete_timer(struct jwp_desc *desc, jwp_timer _timer)
{
	struct jwp_test_data *data = jwp_get_private_data(desc);
	struct cavan_timer *timer = (struct cavan_timer *) _timer;

	cavan_timer_remove(&data->timer_service, timer);
}
#endif

static int test_jwp_receive_handler(struct cavan_thread *thread, void *data)
{
	u8 buff[1024];
	jwp_size_t size;
	struct jwp_desc *desc = data;

	size = desc->hw_read(desc, buff, sizeof(buff));

#if 0
	if (strcmp(thread->name, "service") == 0)
	{
		print_mem(buff, size);
	}
#endif

#if 0
	msleep(500);
#endif

	jwp_write_rx_data(desc, buff, size);

	return 0;
}

#if JWP_USE_TX_QUEUE
static int test_jwp_send_handler(struct cavan_thread *thread, void *data)
{
	struct jwp_desc *desc = data;

	if (jwp_process_tx_data(desc) == false)
	{
		msleep(1);
	}

	return 0;
}
#endif

static int test_jwp_run(int dev_fd, const char *pathname, bool service)
{
	int data_fd;
#if JWP_USE_TIMER
	int ret;
#endif
#if JWP_USE_TX_QUEUE
	struct cavan_thread thread_send;
#endif
	struct cavan_thread thread_recv;
	struct jwp_test_data data;
	struct jwp_desc desc =
	{
		.hw_read = test_jwp_hw_read,
		.hw_write = test_jwp_hw_write,
		.send_complete = test_jwp_send_complete,
		.data_received = test_jwp_data_received,
		.package_received = test_jwp_package_received,
#if JWP_USE_TIMER
		.create_timer = test_jwp_create_timer,
		.delete_timer = test_jwp_delete_timer,
#endif
	};

	println("dev_fd = %d, service = %d, pathname = %s", dev_fd, service, pathname);

#if JWP_USE_TIMER
	ret = cavan_timer_service_start(&data.timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return ret;
	}
#endif

	if (jwp_init(&desc, &data) == false)
	{
		pr_red_info("jwp_init");
		return -EFAULT;
	}

	data_fd = open(pathname, service ? O_WRONLY | O_CREAT | O_TRUNC: O_RDONLY, 0777);
	if (data_fd < 0)
	{
		pr_error_info("open file %s", pathname);
		return data_fd;
	}

	data.data_fd = data_fd;
	data.dev_fd = dev_fd;

#if JWP_USE_TX_QUEUE
	thread_send.name = service ? "service send" : "client send";
	thread_send.handler = test_jwp_send_handler;
	thread_send.wake_handker = NULL;
	cavan_thread_run(&thread_send, &desc);
#endif

	thread_recv.name = service ? "service recv" : "client recv";
	thread_recv.handler = test_jwp_receive_handler;
	thread_recv.wake_handker = NULL;
	cavan_thread_run(&thread_recv, &desc);

	if (service)
	{
		while (1)
		{
			msleep(2000);
		}
	}
	else
	{
		while (1)
		{
			ssize_t rdlen;
			char buff[1024];
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

			for (p = buff, p_end = p + rdlen; p < p_end; p++)
			{
				while (jwp_send_data(&desc, p, 1) != 1)
				{
					// pr_red_info("jwp_send_data");
					msleep(1);
				}
			}
		}

		while (1)
		{
			pr_pos_info();
			msleep(2000);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	pid_t pid;
	int pair[2];

	assert(argc > 2);

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

		test_jwp_run(pair[0], argv[1], false);
	}
	else
	{
		close(pair[0]);

		test_jwp_run(pair[1], argv[2], true);
	}

	return 0;
}
