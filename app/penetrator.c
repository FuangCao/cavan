/*
 * File:		penetrator.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-31 10:32:29
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
#include <cavan/command.h>
#include <cavan/penetrator.h>

static int cavan_penetrate_service_main(int argc, char *argv[])
{
	pr_pos_info();
	return 0;
}

static int cavan_penetrate_client_main(int argc, char *argv[])
{
	pr_pos_info();
	return 0;
}

static int cavan_penetrate_proxy_main(int argc, char *argv[])
{
	pr_pos_info();
	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "service", cavan_penetrate_service_main },
	{ "client", cavan_penetrate_client_main },
	{ "proxy", cavan_penetrate_proxy_main },
} CAVAN_COMMAND_MAP_END;
