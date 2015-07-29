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

#define CAVAN_MUX_LINK_TABLE_MASK		0xFF

struct cavan_mux_link;

struct cavan_mux_package
{
	u16 src_port;
	u16 dest_port;
	u16 length;
	char data[0];
};

struct cavan_mux_package_raw
{
	u16 length;
	struct cavan_mux_package_raw *next;
	struct cavan_mux_package package;
};

struct cavan_mux
{
	struct cavan_lock lock;
	struct cavan_mux_package_raw *packages;

	void *private_data;
	struct cavan_thread recv_thread;
	struct cavan_mux_link *links[CAVAN_MUX_LINK_TABLE_MASK + 1];

	ssize_t (*send)(struct cavan_mux *mux, const void *buff, size_t size);
	ssize_t (*recv)(struct cavan_mux *mux, void *buff, size_t size);
};

struct cavan_mux_link
{
	u16 local_port;
	u16 remote_port;

	void *private_data;
	struct cavan_mux *mux;
	struct cavan_mux_link *next;

	int (*on_received)(struct cavan_mux_link *link, const void *buff, size_t size);
};

int cavan_mux_init(struct cavan_mux *mux);
void cavan_mux_deinit(struct cavan_mux *mux);
void cavan_mux_append_package(struct cavan_mux *mux, struct cavan_mux_package_raw *package);
struct cavan_mux_package_raw *cavan_mux_dequeue_package(struct cavan_mux *mux, size_t length);
void cavan_mux_show_packages(struct cavan_mux *mux);
struct cavan_mux_package *cavan_mux_package_alloc(struct cavan_mux *mux, size_t length);
void cavan_mux_package_free(struct cavan_mux *mux, struct cavan_mux_package *package);
ssize_t cavan_mux_link_send_data(struct cavan_mux_link *link, const void *buff, size_t size);
int cavan_mux_add_link(struct cavan_mux *mux, struct cavan_mux_link *link);
struct cavan_mux_link *cavan_mux_find_link(struct cavan_mux *mux, u16 port);
void cavan_mux_remove_link(struct cavan_mux *mux, struct cavan_mux_link *link);
u16 cavan_mux_alloc_port(struct cavan_mux *mux);
int cavan_mux_recv_package(struct cavan_mux *mux, const struct cavan_mux_package *package);

static inline size_t cavan_mux_package_get_whole_length(const struct cavan_mux_package *package)
{
	return sizeof(struct cavan_mux_package) + package->length;
}
