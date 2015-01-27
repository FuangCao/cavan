#pragma once

/*
 * File:		jwp_udp.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-27 20:06:25
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

struct jwp_udp_service
{
	struct network_service service;
	struct cavan_timer_service timer_service;
	struct network_url url;
};

struct jwp_udp_client
{
	struct network_client client;
	struct jwp_desc jwp;
};

int jwp_udp_client_init(struct jwp_udp_client *client);
int jwp_udp_service_run(struct cavan_dynamic_service *service);
