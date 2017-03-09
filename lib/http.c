/*
 * File:		http.c
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
#include <cavan/http.h>
#include <cavan/web_proxy.h>

#define CAVAN_HTTP_DEBUG	1

void cavan_http_dump_prop(const struct cavan_http_prop *prop)
{
	println("key = %s, value = %s", prop->key, prop->value);
}

void cavan_http_dump_props(const struct cavan_http_prop *props, size_t propc)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + propc; props < prop_end; props++) {
		cavan_http_dump_prop(props);
	}
}

void cavan_http_request_dump(struct cavan_http_request *req)
{
	println("type = %s", req->type);
	println("url = %s", req->url);
	println("version = %s", req->version);
	println("props[%" PRINT_FORMAT_SIZE "/%" PRINT_FORMAT_SIZE "]:", req->prop_used, req->prop_size);
	cavan_http_dump_props(req->props, req->prop_used);
	println("params[%" PRINT_FORMAT_SIZE "/%" PRINT_FORMAT_SIZE "]:", req->param_used, req->param_size);
	cavan_http_dump_props(req->params, req->param_used);
}

struct cavan_http_request *cavan_http_request_alloc(size_t mem_size, size_t prop_size, size_t param_size)
{
	struct cavan_http_request *req = malloc(sizeof(struct cavan_http_request) + sizeof(struct cavan_http_prop) * (prop_size + param_size) + mem_size);
	if (req == NULL) {
		return NULL;
	}

	req->mem = (char *) (req + 1);
	req->mem_size = mem_size;
	req->mem_used = 0;

	req->props = (struct cavan_http_prop *) req->mem + mem_size;
	req->prop_size = prop_size;
	req->prop_used = 0;

	req->params = req->props + prop_size;
	req->param_size = param_size;
	req->param_used = 0;

	return req;
}

void cavan_http_request_free(struct cavan_http_request *req)
{
	free(req);
}

int cavan_http_request_get_type(const char *req, size_t length)
{
	switch (req[0]) {
	case 'C':
		if (length == 7 && text_lhcmp("ONNECT", req + 1) == 0) {
			return HTTP_REQ_CONNECT;
		}
		break;

	case 'D':
		if (length == 6 && text_lhcmp("ELETE", req + 1) == 0) {
			return HTTP_REQ_DELETE;
		}
		break;

	case 'G':
		if (length == 3 && text_lhcmp("ET", req + 1) == 0) {
			return HTTP_REQ_GET;
		}
		break;

	case 'H':
		if (length == 4 && text_lhcmp("EAD", req + 1) == 0) {
			return HTTP_REQ_HEAD;
		}
		break;

	case 'O':
		if (length == 7 && text_lhcmp("PTIONS", req + 1) == 0) {
			return HTTP_REQ_OPTIONS;
		}
		break;

	case 'P':
		if (length == 3 && text_lhcmp("UT", req + 1) == 0) {
			return HTTP_REQ_PUT;
		} else if (length == 4 && text_lhcmp("OST", req + 1) == 0) {
			return HTTP_REQ_POST;
		}
		break;

	case 'T':
		if (length == 5 && text_lhcmp("RACE", req + 1) == 0) {
			return HTTP_REQ_TRACE;
		}
		break;
	}

	return -EINVAL;
}

int cavan_http_request_get_type2(const char *type)
{
	switch (type[0]) {
	case 'C':
		if (strcmp("ONNECT", type + 1) == 0) {
			return HTTP_REQ_CONNECT;
		}
		break;

	case 'D':
		if (strcmp("ELETE", type + 1) == 0) {
			return HTTP_REQ_DELETE;
		}
		break;

	case 'G':
		if (strcmp("ET", type + 1) == 0) {
			return HTTP_REQ_GET;
		}
		break;

	case 'H':
		if (strcmp("EAD", type + 1) == 0) {
			return HTTP_REQ_HEAD;
		}
		break;

	case 'O':
		if (strcmp("PTIONS", type + 1) == 0) {
			return HTTP_REQ_OPTIONS;
		}
		break;

	case 'P':
		if (strcmp("UT", type + 1) == 0) {
			return HTTP_REQ_PUT;
		} else if (strcmp("OST", type + 1) == 0) {
			return HTTP_REQ_POST;
		}
		break;

	case 'T':
		if (strcmp("RACE", type + 1) == 0) {
			return HTTP_REQ_TRACE;
		}
		break;
	}

	return -EINVAL;
}

const char *cavan_http_request_type_tostring(int type)
{
	switch (type) {
	case HTTP_REQ_CONNECT:
		return "CONNECT";
	case HTTP_REQ_DELETE:
		return "DELETE";
	case HTTP_REQ_GET:
		return "GET";
	case HTTP_REQ_HEAD:
		return "HEAD";
	case HTTP_REQ_OPTIONS:
		return "OPTIONS";
	case HTTP_REQ_PUT:
		return "PUT";
	case HTTP_REQ_POST:
		return "POST";
	case HTTP_REQ_TRACE:
		return "TRACE";
	default:
		return "UNKNOWN";
	}
}

size_t cavan_http_parse_request(char *req, char *req_end, struct cavan_http_prop *props, size_t propc)
{
	struct cavan_http_prop *prop, *prop_end;

	if (propc == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + propc;

	for (prop->key = req, prop->value = NULL; req < req_end; req++) {
		switch (*req) {
		case ' ':
		case '\t':
			if (prop->key == req) {
				prop->key++;
			} else if (prop->value == req) {
				prop->value++;
			}
			break;

		case '\r':
		case '\n':
			*req = 0;

			if (prop->value) {
				if (++prop < prop_end) {
					prop->value = NULL;
				} else {
					goto out_complete;
				}
			}

			prop->key = req + 1;
			break;

		case ':':
			if (prop->value) {
				break;
			}

			*req = 0;
			prop->value = req + 1;
			text_clear_space_invert(prop->key, req - 1);
			break;
		}
	}

out_complete:
	return prop - props;
}

size_t cavan_http_parse_prop_value(char *value, struct cavan_http_prop *props, size_t propc)
{
	struct cavan_http_prop *prop, *prop_end;

	if (propc == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + propc;

	prop->key = value;
	prop->value = NULL;

	while (1) {
		switch (*value) {
		case 0:
			if (value > prop->key) {
				text_clear_space_invert(prop->key, value - 1);
				prop++;
			}
			goto out_complete;

		case ';':
			*value = 0;

			if (value > prop->key) {
				text_clear_space_invert(prop->key, value - 1);

				if (++prop < prop_end) {
					prop->key = value + 1;
				} else {
					goto out_complete;
				}
			}
			break;

		case '=':
			*value = 0;
			prop->value = value + 1;
			break;

		case ' ':
		case '\t':
			if (prop->key == value) {
				prop->key++;
			} else if (prop->value == value) {
				prop->value++;
			}
			break;
		}

		value++;
	}

out_complete:
	return prop - props;
}

const struct cavan_http_prop *cavan_http_prop_find(const struct cavan_http_prop *props, size_t propc, const char *key)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + propc; props < prop_end; props++) {
		if (strcmp(props->key, key) == 0) {
			return props;
		}
	}

	return NULL;
}

const char *cavan_http_prop_find_value(const struct cavan_http_prop *props, size_t propc, const char *key)
{
	const struct cavan_http_prop *prop = cavan_http_prop_find(props, propc, key);

	if (prop == NULL) {
		return NULL;
	}

	return prop->value;
}

size_t cavan_http_parse_url_param(char *text, struct cavan_http_prop *props, size_t propc)
{
	char *p;
	struct cavan_http_prop *prop, *prop_end;

	if (propc == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + propc;

	p = text;
	prop->key = text;
	prop->value = NULL;

	while (1) {
		switch (*p) {
		case '=':
			*text = 0;
			prop->value = text + 1;
			break;

		case '&':
			*text = 0;
			if (prop->value) {
				if (++prop < prop_end) {
					prop->key = text + 1;
					prop->value = NULL;
				} else {
					goto out_complete;
				}
			}
			break;

		case 0:
			*text = 0;
			if (prop->value) {
				prop++;
			}
			goto out_complete;

		case '%':
			*text = text2byte(p + 1);
			p += 2;
			break;

		default:
			*text = *p;
		}

		text++;
		p++;
	}

out_complete:
	return prop - props;
}

size_t cavan_http_parse_url(char *url, struct cavan_http_prop *props, size_t size)
{
	size_t count = 0;
	char *p = url;

	while (1) {
		switch (*p) {
		case '%':
			*url = text2byte(p + 1);
			p += 3;
			break;

		case '?':
			if (props && size > 0) {
				count = cavan_http_parse_url_param(p + 1, props, size);
			}
		case 0:
			goto out_complete;

		default:
			*url = *p++;
		}

		url++;
	}

out_complete:
	*url = 0;
	return count;
}

void cavan_http_parse_prop(char *text, struct cavan_http_prop *prop)
{
	prop->key = text;
	prop->value = NULL;

	while (1) {
		switch (*text) {
		case 0:
			text_clear_space_invert(prop->key, text - 1);
			return;

		case ' ':
		case '\t':
			if (prop->key == text) {
				prop->key++;
			} else if (prop->value == text) {
				prop->value++;
			}
			break;

		case ':':
			if (prop->value) {
				break;
			}

			*text = 0;
			prop->value = text + 1;
			text_clear_space_invert(prop->key, text - 1);
			break;
		}

		text++;
	}
}

int cavan_http_read_request(struct cavan_fifo *fifo, struct cavan_http_request *req, boolean full)
{
	char *p;
	int rdlen;
	struct cavan_http_prop *prop, *prop_end;
	char *mem = req->mem, *mem_end = mem + req->mem_size;

	if (full) {
		rdlen = cavan_fifo_read_line_strip(fifo, mem, mem_end - mem);
		if (rdlen < 0) {
			pr_red_info("cavan_fifo_read_line_strip");
			return rdlen;
		}

		p = mem;
		mem = p + rdlen + 1;

		req->type = text_skip_space2(p);
		p = text_find_space2(req->type);
		if (p == NULL) {
			pr_red_info("text_find_space2");
			return -EINVAL;
		}

		*p = 0;

		req->url = text_skip_space2(p + 1);
		p = text_find_space2(req->url);
		if (p != NULL) {
			*p = 0;
			req->version = text_skip_space2(p + 1);
		} else {
			req->version = NULL;
		}
	}

	for (prop = req->props, prop_end = prop + req->prop_size; prop < prop_end; prop++) {
		rdlen = cavan_fifo_read_line_strip(fifo, mem, mem_end - mem);
		if (rdlen < 1) {
			if (rdlen < 0) {
				pr_red_info("cavan_fifo_read_line_strip");
				return rdlen;
			}

			break;
		}

		cavan_http_parse_prop(mem, prop);
		mem += rdlen + 1;
	}

	req->prop_used = prop - req->props;
	req->param_used = cavan_http_parse_url(req->url, req->params, req->param_size);

#if CAVAN_HTTP_DEBUG
	cavan_http_request_dump(req);
#endif

	return 0;
}

int cavan_http_send_reply(struct network_client *client, int code, const char *format, ...)
{
	va_list ap;
	char buff[2048];
	char *p = buff, *p_end = p + sizeof(buff);

	p += snprintf(p, p_end - p,
		"HTTP/1.1 %d None\r\n"
		"Connection: keep-alive\r\n\r\n", code);

	va_start(ap, format);
	p += vsnprintf(p, p_end - p, format, ap);
	va_end(ap);

#if CAVAN_HTTP_DEBUG
	println("reply[%" PRINT_FORMAT_SIZE "]:\n%s", p - buff, buff);
#endif

	return network_client_send(client, buff, p - buff);
}

int cavan_http_open_html_file(const char *title, char *pathname)
{
	int fd;
	char buff[1024];

	if (pathname == NULL) {
		pathname = buff;
	}

	cavan_build_temp_path("cavan-XXXXXX", pathname, sizeof(buff));
	fd = mkstemp(pathname);
	if (fd < 0) {
		pr_error_info("mkstemp `%s'", pathname);
		return fd;
	}

	unlink(pathname);

#if CAVAN_HTTP_DEBUG
	println("pathname = %s, title = %s", pathname, title);
#endif

	ffile_puts(fd, "<!-- This file is automatic generate by Fuang.Cao -->\r\n\r\n");
	ffile_printf(fd, "<html>\r\n\t<head>\r\n\t\t<title>%s</title>\r\n\t</head>\r\n\t<body>\r\n", title);

	return fd;
}

int cavan_http_flush_html_file(int fd)
{
	int ret;

	ret = ffile_puts(fd, "\t</body>\r\n</html>");
	if (ret < 0) {
		pr_red_info("ffile_puts");
		return ret;
	}

	return fsync(fd);
}

int cavan_http_send_file_header(struct network_client *client, const char *filetype, struct tm *time, size_t start, size_t length, size_t size)
{
	char buff[2048];
	char *p = buff, *p_end = p + sizeof(buff);

	p = text_ncopy(p,
		"HTTP/1.1 200 Gatewaying\r\n"
		"Mime-Version: 1.0\r\n"
		"X-Cache: MISS from server\r\n", p_end - p);

	if (filetype == NULL) {
		filetype = "application/octet-stream";
	}

	p += snprintf(p, p_end - p, "Content-Length: %" PRINT_FORMAT_SIZE "\r\n", length);
	p += snprintf(p, p_end - p, "Content-Type: %s\r\n", filetype);
	p += snprintf(p, p_end - p, "Content-Range: bytes %" PRINT_FORMAT_SIZE "-%" PRINT_FORMAT_SIZE "/%" PRINT_FORMAT_SIZE "\r\n", start, start + length - 1, size);

	if (time) {
		p = text_copy(p, "Last-Modified: ");
		p = text_tail(asctime_r(time, p)) - 1;
		p = text_copy(p, "\r\n");
	}

	p = text_copy(p, "Connection: keep-alive\r\n\r\n");

#if CAVAN_HTTP_DEBUG
	println("reply[%" PRINT_FORMAT_SIZE "]:\n%s", p - buff, buff);
#endif

	return client->send(client, buff, p - buff);
}

int cavan_http_send_file(struct network_client *client, int fd, const char *filetype, size_t start, size_t length)
{
	int ret;
	struct stat st;
	struct tm time;

	if (lseek(fd, start, SEEK_SET) != (off_t) start) {
		pr_error_info("lseek");
		return -EFAULT;
	}

	ret = fstat(fd, &st);
	if (ret < 0) {
		pr_red_info("stat");
		return ret;
	}

	if (length == 0 || start + length > (size_t) st.st_size) {
		length = st.st_size - start;
	}

	ret = cavan_http_send_file_header(client, filetype, localtime_r((time_t *) &st.st_mtime, &time), start, length, st.st_size);
	if (ret < 0) {
		pr_red_info("web_proxy_ftp_send_http_reply");
		return ret;
	}

	return network_client_send_file(client, fd, 0, length);
}

int cavan_http_send_file2(struct network_client *client, const char *pathname, const char *filetype, size_t start, size_t length)
{
	int fd;
	int ret;

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pr_err_info("open: %s", pathname);
		return fd;
	}

	ret = cavan_http_send_file(client, fd, filetype, start, length);
	close(fd);

	return ret;
}

int cavan_http_send_file3(struct network_client *client, const char *pathname, const struct cavan_http_prop *props, size_t propc)
{
	const char *range;
	size_t start, length;

	range = cavan_http_prop_find_value(props, propc, "Range");
	if (range == NULL) {
		start = length = 0;
	} else {
		if (text_lhcmp("bytes", range) == 0) {
			range += 5;
		}

		while (cavan_isspace(*range) || *range == '=') {
			range++;
		}

		if (*range == '-') {
			start = 0;
			length = text2value_unsigned(range + 1, NULL, 10);
		} else {
			int ret;
			size_t end;

			ret = sscanf(range, "%" PRINT_FORMAT_SIZE "-%" PRINT_FORMAT_SIZE, &start, &end);
			if (ret < 2) {
				if (ret < 1) {
					start = 0;
				}

				length = 0;
			} else {
				length = end - start + 1;
			}
		}
	}

	return cavan_http_send_file2(client, pathname, NULL, start, length);
}

int cavan_http_list_directory(struct network_client *client, const char *dirname)
{
	int fd;
	DIR *dp;
	int ret;
	struct stat st;
	char *filename;
	char pathname[1024];
	struct dirent *entry;

	dp = opendir(dirname);
	if (dp == NULL) {
		pr_err_info("opendir: %s", dirname);
		return -EFAULT;
	}

	fd = cavan_http_open_html_file(dirname, NULL);
	if (fd < 0) {
		ret = fd;
		pr_red_info("web_proxy_open_html_file");
		goto out_closedir;
	}

	filename = text_path_cat(pathname, sizeof(pathname), dirname, NULL);

	ffile_printf(fd, "\t\t<h5>Path: <a href=\"%s\">%s</a></h5>\r\n", pathname, dirname);
	ffile_printf(fd, "\t\t<h5><a href=\"..\">Parent directory</a> (<a href=\"/\">Root directory</a>)</h5>\r\n");
	ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

	while ((entry = readdir(dp))) {
		char buff[32];
		struct tm time;
		const char *type;

		if (text_is_dot_name(entry->d_name)) {
			continue;
		}

		strcpy(filename, entry->d_name);

		if (file_stat(pathname, &st) < 0) {
			continue;
		}

		ffile_puts(fd, "\t\t\t<tr class=\"entry\">");

		switch (st.st_mode & S_IFMT) {
		case S_IFLNK:
			type = "LINK";
			break;

		case S_IFIFO:
			type = "FIFO";
			break;

		case S_IFCHR:
			type = "CHR";
			break;

		case S_IFBLK:
			type = "BLK";
			break;

		case S_IFDIR:
			type = "DIR";
			break;

		case S_IFSOCK:
			type = "SOCK";
			break;

		default:
			type = "FILE";
		}

		ffile_printf(fd, "<td class=\"type\">[%s]</td><td class=\"filename\">", type);

		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			ffile_printf(fd, "<a href=\"%s/\">%s</a>", entry->d_name, entry->d_name);
		} else {
			ffile_printf(fd, "<a href=\"%s\">%s</a>", entry->d_name, entry->d_name);
		}

		ffile_puts(fd, "</td><td class=\"size\">");
		ffile_write(fd, buff, mem_size_tostring_simple(st.st_size, buff, sizeof(buff)) - buff);

		if (localtime_r((time_t *) &st.st_mtime, &time) == NULL) {
			memset(&time, 0x00, sizeof(time));
		}

		ffile_printf(fd, "</td><td class=\"date\">%04d-%02d-%02d %02d:%02d:%02d</td>",
			time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

		ffile_printf(fd, "</tr>\r\n");
	}

	ffile_puts(fd, "\t\t</table>\r\n");

	ret = cavan_http_flush_html_file(fd);
	if (ret < 0) {
		pr_red_info("cavan_http_flush_html_file: %d", ret);
		goto out_closedir;
	}

	ret = cavan_http_send_html(client, fd);
	if (ret < 0) {
		pr_red_info("stat");
	}

	close(fd);
out_closedir:
	closedir(dp);
	return ret;
}

int cavan_http_process_get(struct network_client *client, struct cavan_http_request *req)
{
	int ret;
	struct stat st;

	ret = file_stat(req->url, &st);
	if (ret < 0) {
		cavan_http_send_reply(client, 404, "File not found: %s", req->url);
		return ret;
	}

	switch (st.st_mode & S_IFMT) {
	case S_IFREG:
		return cavan_http_send_file3(client, req->url, req->props, req->prop_used);

	case S_IFDIR:
		return cavan_http_list_directory(client, req->url);

	default:
		cavan_http_send_reply(client, 403, "Invalid file type");
		return -EINVAL;
	}
}

ssize_t cavan_http_read_multiform_header(struct network_client *client, const char *boundary, char *buff, size_t size)
{
	char c;
	int count;
	char *p, *p_end;

	if (network_client_recv(client, buff, 2) != 2) {
		return -EFAULT;
	}

	if (strncmp("--", buff, 2) != 0) {
		return -EINVAL;
	}

	while (1) {
		if (network_client_recv(client, &c, 1) != 1) {
			return -EFAULT;
		}

		if (c != *boundary) {
			if (*boundary != 0) {
				return -EINVAL;
			}

			break;
		}

		boundary++;
	}

	count = 0;

	for (count = 0; count < 2 && cavan_islf(c); count++) {
		if (network_client_recv(client, &c, 1) != 1) {
			return -EFAULT;
		}
	}

	if (count < 1) {
		return -EINVAL;
	}

	count = 0;

	p = buff;
	p_end = p + size;

	while (p < p_end) {
		*p++ = c;

		switch (c) {
		case '\n':
			if (count > 0) {
				goto out_complete;
			}
			count++;
		case '\r':
			break;

		default:
			count = 0;
		}

		if (network_client_recv(client, &c, 1) != 1) {
			return -EFAULT;
		}
	}

out_complete:
	return p - buff;
}

ssize_t cavan_http_file_receive(struct network_client *client, const char *pathname, const char *boundary, size_t length)
{
	char c;
	int fd;
	int lf;
	int count;
	int rdlen;
	const char *p;
	char lf_buff[2];
	char header[1024];

	rdlen = cavan_http_read_multiform_header(client, boundary, header, sizeof(header));
	if (rdlen < 0) {
		pr_red_info("cavan_http_read_multiform_header: %d", rdlen);
		return rdlen;
	}

	header[rdlen] = 0;

#if CAVAN_HTTP_DEBUG
	println("header[%d]:\n%s", rdlen, header);
#endif

	fd = open("/tmp/cavan-http-receive.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open: %d", fd);
		return fd;
	}

	lf = 0;
	count = 0;

	while (1) {
		if (network_client_recv(client, &c, 1) != 1) {
			return -EFAULT;
		}

		switch (c) {
		case '\n':
		case '\r':
			while (count > 0) {
				if (write(fd, "-", 1) != 1) {
					return -EFAULT;
				}

				count--;
			}

			if (lf >= (int) sizeof(lf_buff)) {
				int i;

				if (write(fd, lf_buff, 1) != 1) {
					return -EFAULT;
				}

				lf--;

				for (i = 0; i < lf; i++) {
					lf_buff[i] = lf_buff[i + 1];
				}
			}

			lf_buff[lf++] = c;
			break;

		case '-':
			if (lf < (int) sizeof(lf_buff)) {
				if (lf > 0) {
					if (write(fd, lf_buff, lf) != lf) {
						return -EFAULT;
					}

					lf = 0;
				}

				if (write(fd, &c, 1) != 1) {
					return -EFAULT;
				}

				break;
			}

			if (++count > 1) {
				p = boundary;

				while (1) {
					if (network_client_recv(client, &c, 1) != 1) {
						return -EFAULT;
					}

					if (c != *p) {
						if (*p != 0 || c != '-') {
							break;
						}

						if (count > 1) {
							count--;
							continue;
						} else {
							goto out_close_fd;
						}
					}

					p++;
				}

				if (write(fd, lf_buff, lf) != lf) {
					return -EFAULT;
				}

				lf = 0;

				if (write(fd, "--", 2) != 2) {
					return -EFAULT;
				}

				count = p - boundary;
				if (write(fd, p, count) != count) {
					return -EFAULT;
				}

				if (write(fd, &c, 1) != 1) {
					return -EFAULT;
				}

				count = 0;
			}
			break;

		default:
			if (lf > 0) {
				if (write(fd, lf_buff, lf) != lf) {
					return -EFAULT;
				}

				lf = 0;
			}

			while (count > 0) {
				if (write(fd, "-", 1) != 1) {
					return -EFAULT;
				}

				count--;
			}

			if (write(fd, &c, 1) != 1) {
				return -EFAULT;
			}
		}
	}

out_close_fd:
	close(fd);
	return 0;
}

char *cavan_http_get_boundary(struct cavan_http_prop *props, size_t propc)
{
	int count;
	struct cavan_http_prop args[2];
	char *value = (char *) cavan_http_prop_find_value(props, propc, "Content-Type");

#if CAVAN_HTTP_DEBUG
	println("Content-Type = %s", value);
#endif

	if (value == NULL) {
		return NULL;
	}

	count = cavan_http_parse_prop_value(value, args, NELEM(args));

#if CAVAN_HTTP_DEBUG
	cavan_http_dump_props(args, count);
#endif

	if (count < 2) {
		pr_red_info("cavan_http_parse_prop_value: %d", count);
		return NULL;
	}

	if (strcmp(args[0].key, "multipart/form-data") != 0) {
		pr_red_info("Invalid key: %s", args[0].key);
		return NULL;
	}

	if (strcmp(args[1].key, "boundary") != 0) {
		pr_red_info("Invalid key: %s", args[1].key);
		return NULL;
	}

	return (char *) args[1].value;
}

int cavan_http_process_post(struct network_client *client, struct cavan_http_request *req)
{
	const char *boundary = cavan_http_get_boundary(req->props, req->prop_used);

	if (boundary == NULL) {
		int rdlen;
		char buff[1024];

		rdlen = network_client_recv(client, buff, sizeof(buff));
		if (rdlen > 0) {
			buff[rdlen] = 0;
			println("buff[%d]:\n%s", rdlen, buff);
		}

		cavan_http_send_reply(client, 403, "boundary not found");
		return -EINVAL;
	}

	cavan_http_file_receive(client, req->url, boundary, 0);

	return -EINVAL;
}

// ================================================================================

static int cavan_http_start_handler(struct cavan_dynamic_service *service)
{
	struct cavan_http_service *http = cavan_dynamic_service_get_data(service);

	return network_service_open(&http->service, &http->url, 0);
}

static void cavan_http_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_http_service *http = cavan_dynamic_service_get_data(service);

	network_service_close(&http->service);
}

static int cavan_http_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_http_service *http = cavan_dynamic_service_get_data(service);

	return network_service_accept(&http->service, conn);
}

static void cavan_http_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int cavan_http_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	int type;
	struct cavan_fifo fifo;
	struct cavan_http_request *req;
	struct network_client *client = conn;

	req = cavan_http_request_alloc(1024, 20, 10);
	if (req == NULL) {
		pr_red_info("cavan_http_request_alloc");
		return -EFAULT;
	}

	ret = cavan_fifo_init(&fifo, 1024, client);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		goto out_cavan_http_request_free;
	}

	fifo.read = network_client_fifo_read;

	ret = cavan_http_read_request(&fifo, req, true);
	if (ret < 0) {
		pr_red_info("cavan_http_read_request");
		goto out_cavan_fifo_deinit;
	}

	type = cavan_http_request_get_type2(req->type);
	switch (type) {
	case HTTP_REQ_GET:
		cavan_http_process_get(client, req);
		break;

	case HTTP_REQ_POST:
		cavan_http_process_post(client, req);
		break;
	}

out_cavan_fifo_deinit:
	cavan_fifo_deinit(&fifo);
out_cavan_http_request_free:
	cavan_http_request_free(req);
	return 0;
}

int cavan_http_service_run(struct cavan_dynamic_service *service)
{
	service->name = "HTTP";
	service->conn_size = sizeof(struct network_client);
	service->start = cavan_http_start_handler;
	service->stop = cavan_http_stop_handler;
	service->run = cavan_http_service_run_handler;
	service->open_connect = cavan_http_open_connect;
	service->close_connect = cavan_http_close_connect;

	return cavan_dynamic_service_run(service);
}
