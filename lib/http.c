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
#include <cavan/android.h>
#include <cavan/web_proxy.h>

#define CAVAN_HTTP_DEBUG	0

void cavan_http_dump_prop(const struct cavan_http_prop *prop)
{
	pd_info("key = %s, value = %s", prop->key, prop->value);
}

void cavan_http_dump_props(const struct cavan_http_prop *props, size_t size)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + size; props < prop_end; props++) {
		cavan_http_dump_prop(props);
	}
}

void cavan_http_dump_request(struct cavan_http_request *req)
{
	print_sep(80);

	if (req->type) {
		pd_info("type = %s", req->type);
		pd_info("url = %s", req->url);
		pd_info("version = %s", req->version);
	}

	pd_info("props[%" PRINT_FORMAT_SIZE "/%" PRINT_FORMAT_SIZE "]:", req->prop_used, req->prop_size);
	cavan_http_dump_props(req->props, req->prop_used);
	pd_info("params[%" PRINT_FORMAT_SIZE "/%" PRINT_FORMAT_SIZE "]:", req->param_used, req->param_size);
	cavan_http_dump_props(req->params, req->param_used);
}

struct cavan_http_request *cavan_http_request_alloc(size_t mem_size, size_t prop_size, size_t param_size)
{
	struct cavan_http_request *req = malloc(sizeof(struct cavan_http_request) + sizeof(struct cavan_http_prop) * (prop_size + param_size) + mem_size);
	if (req == NULL) {
		return NULL;
	}

	req->type = NULL;
	req->url = NULL;
	req->version = NULL;

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

int cavan_http_get_request_type(const char *req, size_t length)
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

int cavan_http_get_request_type2(const char *type)
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

size_t cavan_http_parse_request(char *req, char *req_end, struct cavan_http_prop *props, size_t size)
{
	struct cavan_http_prop *prop, *prop_end;

	if (size == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + size;

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

size_t cavan_http_parse_prop_value(char *value, struct cavan_http_prop *props, size_t size)
{
	struct cavan_http_prop *prop, *prop_end;

	if (size == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + size;

	prop->key = value;
	prop->value = NULL;

	while (1) {
		switch (*value) {
		case 0:
			if (value > prop->key) {
				text_clear_space_and_quote_invert(prop->key, value - 1);
				prop++;
			}
			goto out_complete;

		case ';':
			*value = 0;

			if (value > prop->key) {
				text_clear_space_and_quote_invert(prop->key, value - 1);

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

		case '"':
		case '\'':
			if (prop->value == value) {
				prop->value++;
			}
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

const struct cavan_http_prop *cavan_http_find_prop(const struct cavan_http_prop *props, size_t size, const char *key)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + size; props < prop_end; props++) {
		if (strcmp(props->key, key) == 0) {
			return props;
		}
	}

	return NULL;
}

const char *cavan_http_find_prop_simple(const struct cavan_http_prop *props, size_t size, const char *key)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + size; props < prop_end; props++) {
		if (strcmp(props->key, key) == 0) {
			return props->value;
		}
	}

	return NULL;
}

size_t cavan_http_parse_url_param(char *text, struct cavan_http_prop *props, size_t size)
{
	char *p;
	struct cavan_http_prop *prop, *prop_end;

	if (size == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + size;

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

int cavan_http_read_props(struct cavan_fifo *fifo, struct cavan_http_request *req)
{
	int rdlen;
	struct cavan_http_prop *prop, *prop_end;
	char *mem = req->mem + req->mem_used, *mem_end = mem + req->mem_size;

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

	return 0;
}

int cavan_http_read_request(struct cavan_fifo *fifo, struct cavan_http_request *req)
{
	int rdlen;
	char *texts[3];

	rdlen = cavan_fifo_read_line_strip(fifo, req->mem, req->mem_size);
	if (rdlen < 0) {
		pr_red_info("cavan_fifo_read_line_strip");
		return rdlen;
	}

	req->mem_used = rdlen + 1;

	rdlen = text_split_by_space(req->mem, texts, NELEM(texts));
	if (rdlen < 2) {
		pr_red_info("text_split_by_space: %d", rdlen);
		return -EINVAL;
	}

	req->type = texts[0];
	req->url = texts[1];

	if (rdlen > 2) {
		req->version = texts[2];
	} else {
		req->version = NULL;
	}

	req->param_used = cavan_http_parse_url(req->url, req->params, req->param_size);

	return cavan_http_read_props(fifo, req);
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
	pd_info("reply[%" PRINT_FORMAT_SIZE "]:\n%s", p - buff, buff);
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

	cavan_path_build_tmp_path("cavan-XXXXXX", pathname, sizeof(buff));
	fd = mkstemp(pathname);
	if (fd < 0) {
		pr_error_info("mkstemp `%s'", pathname);
		return fd;
	}

	unlink(pathname);

#if CAVAN_HTTP_DEBUG
	pd_info("pathname = %s, title = %s", pathname, title);
#endif

	ffile_puts(fd, "<!-- This file is automatic generate by Fuang.Cao -->\r\n\r\n");
	ffile_puts(fd, "<html>\r\n\t<head>\r\n\t\t<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\r\n");
	ffile_printf(fd, "\t\t<title>%s</title>\r\n\t</head>\r\n\t<body>\r\n", title);

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
	pd_info("reply[%" PRINT_FORMAT_SIZE "]:\n%s", p - buff, buff);
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

int cavan_http_send_file3(struct network_client *client, const char *pathname, const struct cavan_http_prop *props, size_t size)
{
	const char *range;
	size_t start, length;

	range = cavan_http_find_prop_simple(props, size, "Range");
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

int cavan_http_write_path_href(int fd, const char *pathname, size_t psize, const char *name, size_t nsize)
{
	int ret;

	ret = ffile_puts(fd, "<a href=\"");
	ret |= ffile_write(fd, pathname, psize);
	ret |= ffile_puts(fd, "\">");
	ret |= ffile_write(fd, name, nsize);
	ret |= ffile_puts(fd, "</a>");

	return ret;
}

int cavan_http_write_path_href2(int fd, const char *pathname, const char *name)
{
	return cavan_http_write_path_href(fd, pathname, strlen(pathname), name, strlen(name));
}

int cavan_http_write_path_hrefs(int fd, const char *pathname)
{
	int ret = 0;
	const char *p = pathname;
	const char *filename = pathname;

	while (1) {
		switch (*p) {
		case 0:
			if (p > filename) {
				return cavan_http_write_path_href(fd, pathname, p - pathname, filename, p - filename);
			}
			return 0;

		case '/':
			if (p > filename) {
				ret |= cavan_http_write_path_href(fd, pathname, p - pathname + 1, filename, p - filename);
				ret |= ffile_putchar(fd, '/');
			} else if (p == pathname) {
				ret |= cavan_http_write_path_href2(fd, "/", "ROOT");
				ret |= ffile_putchar(fd, '/');
			}

			if (ret < 0) {
				return ret;
			}

			filename = p + 1;
			break;
		}

		p++;
	}
}

int cavan_http_list_directory(struct network_client *client, const char *dirname, const char *filter)
{
	int i;
	int fd;
	DIR *dp;
	int ret;
	struct stat st;
	const char *env;
	char *filename;
	char pathname[1024];
	struct dirent *entry;

#if CAVAN_HTTP_DEBUG
	println("filter = %s", filter);
#endif

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

	ffile_puts(fd, "\t\t<script type=\"text/javascript\">\r\n");
	ffile_puts(fd, "\t\tfunction onUploadSubmit(form) {\r\n");
	ffile_puts(fd, "\t\t\tif (form.pathname.value.length > 0) {\r\n");
	ffile_puts(fd, "\t\t\t\treturn true;\r\n");
	ffile_puts(fd, "\t\t\t}\r\n");
	ffile_puts(fd, "\t\t\talert(\"Please select a file!\");\r\n");
	ffile_puts(fd, "\t\t\treturn false;\r\n");
	ffile_puts(fd, "\t\t}\r\n");
	ffile_puts(fd, "\t\t</script>\r\n");
	ffile_puts(fd, "\t\t<h5>Current directory: ");

	filename = cavan_path_copy(pathname, sizeof(pathname), dirname, true);

	ret = cavan_http_write_path_hrefs(fd, pathname);
	if (ret < 0) {
		pr_red_info("cavan_http_write_path_html: %d", ret);
		goto out_closedir;
	}

	ffile_puts(fd, "</h5>\r\n\t\t<h5>[<a href=\"..\">Parent</a>]");

	env = cavan_getenv("HOME", NULL);
	if (env != NULL) {
		ffile_printf(fd, " [<a href=\"%s/\">Home</a>]", env);
	}

	env = cavan_path_get_tmp_directory();
	if (env != NULL) {
		ffile_printf(fd, " [<a href=\"%s/\">Temp</a>]", env);
	}

	env = cavan_getenv("APP_PATH", NULL);
	if (env != NULL) {
		ffile_printf(fd, " [<a href=\"%s/\">App</a>]", env);
	}

	for (i = 0; i < 10; i++) {
		char key[64];

		snprintf(key, sizeof(key), "SDCARD%d_PATH", i);

		env = cavan_getenv(key, NULL);
		if (env == NULL) {
			break;
		}

		ffile_printf(fd, " [<a href=\"%s/\">SDcard%d</a>]", env, i);
	}

	ffile_puts(fd, "</h5>\r\n\t\t<form enctype=\"multipart/form-data\" onsubmit=\"return onUploadSubmit(this)\" action=\".\" method=\"post\">\r\n");
	ffile_puts(fd, "\t\t\t<input type=\"submit\" value=\"Upload\">\r\n");
	ffile_puts(fd, "\t\t\t<input id=\"upload\" name=\"pathname\" type=\"file\">\r\n");
	ffile_puts(fd, "\t\t</form>\r\n");
	ffile_puts(fd, "\t\t<form method=\"get\">\r\n");
	ffile_printf(fd, "\t\t\t<input name=\"filter\" type=\"text\" value=\"%s\">\r\n", text_fixup_null_simple(filter));
	ffile_puts(fd, "\t\t\t<input type=\"submit\" value=\"Search\">\r\n");
	ffile_puts(fd, "\t\t</form>\r\n");
	ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

	filter = text_fixup_empty_simple(filter);

	while ((entry = readdir(dp))) {
		char buff[32];
		struct tm time;
		const char *type;

		if (cavan_path_is_dot_name(entry->d_name)) {
			continue;
		}

		if (filter != NULL && strcasestr(entry->d_name, filter) == NULL) {
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
		return cavan_http_list_directory(client, req->url, cavan_http_request_find_param_simple(req, "filter"));

	default:
		cavan_http_send_reply(client, 403, "Invalid file type");
		return -EINVAL;
	}
}

int cavan_http_read_multiform_header(struct cavan_fifo *fifo, struct cavan_http_request *header, const char *boundary)
{
	int ret;
	int rdlen;
	const char *disposition;

	rdlen = cavan_fifo_read_line_strip(fifo, header->mem, header->mem_size);
	if (rdlen < 0) {
		pr_red_info("cavan_fifo_read_line_strip");
		return rdlen;
	}

	if (text_lhcmp("--", header->mem) || strcmp(header->mem + 2, boundary)) {
		pr_red_info("Invalid boundary: %s", header->mem);
		return -EINVAL;
	}

	header->mem_used = rdlen + 1;

	ret = cavan_http_read_props(fifo, header);
	if (ret < 0) {
		pr_red_info("cavan_http_read_props");
		return ret;
	}

	disposition = cavan_http_request_find_prop_simple(header, "Content-Disposition");

#if CAVAN_HTTP_DEBUG
	pd_info("Content-Description = %s", disposition);
#endif

	if (disposition != NULL) {
		header->param_used = cavan_http_parse_prop_value((char *) disposition, header->params, header->param_size);
	}

	return 0;
}

ssize_t cavan_http_file_receive(struct cavan_fifo *fifo, const char *dirname, const char *boundary)
{
	int fd;
	int ret;
	size_t total;
	char pathname[1024];
	char *pathname_end;
	const char *filename;
	const char *mime_type;
	struct cavan_http_request *header;

	header = cavan_http_request_alloc(1024, 10, 5);
	if (header == NULL) {
		pr_red_info("cavan_http_request_alloc");
		return -ENOMEM;
	}

	ret = cavan_http_read_multiform_header(fifo, header, boundary);
	if (ret < 0) {
		pr_red_info("cavan_http_read_multiform_header: %d", ret);
		return ret;
	}

#if CAVAN_HTTP_DEBUG
	cavan_http_dump_request(header);
#endif

	mime_type = cavan_http_request_find_prop_simple(header, "Content-Type");
	println("mime_type = %s", mime_type);

	filename = cavan_http_request_find_param_simple(header, "filename");
	if (filename == NULL || filename[0] == 0) {
		pr_red_info("filename not found");
		return -EINVAL;
	}

	pathname_end = cavan_path_cat(pathname, sizeof(pathname), dirname, filename, false);
	strcpy(pathname_end, ".cavan.cache");

	pd_info("pathname = %s", pathname);

	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open: %d", fd);
		return fd;
	}

	total = 0;

	while (1) {
		int rdlen;
		char buff[1024];

		rdlen = cavan_fifo_read_line(fifo, buff, sizeof(buff));
		if (rdlen < 0) {
			ret = rdlen;
			pr_err_info("cavan_fifo_read_line: %d", rdlen);
			goto out_close_fd;
		}

		if (text_lhcmp("--", buff) == 0 &&
			text_lhcmp(boundary, buff + 2) == 0 &&
			text_lhcmp("--", buff + rdlen - 4) == 0) {
			break;
		}

		ret = write(fd, buff, rdlen);
		if (ret < rdlen) {
			if (ret >= 0) {
				ret = -EFAULT;
			}

			pr_err_info("write: %d", ret);
			goto out_close_fd;
		}

		total += rdlen;
	}

	if (total >= 2) {
		ret = ftruncate(fd, total - 2);
		if (ret < 0) {
			pr_err_info("ftruncate: %d", ret);
			goto out_close_fd;
		}
	}

	close(fd);

	ret = cavan_rename_part(pathname, NULL, pathname_end);
	if (ret < 0) {
		pr_err_info("cavan_rename_part: %d", ret);
		goto out_unlink_pathname;
	}

	*pathname_end = 0;
	pd_info("pathname = %s", pathname);

#ifdef CONFIG_ANDROID
	if (mime_type != NULL && strcmp(mime_type, "application/vnd.android.package-archive") == 0) {
		android_install_application_async(pathname, 0);
	}
#endif

	return 0;

out_close_fd:
	close(fd);
out_unlink_pathname:
	unlink(pathname);
	return ret;
}

char *cavan_http_get_boundary(struct cavan_http_prop *props, size_t size)
{
	int count;
	struct cavan_http_prop args[2];
	char *value = (char *) cavan_http_find_prop_simple(props, size, "Content-Type");

#if CAVAN_HTTP_DEBUG
	pd_info("Content-Type = %s", value);
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

int cavan_http_process_post(struct cavan_fifo *fifo, struct cavan_http_request *req)
{
	int ret;
	const char *text;
	const char *boundary = cavan_http_get_boundary(req->props, req->prop_used);

	if (boundary == NULL) {
		cavan_http_send_reply(fifo->private_data, 403, "boundary not found");
		return -EINVAL;
	}

	text = cavan_http_request_find_prop_simple(req, "Content-Length");
	if (text != NULL) {
		cavan_fifo_set_available(fifo, text2value_unsigned(text, NULL, 10));
	}

	do {
		ret = cavan_http_file_receive(fifo, req->url, boundary);
		if (ret < 0) {
			cavan_http_send_reply(fifo->private_data, 403, "Failed to upload: %d", ret);
			return ret;
		}
	} while (cavan_fifo_get_remain(fifo) > 0);

	return cavan_http_send_reply(fifo->private_data, 200, "Upload successfull");
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

	ret = cavan_http_read_request(&fifo, req);
	if (ret < 0) {
		pr_red_info("cavan_http_read_request");
		goto out_cavan_fifo_deinit;
	}

#if CAVAN_HTTP_DEBUG
	cavan_http_dump_request(req);
#endif

	type = cavan_http_get_request_type2(req->type);
	switch (type) {
	case HTTP_REQ_GET:
		cavan_http_process_get(client, req);
		break;

	case HTTP_REQ_POST:
		cavan_http_process_post(&fifo, req);
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
