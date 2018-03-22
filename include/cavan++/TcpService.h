#pragma once

/*
 * File:		TcpService.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:31:13
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
#include <cavan++/TcpClient.h>
#include <cavan++/NetworkService.h>

class TcpService : public NetworkService {
public:
	virtual ~TcpService() {}

public:
	virtual int open(NetworkUrl *url);
	virtual NetworkClient *accept(void);

	virtual TcpClient *newTcpClient(void) {
		return new TcpClient();
	}

	virtual void close(void) {
		if (mSockfd != INVALID_SOCKET) {
			closeSocket(mSockfd, false);
			mSockfd = INVALID_SOCKET;
		}
	}
};

class TcpServicePacked : public TcpService {
public:
	virtual NetworkClient *accept(void);
};

class TcpServiceMasked : public TcpService {
public:
	virtual NetworkClient *accept(void);
};
