#pragma once

/*
 * File:		NetworkService.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:30:00
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
#include <cavan++/EpollService.h>
#include <cavan++/NetworkClient.h>

class NetworkService : public NetworkBase {
public:
	virtual ~NetworkService() {}

public:
	virtual int accept(struct sockaddr *addr, socklen_t *addrlen) {
		return ::accept(mSockfd, addr, addrlen);
	}

	virtual int accept(struct sockaddr_in *addr) {
		socklen_t addrlen;
		return accept((struct sockaddr *) addr, &addrlen);
	}

	virtual NetworkClient *accept(void) = 0;
};

class NetworkEpollService : public EpollService, public EpollClient {
private:
	NetworkService *mService;

public:
	NetworkEpollService(NetworkService *service) : mService(service) {}

protected:
	virtual EpollClient *newEpollClient(NetworkClient *client) = 0;
	virtual int onEpollIn(EpollService *service);

	virtual int getEpollFd(void) {
		return mService->getSockfd();
	}

	virtual int onEpollStarted(void) {
		return addEpollTo(this);
	}
};
