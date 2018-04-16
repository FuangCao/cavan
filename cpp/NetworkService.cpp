/*
 * File:		NetworkService.cpp
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
#include <cavan++/NetworkClient.h>
#include <cavan++/NetworkService.h>

int NetworkEpollService::onEpollIn(EpollService *service)
{
	NetworkClient *client;
	EpollClient *epoll;

	client = mService->accept();
	if (client == NULL) {
		return -EFAULT;
	}

	if (client->onConnected() < 0) {
		goto out_delete_client;
	}

	epoll = newEpollClient(client);
	if (epoll == NULL) {
		goto out_delete_client;
	}

	if (epoll->addEpollTo(this) < 0) {
		goto out_delete_epoll;
	}

	return 0;

out_delete_epoll:
	delete epoll;
out_delete_client:
	delete client;
	return -EFAULT;
}

int NetworkEpollService::open(NetworkUrl *url)
{
	NetworkService *service = url->openService();
	if (service == NULL) {
		return -EFAULT;
	}

	mService = service;

	return start();
}

int NetworkEpollService::open(const char *url_text)
{
	NetworkUrl url;

	if (url.parse(url_text)) {
		return open(&url);
	}

	return -EFAULT;
}
