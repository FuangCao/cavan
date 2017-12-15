#pragma once

/*
 * File:		role_change.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-15 19:07:42
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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

struct role_change_client {
	struct network_client client;
	struct network_url url;
	struct network_url url_proxy;
};

struct role_change_service {
	struct network_service service;
	struct network_service proxy;
	struct network_url url;
	struct network_url url_proxy;
};

int role_change_service_run(struct cavan_dynamic_service *service);
