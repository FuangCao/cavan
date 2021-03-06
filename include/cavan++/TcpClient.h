#pragma once

/*
 * File:		TcpClient.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:31:07
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
#include <cavan++/NetworkClient.h>

class TcpClient : public NetworkClient {
public:
	virtual ~TcpClient() {}

public:
	virtual int open(NetworkUrl *url);

	virtual void close(void) {
		if (mSockfd != INVALID_SOCKET) {
			closeSocket(mSockfd, true);
			mSockfd = INVALID_SOCKET;
		}
	}
};

class TcpClientPacked : public TcpClient {
public:
	virtual ssize_t send(const void *buff, size_t size) {
		return sendPacked(buff, size);
	}

	virtual ssize_t recv(void *buff, size_t size) {
		return recvPacked(buff, size);
	}
};

class TcpClientMasked : public TcpClient {
public:
	virtual ssize_t send(const void *buff, size_t size) {
		return sendMasked(buff, size);
	}

	virtual ssize_t recv(void *buff, size_t size) {
		return recvMasked(buff, size);
	}
};

