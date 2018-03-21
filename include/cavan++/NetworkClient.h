#pragma once

/*
 * File:		NetworkClient.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:29:51
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
#include <sys/socket.h>

class NetworkClient {
private:
	int mSockfd;

public:
	NetworkClient(void) : mSockfd(INVALID_SOCKET) {}
	virtual ~NetworkClient() {}

	virtual int getSockfd(void) {
		return mSockfd;
	}

	virtual void setSockfd(int sockfd) {
		mSockfd = sockfd;
	}

public:
	virtual ssize_t sendRaw(const void *buff, size_t size) {
		return ::send(mSockfd, buff, size, MSG_NOSIGNAL);
	}

	virtual ssize_t recvRaw(void *buff, size_t size) {
		return ::recv(mSockfd, buff, size, MSG_NOSIGNAL);
	}

	template <typename T> bool send(T value) {
		return (send(&value, sizeof(T)) == sizeof(T));
	}

	template <typename T> bool recv(T *value) {
		return (recv(value, sizeof(T)) == sizeof(T));
	}

public:
	virtual bool connect(const NetworkUrl *url);
	virtual int openSocket(void);
	virtual void closeSocket(int sockfd);
	virtual ssize_t send(const void *buff, size_t size);
	virtual ssize_t recv(void *buff, size_t size);
	virtual ssize_t fill(void *buff, size_t size);
	virtual void mask(uchar *buff, size_t size);
	virtual void mask(const uchar *src, uchar *dest, size_t size);
	virtual ssize_t sendPacket(const void *buff, size_t size);
	virtual ssize_t recvPacket(void *buff, size_t size);
	virtual ssize_t sendMasked(const void *buff, size_t size);
	virtual ssize_t recvMasked(void *buff, size_t size);
};
