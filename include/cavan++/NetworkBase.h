#pragma once

/*
 * File:		NetworkBase.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-22 10:47:36
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
#include <netinet/in.h>
#include <arpa/inet.h>

class NetworkBase {
protected:
	int mSockfd;

public:
	NetworkBase(void) : mSockfd(INVALID_SOCKET) {}

	virtual ~NetworkBase() {
		if (mSockfd != INVALID_SOCKET) {
			closeSocket(mSockfd, true);
		}
	}

	virtual void setSockfd(int sockfd) {
		mSockfd = sockfd;
	}

	virtual int getSockfd(void) {
		return mSockfd;
	}

public:
	static int setSockOptInt(int sockfd, int level, int optname, int value) {
		return setsockopt(sockfd, level, optname, (void *) &value, sizeof(value));
	}

	static int setReuseAddr(int sockfd, int reuse) {
		return setSockOptInt(sockfd, SOL_SOCKET, SO_REUSEADDR, reuse);
	}

	static int setReusePort(int sockfd, int reuse) {
#ifdef SO_REUSEPORT
		return setSockOptInt(sockfd, SOL_SOCKET, SO_REUSEPORT, reuse);
#else
		return 0;
#endif
	}

public:
	int openSocket(int domain, int type) {
		return ::socket(domain, type, 0);
	}

	int openSocketInet(int type) {
		int sockfd = openSocket(PF_INET, type);
		if (sockfd < 0) {
			return sockfd;
		}

		setReuseAddr(sockfd, 1);

		return sockfd;
	}

	int openSocketTcp(void) {
		return openSocketInet(SOCK_STREAM);
	}

	int openSocketUdp(void) {
		return openSocketInet(SOCK_DGRAM);
	}

	void closeSocket(int sockfd, bool shutdown = false) {
		if (shutdown) {
			::shutdown(sockfd, SHUT_RDWR);
		}

		::close(sockfd);
	}

	int setReuseAddr(int reuse = 1) {
		return setReuseAddr(mSockfd, reuse);
	}

	int setReusePort(int reuse = 1) {
#ifdef SO_REUSEPORT
		return setReusePort(mSockfd, reuse);
#else
		return 0;
#endif
	}

	virtual bool setBlockEnable(bool enable);
	virtual int open(NetworkUrl *url) = 0;
	virtual void close(void) = 0;
};
