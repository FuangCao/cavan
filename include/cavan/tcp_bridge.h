#pragma once

/*
 * File:		tcp_bridge.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-21 17:48:31
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

ssize_t cavan_tcp_bridge_copy(struct network_client *src, struct network_client *dest);
void cavan_tcp_bridge_main_loop(struct network_client *client1, struct network_client *client2);
int cavan_tcp_bridge_run(const char *url1, const char *url2);
