#pragma once

/*
 * File:		HttpService.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-04-09 15:18:00
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
#include <cavan++/HttpClient.h>
#include <cavan++/NetworkService.h>

class HttpService : public NetworkEpollService {
public:
	HttpService(NetworkService *service = NULL) : NetworkEpollService(service) {}
	virtual ~HttpService() {}
};
