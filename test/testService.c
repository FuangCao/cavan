/*
 * File:		testService.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-09-30 20:48:15
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
#include <cavan/service.h>

struct test_service_data
{
	int data;
};

static void *test_open_connect(struct cavan_dynamic_service *service)
{
	pr_pos_info();

	return malloc(sizeof(struct test_service_data));
}

static void test_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	pr_pos_info();

	free(conn);
}

static int test_service_handler(struct cavan_dynamic_service *service, void *conn)
{
	pr_pos_info();

	msleep(500);

	pr_pos_info();

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	struct cavan_dynamic_service *service;

	service = cavan_dynamic_service_create(0);
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -EFAULT;
	}

	service->name = "TEST";
	service->min = 2;
	service->max = 10;
	service->open_connect = test_open_connect;
	service->close_connect = test_close_connect;
	service->service_handler = test_service_handler;

	pr_pos_info();

	ret = cavan_dynamic_service_run(service);
	if (ret < 0)
	{
		pr_red_info("cavan_dynamic_service_run");
	}
	pr_pos_info();

	cavan_dynamic_service_destroy(service);
	pr_pos_info();

	return ret;
}
