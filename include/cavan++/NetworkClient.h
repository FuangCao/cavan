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
#include <cavan++/NetworkBase.h>
#include <cavan++/EpollClient.h>

class NetworkClient : public NetworkBase {
public:
	virtual ~NetworkClient() {}

public:
	virtual ssize_t sendraw(const void *buff, size_t size) {
		return ::send(mSockfd, buff, size, MSG_NOSIGNAL);
	}

	virtual ssize_t recvraw(void *buff, size_t size) {
		return ::recv(mSockfd, buff, size, MSG_NOSIGNAL);
	}

	virtual ssize_t sendto(const void *buff, size_t size, const struct sockaddr *addr, socklen_t addrlen) {
		return ::sendto(mSockfd, buff, size, MSG_NOSIGNAL, addr, addrlen);
	}

	virtual ssize_t recvfrom(void *buff, size_t size, struct sockaddr *addr, socklen_t *addrlen) {
		return ::recvfrom(mSockfd, buff, size, MSG_NOSIGNAL, addr, addrlen);
	}

	virtual ssize_t sendto(const void *buff, size_t size, const struct sockaddr_in *addr) {
		return ::sendto(mSockfd, buff, size, MSG_NOSIGNAL, (const struct sockaddr *) addr, sizeof(*addr));
	}

	virtual ssize_t recvfrom(void *buff, size_t size, struct sockaddr_in *addr) {
		socklen_t addrlen;
		return ::recvfrom(mSockfd, buff, size, MSG_NOSIGNAL, (struct sockaddr *) addr, &addrlen);
	}

	template <typename T> bool send(T value) {
		return (send(&value, sizeof(T)) == sizeof(T));
	}

	template <typename T> bool recv(T *value) {
		return (recv(value, sizeof(T)) == sizeof(T));
	}

public:
	virtual ssize_t send(const void *buff, size_t size);
	virtual ssize_t recv(void *buff, size_t size);
	virtual ssize_t fill(void *buff, size_t size);
	virtual void mask(uchar *buff, size_t size);
	virtual void mask(const uchar *src, uchar *dest, size_t size);
	virtual ssize_t sendPacked(const void *buff, size_t size);
	virtual ssize_t recvPacked(void *buff, size_t size);
	virtual ssize_t sendMasked(const void *buff, size_t size);
	virtual ssize_t recvMasked(void *buff, size_t size);
};

template <class T>
class NetworkEpollClient : public EpollClient {
private:
	NetworkClient &mClient;
	T mHeader;

public:
	NetworkEpollClient(NetworkClient &client) : mClient(client) {}
	virtual ~NetworkEpollClient() {};


};
