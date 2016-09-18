#pragma once

/*
 * File:		tcp_repeater.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 16:30:05
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/network.h>
#include <cavan/service.h>

#define CAVAN_TCP_REPEATER_PORT		8864

struct cavan_tcp_repeater_conn {
	struct network_client client;
	struct cavan_tcp_repeater_conn *prev;
	struct cavan_tcp_repeater_conn *next;
};

struct cavan_tcp_repeater {
	struct network_service service;
	struct network_url url;
	struct cavan_tcp_repeater_conn *head;
};

int cavan_tcp_repeater_run(struct cavan_dynamic_service *service);
