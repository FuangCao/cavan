/*
 * File:		jwp_udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-27 20:06:43
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
#include <cavan/service.h>
#include <cavan/jwp_udp.h>

int main(int argc, char *argv[])
{
	int ret;
	struct network_url *url;
	struct cavan_dynamic_service *service;
	struct jwp_udp_service *jwp_udp;

	service = cavan_dynamic_service_create(sizeof(struct jwp_udp_service));
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 10;
	service->max = 1000;
	service->super_permission = 0;

	jwp_udp = cavan_dynamic_service_get_data(service);
	url = &jwp_udp->url;
	network_url_init(url, "udp", "any", 1234, CAVAN_NETWORK_SOCKET);

	ret = jwp_udp_service_run(service);
	cavan_dynamic_service_destroy(service);

	return ret;
}
