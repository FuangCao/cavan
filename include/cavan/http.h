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
#ifdef CONFIG_CAVAN_CURL
#include <curl/curl.h>
#endif

#define CAVAN_HTTP_PORT				80

typedef enum {
	HTTP_REQ_CONNECT,
	HTTP_REQ_DELETE,
	HTTP_REQ_GET,
	HTTP_REQ_HEAD,
	HTTP_REQ_OPTIONS,
	HTTP_REQ_PUT,
	HTTP_REQ_POST,
	HTTP_REQ_TRACE,
	HTTP_REQ_PROPFIND,
} http_request_type_t;

struct cavan_http_service {
	struct network_service service;
	struct network_url url;
};

struct cavan_http_prop {
	const char *key;
	const char *value;
};

struct cavan_http_request {
	char *type;
	char *url;
	char *version;

	char *mem;
	size_t mem_size;
	size_t mem_used;

	struct cavan_http_prop *props;
	size_t prop_size;
	size_t prop_used;

	struct cavan_http_prop *params;
	size_t param_size;
	size_t param_used;
};

struct cavan_http_stream {
	char *buff;
	size_t size;
	size_t used;
};

void cavan_http_dump_prop(const struct cavan_http_prop *prop);
void cavan_http_dump_props(const struct cavan_http_prop *props, size_t size);
void cavan_http_dump_request(struct cavan_http_request *req);

struct cavan_http_request *cavan_http_request_alloc(size_t mem_size, size_t prop_size, size_t param_size);
void cavan_http_parse_prop(char *text, struct cavan_http_prop *prop);
void cavan_http_request_free(struct cavan_http_request *req);
void cavan_http_request_reset(struct cavan_http_request *req);
int cavan_http_read_props(struct cavan_fifo *fifo, struct cavan_http_request *req);
int cavan_http_read_request(struct cavan_fifo *fifo, struct cavan_http_request *req);

int cavan_http_get_request_type(const char *req, size_t length);
int cavan_http_get_request_type2(const char *type);
const char *cavan_http_request_type_tostring(int type);
size_t cavan_http_parse_request(char *req, char *req_end, struct cavan_http_prop *props, size_t size);
size_t cavan_http_parse_prop_value(char *value, struct cavan_http_prop *props, size_t size);
const struct cavan_http_prop *cavan_http_find_prop(const struct cavan_http_prop *props, size_t size, const char *key);
const char *cavan_http_find_prop_simple(const struct cavan_http_prop *props, size_t size, const char *key);
size_t cavan_http_parse_url_param(char *url, struct cavan_http_prop *props, size_t size);
size_t cavan_http_parse_url(char *url, struct cavan_http_prop *props, size_t size);
char *cavan_http_get_boundary(struct cavan_http_prop *props, size_t size);

int cavan_http_send_reply(struct network_client *client, int code, const char *format, ...);
int cavan_http_open_html_file(const char *title, char *pathname);
int cavan_http_flush_html_file(int fd);
int cavan_http_send_file_header(struct network_client *client, const char *filetype, struct tm *time, size_t start, size_t length, size_t size);
int cavan_http_send_file(struct network_client *client, int fd, const char *filetype, size_t start, size_t length);
int cavan_http_send_file2(struct network_client *client, const char *pathname, const char *filetype, size_t start, size_t length);
int cavan_http_send_file3(struct network_client *client, const char *pathname, const struct cavan_http_prop *props, size_t size);
int cavan_http_write_path_href(int fd, const char *pathname, size_t psize, const char *name, size_t nsize);
int cavan_http_write_path_href2(int fd, const char *pathname, const char *name);
int cavan_http_write_path_hrefs(int fd, const char *pathname);
int cavan_http_list_directory(struct network_client *client, const char *dirname, const char *filter);
int cavan_http_read_multiform_header(struct cavan_fifo *fifo, struct cavan_http_request *header, const char *boundary);
ssize_t cavan_http_receive_file(struct cavan_fifo *fifo, struct cavan_http_request *header, const char *dirname, const char *boundary);

int cavan_http_process_get(struct network_client *client, struct cavan_http_request *req);
int cavan_http_process_post(struct cavan_fifo *fifo, struct cavan_http_request *req);
int cavan_http_process_propfind(struct cavan_fifo *fifo, struct cavan_http_request *req);

int cavan_http_service_run(struct cavan_dynamic_service *service);

ssize_t http_client_send_request(const char *url, const char *post, const char *headers[], size_t header_size, void *rsp, size_t rsp_size);

// ================================================================================

static inline int cavan_http_send_html(struct network_client *client, int fd)
{
	return cavan_http_send_file(client, fd, "text/html", 0, 0);
}

static inline const struct cavan_http_prop *cavan_http_request_find_prop(struct cavan_http_request *req, const char *key)
{
	return cavan_http_find_prop(req->props, req->prop_used, key);
}

static inline const char *cavan_http_request_find_prop_simple(struct cavan_http_request *req, const char *key)
{
	return cavan_http_find_prop_simple(req->props, req->prop_used, key);
}

static inline const struct cavan_http_prop *cavan_http_request_find_param(struct cavan_http_request *req, const char *key)
{
	return cavan_http_find_prop(req->params, req->param_used, key);
}

static inline const char *cavan_http_request_find_param_simple(struct cavan_http_request *req, const char *key)
{
	return cavan_http_find_prop_simple(req->params, req->param_used, key);
}
