#pragma once

/*
 * File:		http.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-02-13 17:52:12
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/network.h>
#include <cavan/service.h>

#define CAVAN_HTTP_PORT				80
#define CAVAN_HTTP_NAME				"CavanHttp"

typedef enum {
	HTTP_REQ_CONNECT,
	HTTP_REQ_DELETE,
	HTTP_REQ_GET,
	HTTP_REQ_HEAD,
	HTTP_REQ_OPTIONS,
	HTTP_REQ_PUT,
	HTTP_REQ_POST,
	HTTP_REQ_TRACE,
} http_request_type_t;

struct cavan_http_client {
	struct cavan_epoll_client epoll;
	struct network_client network;
};

struct cavan_http_service {
	struct network_service service;
	struct network_url url;
};

struct cavan_http_prop {
	const char *key;
	const char *value;
};

int cavan_http_get_request_type(const char *req, size_t length);
const char *cavan_http_request_type_tostring(int type);
char *cavan_http_find_prop(const char *req, const char *req_end, const char *name, size_t namelen);
char *cavan_http_set_prop(char *req, char *req_end, const char *name, int namelen, const char *value, int valuelen);
ssize_t cavan_http_read_request(struct network_client *client, char *buff, size_t size);
int cavan_http_parse_request(char *req, char *req_end, struct cavan_http_prop *props, int size);
void cavan_http_dump_prop(const struct cavan_http_prop *prop);
void cavan_http_dump_props(const struct cavan_http_prop *props, int count);
const struct cavan_http_prop *cavan_http_prop_find(const struct cavan_http_prop *props, int count, const char *key);
const char *cavan_http_prop_find_value(const struct cavan_http_prop *props, int count, const char *key);

int cavan_http_open_html_file(const char *title, char *pathname);
int cavan_http_flush_html_file(int fd);
int cavan_http_send_file_header(struct network_client *client, const char *service, const char *filetype, struct tm *time, size_t start, size_t length, size_t size);
int cavan_http_send_file(struct network_client *client, const char *service, int fd, const char *filetype, size_t start, size_t length);
int cavan_http_send_file2(struct network_client *client, const char *service, const char *pathname, const char *filetype, size_t start, size_t length);
int cavan_http_send_file3(struct network_client *client, const char *service, const char *pathname, const struct cavan_http_prop *props, int count);
int cavan_http_list_directory(struct network_client *client, const char *pathname);
int cavan_http_process_get(struct network_client *client, struct cavan_http_prop *props, int count, const char *pathname, size_t start, size_t length);

int cavan_http_service_run(struct cavan_dynamic_service *service);

static inline int cavan_http_send_html(struct network_client *client, const char *service, int fd)
{
	return cavan_http_send_file(client, service, fd, "text/html", 0, 0);
}
