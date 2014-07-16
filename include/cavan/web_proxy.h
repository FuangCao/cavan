#pragma once

/*
 * File:		web_proxy.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-10-04 12:45:41
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/service.h>

#define CAVAN_WEB_PROXY_PORT	9090

typedef enum
{
	HTTP_REQ_CONNECT,
	HTTP_REQ_DELETE,
	HTTP_REQ_GET,
	HTTP_REQ_HEAD,
	HTTP_REQ_OPTIONS,
	HTTP_REQ_PUT,
	HTTP_REQ_POST,
	HTTP_REQ_TRACE,
} http_request_type_t;

struct web_proxy_service
{
	struct network_service service;
	struct network_url url;
	struct network_url url_proxy;
	size_t proxy_hostlen;
};

const char *web_proxy_request_type_tostring(int type);
int web_proxy_get_request_type(const char *req, size_t length);
int web_proxy_service_run(struct cavan_dynamic_service *service);
