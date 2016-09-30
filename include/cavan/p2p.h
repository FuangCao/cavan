#pragma once

/*
 * File:		p2p.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-28 17:32:08
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

struct cavan_p2p_client {
	struct network_client client;
	struct sockaddr_in addr;
	u32 user_id;
	u32 message_id;

	struct cavan_p2p_client *prev;
	struct cavan_p2p_client *next;
};

struct cavan_p2p_service {
	struct network_service service;
	struct network_url url;
	u32 message_id;
};

struct cavan_p2p_message {
	u32 target_user_id;
	u32 source_user_id;
	u32 message_id;
};
