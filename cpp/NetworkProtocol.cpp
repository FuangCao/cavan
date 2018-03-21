/*
 * File:		NetworkProtocol.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 11:23:19
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
#include <cavan++/TcpClient.h>
#include <cavan++/TcpService.h>

class NetworkProtocolTcp : public NetworkProtocol {
public:
	virtual const char *getName(void) {
		return "tcp";
	}

	virtual NetworkClient *newClient(void) {
		return new TcpClient();
	}

	virtual NetworkService *newService(void) {
		return new TcpService();
	}
};
