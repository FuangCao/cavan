#define CAVAN_CMD_NAME im_client

/*
 * File:		im_client.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-22 11:54:16
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
#include <cavan++/NetworkUrl.h>
#include <cavan++/NetworkClient.h>

int main(int argc, char *argv[])
{
	assert(argc > 1);

	NetworkClient *client = NetworkUrl::openClient(argv[1]);
	if (client == NULL) {
		pr_red_info("openClient");
		return -EFAULT;
	}

 	while (client->send("123456", 6) > 0) {
		pr_pos_info();
		msleep(2000);
	}

	client->close();

	return 0;
}
