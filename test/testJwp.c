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

struct jwp_test_data
{
	int fd_read;
	int fd_write;
	struct cavan_timer_service timer_service;
};

struct jwp_test_timer
{
	struct cavan_timer timer;
	void (*handler)(struct jwp_desc *desc, jwp_timer timer);
};

static jwp_size_t test_jwp_hw_read(struct jwp_desc *desc, void *buff, jwp_size_t size)
{
#if 0
	struct jwp_test_data *data = jwp_get_private_data(desc);

	return read(data->fd_read, buff, size);
#else
	pr_pos_info();

	return 0;
#endif
}

static jwp_size_t test_jwp_hw_write(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
#if 0
	struct jwp_test_data *data = jwp_get_private_data(desc);

	return write(data->fd_write, buff, size);
#else
	pr_pos_info();

	return size;
#endif
}

static void test_jwp_data_received(struct jwp_desc *desc, const void *data, jwp_size_t size)
{
	pr_pos_info();
}

static void test_jwp_package_received(struct jwp_desc *desc, struct jwp_package *pkg)
{
	pr_pos_info();

	jwp_package_dump(pkg);
}

static int test_jwp_timer_handler(struct cavan_timer *_timer, void *data)
{
	struct jwp_test_timer *timer = (struct jwp_test_timer *) _timer;

	pr_pos_info();

	timer->handler(_timer->private_data, (jwp_timer) timer);

	return 0;
}

static jwp_timer test_jwp_create_timer(struct jwp_desc *desc, jwp_timer _timer, jwp_time_t ms, void (*handler)(struct jwp_desc *desc, jwp_timer timer))
{
	struct jwp_test_data *data = jwp_get_private_data(desc);
	struct jwp_test_timer *timer;

	pr_pos_info();

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

int main(int argc, char *argv[])
{
	int ret;
	struct jwp_test_data data;
	struct jwp_desc desc =
	{
		.hw_read = test_jwp_hw_read,
		.hw_write = test_jwp_hw_write,
		.data_received = test_jwp_data_received,
		.package_received = test_jwp_package_received,
		.create_timer = test_jwp_create_timer,
		.delete_timer = test_jwp_delete_timer,
	};
	u8 buff[] = { 1, 2, 3, 4, 5, JWP_MAGIC_LOW, JWP_MAGIC_HIGH, JWP_PKG_DATA, 1, 2, 0, 'A', 'B', 'C', 'D' };

	ret = cavan_timer_service_start(&data.timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return ret;
	}

	if (jwp_init(&desc, &data) == false)
	{
		pr_red_info("jwp_init");
		return -EFAULT;
	}

	jwp_write_data(&desc, buff, sizeof(buff));
	jwp_send_package_sync(&desc, JWP_PKG_DATA, "12345", 5);

	while (1)
	{
		msleep(2000);
	}

	return 0;
}
