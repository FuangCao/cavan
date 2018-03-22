/*
 * File:		im_server.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-22 11:54:10
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
#include <cavan++/NetworkProtocol.h>

int main(int argc, char *argv[])
{
	assert(argc > 1);

	NetworkService *service = NetworkUrl::openService(argv[1]);
	if (service == NULL) {
		return -EFAULT;
	}

	while (1) {
		println("accept");

		NetworkClient *client = service->accept();
		if (client == NULL) {
			break;
		}

		println("recv");

		while (1) {
			char buff[1024];
			int rdlen;

			rdlen = client->recv(buff, sizeof(buff));
			if (rdlen > 0) {
				buff[rdlen] = 0;
				println("buff[%d] = %s", rdlen, buff);
			} else {
				break;
			}
		}
	}

	return 0;
}
