#define CAVAN_CMD_NAME service_manager

/*
 * File:		service_manager.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-05-03 17:26:58
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
#include <cavan/command.h>
#include <cavan/service.h>

static void service_list_handler(struct cavan_dynamic_service *service, void *data)
{
	pd_info("%s", service->name);
}

static int do_service_list(int argc, char *argv[])
{
	cavan_dynamic_service_scan(NULL, service_list_handler);

	return 0;
}

static int do_service_stop(int argc, char *argv[])
{
	if (argc < 2) {
		pd_err_info("Please give service name");
		return -EINVAL;
	}

	if (cavan_dynamic_service_stop_by_name(argv[1])) {
		pd_green_info("OK");
	} else {
		pd_err_info("Failed");
		return -EFAULT;
	}

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "list", do_service_list },
	{ "stop", do_service_stop },
} CAVAN_COMMAND_MAP_END;
