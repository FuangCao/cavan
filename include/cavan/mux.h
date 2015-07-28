#pragma once

/*
 * File:		mux.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-28 11:43:37
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
#include <cavan/thread.h>

struct cavan_mux_package
{
	union
	{
		struct
		{
			u32 src_port;
			u32 dest_port;
		};

		struct cavan_mux_package *next;
	};

	u32 length;
	char data[0];
};

struct cavan_mux
{
	u32 local_port;
	u32 remote_port;
	void *link_data;

	struct cavan_lock lock;
	struct cavan_mux_package *packages;

	ssize_t (*send)(struct cavan_mux *mux, const void *buff, size_t size);
	ssize_t (*recv)(struct cavan_mux *mux, void *buff, size_t size);
};

int cavan_mux_init(struct cavan_mux *mux);
void cavan_mux_deinit(struct cavan_mux *mux);
void cavan_mux_append_package(struct cavan_mux *mux, struct cavan_mux_package *package);
struct cavan_mux_package *cavan_mux_dequeue_package(struct cavan_mux *mux, size_t length);
void cavan_mux_show_packages(struct cavan_mux *mux);
struct cavan_mux_package *cavan_mux_package_alloc(struct cavan_mux *mux, size_t length);
void cavan_mux_package_free(struct cavan_mux *mux, struct cavan_mux_package *package);
