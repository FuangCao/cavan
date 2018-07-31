/*
 * File:		penetrator.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-31 10:32:06
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/penetrator.h>

int cavan_penetrate_service_init(struct cavan_penetrate_service *service)
{
	return 0;
}

void cavan_penetrate_service_deinit(struct cavan_penetrate_service *service)
{
}

int cavan_penetrate_client_init(struct cavan_penetrate_client *client)
{
	return 0;
}

void cavan_penetrate_client_deinit(struct cavan_penetrate_client *client)
{
}

int cavan_penetrate_proxy_init(struct cavan_penetrate_proxy *proxy)
{
	return 0;
}

void cavan_penetrate_proxy_deinit(struct cavan_penetrate_proxy *proxy)
{
}
