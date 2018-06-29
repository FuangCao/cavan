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
#include <cavan++/EpollClient.h>
#include <cavan++/EpollService.h>
#include <cavan++/NetworkProtocol.h>

class TestEpollClient : public NetworkEpollClient<EpollBufferU16> {
public:
	TestEpollClient(EpollService *service, NetworkClient *client) : NetworkEpollClient(service, client) {}

protected:
	virtual int onEpollPackReceived(char *buff, u16 size) {
		EpollPacket *writer = new EpollPacket(size + 2);
		writer->write(&size, sizeof(size));
		writer->write(buff, size);
		sendEpollPacket(writer);
		return 0;
	}
};

class TestEpollService : public NetworkEpollService {
public:
	TestEpollService(NetworkService *service) : NetworkEpollService(service) {}

protected:
	EpollClient *newEpollClient(NetworkClient *client) {
		return new TestEpollClient(this, client);
	}
};

int main(int argc, char *argv[])
{
	assert(argc > 1);

	NetworkService *service = NetworkUrl::openService(argv[1]);
	if (service == NULL) {
		return -EFAULT;
	}

	TestEpollService epoll(service);
	epoll.start();
	epoll.join();

	return 0;
}
