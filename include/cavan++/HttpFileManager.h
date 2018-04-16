#pragma once

/*
 * File:		HttpFileManager.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-04-10 12:02:44
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
#include <cavan++/HttpService.h>

class HttpFileClient : public HttpClient {
public:
	HttpFileClient(NetworkClient *client) : HttpClient(client) {}
	virtual ~HttpFileClient() {}

protected:
	virtual int onHttpPacketReceived(HttpPacket *packet);
};

class HttpFileService : public HttpService {
protected:
	virtual EpollClient *newEpollClient(NetworkClient *client) {
		return new HttpFileClient(client);
	}
};
