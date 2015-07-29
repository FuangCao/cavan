/*
 * File:		testMux.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-28 11:44:01
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
#include <cavan/mux.h>

static ssize_t test_mux_send(struct cavan_mux *mux, const void *buff, size_t size)
{
	int *pipefd = mux->private_data;

	return write(pipefd[1], buff, size);
}

static ssize_t test_mux_recv(struct cavan_mux *mux, void *buff, size_t size)
{
	int *pipefd = mux->private_data;

	return read(pipefd[0], buff, size);
}

static int test_mux_on_received(struct cavan_mux_link *link, const void *buff, size_t size)
{
	println("%s: port = %d, size = %ld, buff = %s", (char *) link->private_data, link->local_port, size, (char *) buff);

	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int ret;
	struct cavan_mux mux;
	struct cavan_mux_link link1, link2;
	struct cavan_mux_package *package, *packages[10];
	int pipefd[2];

	ret = pipe(pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		return ret;
	}

	mux.send = test_mux_send;
	mux.recv = test_mux_recv;
	ret = cavan_mux_init(&mux, pipefd);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_init");
		return ret;
	}

	cavan_mux_show_packages(&mux);

	for (i = 0; i < NELEM(packages); i++)
	{
		packages[i] = cavan_mux_package_alloc(&mux, (i + 1) * 100);
	}

	cavan_mux_show_packages(&mux);

	for (i = NELEM(packages) - 1; i >= 0; i--)
	{
		cavan_mux_package_free(&mux, packages[i]);
	}

	cavan_mux_show_packages(&mux);

	package = cavan_mux_package_alloc(&mux, 560);
	if (package)
	{
		println("alloc length = %d", package->length);
	}

	cavan_mux_show_packages(&mux);

	link1.on_received = link2.on_received = test_mux_on_received;

	link1.private_data = "Link1";
	ret = cavan_mux_bind(&mux, &link1, 0);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_bind");
	}

	link2.private_data = "Link2";
	ret = cavan_mux_bind(&mux, &link2, 0);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_bind");
	}

	println("port1 = %d, port2 = %d", link1.local_port, link2.local_port);
	link1.remote_port = link2.local_port;
	ret = cavan_mux_link_send_data(&link1, "1234567890", 10);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_link_send_data");
	}

	msleep(100);

	link2.remote_port = link1.local_port;
	ret = cavan_mux_link_send_data(&link2, "ABCDEFGHIJKL", 10);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_link_send_data");
	}

	msleep(5000);

	cavan_mux_deinit(&mux);

	return 0;
}
