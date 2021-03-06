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
#include <cavan/ftp.h>
#include <cavan/http.h>
#include <cavan/timer.h>
#include <cavan/android.h>
#include <cavan/web_proxy.h>

#define CAVAN_HTTP_DEBUG	0

const char *http_mime_type_bin = "application/octet-stream";
const char *http_mime_type_txt = "text/plain";
const char *http_mime_type_html = "text/html";
const char *http_mime_type_js = "application/x-javascript";
const char *http_mime_type_apk = "application/vnd.android.package-archive";
const char *http_mime_type_cer = "application/x-x509-ca-cert";

const cavan_string_t http_header_names[HTTP_HEADER_COUNT] = {
	CAVAN_STRING_INITIALIZER("Host"),
	CAVAN_STRING_INITIALIZER("Date"),
	CAVAN_STRING_INITIALIZER("Content-Type"),
	CAVAN_STRING_INITIALIZER("Content-Length"),
	CAVAN_STRING_INITIALIZER("Content-Encoding"),
	CAVAN_STRING_INITIALIZER("Transfer-Encoding"),
};

const struct cavan_http_signin_info http_signin_infos[] = {
	{ "XJXQY", "http://jxqy.qq.com/act/agile/48347/index.html?appid=wxacbfe7e1bb3e800f&logtype=wx" },
	{ "RXCQ", "http://mir.qq.com/act/a20150729cdkey/m/wx.htm" },
	{ "XYCQ", "https://xycq.qq.com/act/agile2_62354/index_2.html" },
	{ "CQBY", "http://shuang.qq.com/cp/a20171122cdkm/index.shtml" },
	{ "CYHX", "http://open.weixin.qq.com/connect/oauth2/authorize?appid=wx58164a91f1821369&redirect_uri=http%3A%2F%2Fcfm.qq.com%2Fcp%2Fa20151125king%2Fwxcdkey.htm%3Facctype%3Dwx%26appid%3Dwx58164a91f1821369&response_type=code&scope=snsapi_userinfo&state=STATE&version=62060028" },
	{ "XXSY", "http://xxsy.qq.com/act/a20170303cdkm/index.html" },
	{ "TLBB", "http://tlbb.qq.com/act/agile/108149/index.html?appid=wx44b60f3ea0534cd7&logtype=wx" },
	{ "ZTJSY", "http://ztj.qq.com/act/agile2_117540/index.html" },
	{ "ZTSJB", "http://zt.qq.com/act/agile2.0/47886/zt.html" },
	{ "JDQS-QJCJ", "https://pubgm.qq.com/act/agile2_136573/index.html" },
};

int cavan_http_get_header_type(const char *name, int length)
{
	const cavan_string_t *p, *p_end;

	for (p = http_header_names, p_end = p + HTTP_HEADER_COUNT; p < p_end; p++) {
		if (length == p->length && strncasecmp(name, p->text, length) == 0) {
			return p - http_header_names;
		}
	}

	return -EINVAL;
}

int cavan_http_time_tostring(struct tm *time, char *buff, int size)
{
	return snprintf(buff, size, "%s, %02d-%s-%04d %02d:%02d:%02d GMT", week_tostring(time->tm_wday),
		time->tm_mday, month_tostring(time->tm_mon), CAVAN_TIME_FIXUP_YEAR(time->tm_year),
		time->tm_hour, time->tm_min, time->tm_sec);
}

int cavan_http_time_tostring2(const time_t *time, char *buff, int size)
{
	return cavan_http_time_tostring(gmtime(time), buff, size);
}

int cavan_http_time_tostring3(char *buff, int size)
{
	time_t now = time(NULL);
	return cavan_http_time_tostring2(&now, buff, size);
}

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

	req->props = (struct cavan_http_prop *) (req->mem + mem_size);
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

void cavan_http_request_reset(struct cavan_http_request *req)
{
	req->mem_used = 0;
	req->prop_used = 0;
	req->param_used = 0;
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
		} else if (length == 8 && text_lhcmp("ROPFIND", req + 1) == 0) {
			return HTTP_REQ_PROPFIND;
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
		} else if (strcmp("ROPFIND", type + 1) == 0) {
			return HTTP_REQ_PROPFIND;
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
		if (strcasecmp(props->key, key) == 0) {
			return props;
		}
	}

	return NULL;
}

const char *cavan_http_find_prop_simple(const struct cavan_http_prop *props, size_t size, const char *key)
{
	const struct cavan_http_prop *prop_end;

	for (prop_end = props + size; props < prop_end; props++) {
		if (strcasecmp(props->key, key) == 0) {
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
	struct cavan_http_prop *prop, *prop_end;
	char *mem = req->mem + req->mem_used;
	char *mem_end = req->mem + req->mem_size;

	for (prop = req->props, prop_end = prop + req->prop_size; prop < prop_end; prop++) {
		char *p = cavan_fifo_read_line_strip(fifo, mem, mem_end - mem);
		if (p == NULL) {
			pr_red_info("cavan_fifo_read_line_strip");
			return -EFAULT;
		}

		if (p > mem) {
			cavan_http_parse_prop(mem, prop);
		} else {
			break;
		}

		mem = p + 1;
	}

	req->prop_used = prop - req->props;

	return 0;
}

int cavan_http_read_request(struct cavan_fifo *fifo, struct cavan_http_request *req)
{
	int ret;
	char *p;
	char *texts[3];

	p = cavan_fifo_read_line_strip(fifo, req->mem, req->mem_size);
	if (p == NULL) {
		pr_red_info("cavan_fifo_read_line_strip");
		return -EFAULT;
	}

	req->mem_used = (p - req->mem) + 1;

	ret = text_split_by_space(req->mem, texts, NELEM(texts));
	if (ret < 2) {
		pr_red_info("text_split_by_space: %d", ret);
		return -EINVAL;
	}

	req->type = texts[0];
	req->url = texts[1];

	if (ret > 2) {
		req->version = texts[2];
	} else {
		req->version = NULL;
	}

	req->param_used = cavan_http_parse_url(req->url, req->params, req->param_size);

	ret = cavan_http_read_props(fifo, req);
	if (ret < 0) {
		pr_red_info("cavan_http_read_props: %d", ret);
		return ret;
	}

	return 0;
}

int cavan_http_send_reply(struct network_client *client, int code, const char *body, int length)
{
	int ret;

	ret = network_client_printf(client, "HTTP/1.1 %d None\r\n"
		"Connection: keep-alive\r\nContent-Length: %d\r\n\r\n", code, length);
	if (ret < 0) {
		return ret;
	}

	return network_client_send(client, body, length);
}

int cavan_http_send_replyf(struct network_client *client, int code, const char *format, ...)
{
	char buff[1024];
	va_list ap;
	int length;

	va_start(ap, format);
	length = vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	return cavan_http_send_reply(client, code, buff, length);
}

int cavan_http_send_redirect(struct network_client *client, const char *pathname)
{
	return network_client_printf(client, "HTTP/1.1 301 Permanently Moved\r\nConnection: keep-alive\r\nContent-Length: 0\r\nLocation: %s\r\n\r\n", pathname);
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
	ffile_printf(fd, "<html>\r\n\t<head>\r\n\t\t<meta http-equiv=\"content-type\" content=\"%s;charset=utf-8\" />\r\n", http_mime_type_html);
	ffile_puts(fd, "\t\t<meta name=\"viewport\" content=\"initial-scale=1\" />\r\n");
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
		filetype = http_mime_type_bin;
	}

	println("filetype = %s", filetype);

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

const char *cavan_http_get_mime_type(const char *pathname)
{
	const char *ext = cavan_file_get_extension(pathname);

	if (ext != NULL) {
		if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "xhtml") == 0) {
			return http_mime_type_html;
		}

		if (strcasecmp(ext, "txt") == 0) {
			return http_mime_type_txt;
		}

		if (strcasecmp(ext, "js") == 0) {
			return http_mime_type_js;
		}

		if (strcasecmp(ext, "apk") == 0) {
			return http_mime_type_apk;
		}

		if (strcasecmp(ext, "cer") == 0) {
			return http_mime_type_cer;
		}
	}

	return http_mime_type_bin;
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

	if (filetype == NULL) {
		filetype = cavan_http_get_mime_type(pathname);
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

int cavan_http_write_path_hrefs(struct network_client *client, int fd, const char *pathname)
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
				struct sockaddr_in addr;
				char buff[32];
				int length;

				if (network_client_get_remote_addr(client, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
					filename = "ROOT";
					length = 4;
				} else {
					length = snprintf(buff, sizeof(buff), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
					filename = buff;
				}

				ret |= cavan_http_write_path_href(fd, "/", 1, filename, length);
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
	int count;
	int ret = 0;
	struct stat st;
	const char *env;
	char *filename;
	char pathname[1024];
	struct dirent **entries;

	if (text_get_char(dirname, -1) != '/') {
		cavan_path_copy(pathname, sizeof(pathname), dirname, true);
		return cavan_http_send_redirect(client, pathname);
	}

#if CAVAN_HTTP_DEBUG
	println("filter = %s", filter);
#endif

	fd = cavan_http_open_html_file(dirname, NULL);
	if (fd < 0) {
		pr_red_info("web_proxy_open_html_file");
		return fd;
	}

	ret |= ffile_puts(fd, "\t\t<script type=\"text/javascript\">\r\n");
	ret |= ffile_puts(fd, "\t\tfunction onUploadSubmit(form) {\r\n");
	ret |= ffile_puts(fd, "\t\t\tif (form.pathname.value.length > 0) {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\treturn true;\r\n");
	ret |= ffile_puts(fd, "\t\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t\talert(\"Please select a file!\");\r\n");
	ret |= ffile_puts(fd, "\t\t\treturn false;\r\n");
	ret |= ffile_puts(fd, "\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t</script>\r\n");
	ret |= ffile_puts(fd, "\t\t<h5>Current directory: ");

	if (ret < 0) {
		pr_red_info("ffile_puts");
		goto out_close_fd;
	}

	filename = cavan_path_copy(pathname, sizeof(pathname), dirname, true);

	ret = cavan_http_write_path_hrefs(client, fd, pathname);
	if (ret < 0) {
		pr_red_info("cavan_http_write_path_html: %d", ret);
		goto out_close_fd;
	}

	ret |= ffile_puts(fd, "</h5>\r\n\t\t<h5>[<a href=\"..\">Parent</a>]");

	env = cavan_getenv("HOME", NULL);
	if (env != NULL) {
		ret |= ffile_printf(fd, " [<a href=\"%s/\">Home</a>]", env);
	}

	env = cavan_path_get_tmp_directory();
	if (env != NULL) {
		ret |= ffile_printf(fd, " [<a href=\"%s/\">Temp</a>]", env);
	}

	env = cavan_getenv("APP_PATH", NULL);
	if (env != NULL) {
		ret |= ffile_printf(fd, " [<a href=\"%s/\">App</a>]", env);
	}

	env = cavan_getenv("SHARED_PATH", NULL);
	if (env != NULL) {
		ret |= ffile_printf(fd, " [<a href=\"%s/\">Shared</a>]", env);
	}

	if (ret < 0) {
		pr_red_info("ffile_printf");
		goto out_close_fd;
	}

	for (i = 0; i < 10; i++) {
		char key[64];

		snprintf(key, sizeof(key), "SDCARD%d_PATH", i);

		env = cavan_getenv(key, NULL);
		if (env == NULL) {
			break;
		}

		ret = ffile_printf(fd, " [<a href=\"%s/\">SDcard%d</a>]", env, i);
		if (ret < 0) {
			pr_red_info("ffile_printf");
			goto out_close_fd;
		}
	}

	ret |= ffile_puts(fd, "</h5>\r\n");
	if (ret < 0) {
		pr_red_info("ffile_puts");
		goto out_close_fd;
	}

	count = scandir(dirname, &entries, NULL, alphasort);
	if (count < 0) {
		pr_err_info("opendir: %s", dirname);
		ret |= ffile_printf(fd, "\t\t<h5><font color=\"#FF0000\">Failed to open: %s</font></h5>\r\n", strerror(errno));
	} else {
		filter = text_fixup_empty_simple(filter);

		ret |= ffile_printf(fd, "\t\t<form enctype=\"multipart/form-data\" onsubmit=\"return onUploadSubmit(this)\" action=\"%s\" method=\"post\">\r\n", dirname);
		ret |= ffile_puts(fd, "\t\t\t<input type=\"submit\" value=\"Upload\" />\r\n");
		ret |= ffile_puts(fd, "\t\t\t<input id=\"upload\" name=\"pathname\" type=\"file\" />\r\n");
		ret |= ffile_puts(fd, "\t\t</form>\r\n");
		ret |= ffile_puts(fd, "\t\t<form method=\"get\">\r\n");
		ret |= ffile_printf(fd, "\t\t\t<input name=\"filter\" type=\"text\" value=\"%s\" />\r\n", text_fixup_null_simple(filter));
		ret |= ffile_puts(fd, "\t\t\t<input type=\"submit\" value=\"Search\" />\r\n");
		ret |= ffile_puts(fd, "\t\t</form>\r\n");
		ret |= ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
		ret |= ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

		if (ret < 0) {
			pr_red_info("ffile_puts");
			goto out_free_entries;
		}

		for (i = 0; i < count; i++) {
			char buff[32];
			struct tm time;
			const char *type;
			struct dirent *entry = entries[i];

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

			ret |= ffile_puts(fd, "\t\t\t<tr>");

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

			ret |= ffile_printf(fd, "<td>[%s]</td><td>", type);

			if ((st.st_mode & S_IFMT) == S_IFDIR) {
				ret |= ffile_printf(fd, "<a href=\"%s/\">%s</a>", entry->d_name, entry->d_name);
			} else {
				ret |= ffile_printf(fd, "<a href=\"%s\">%s</a>", entry->d_name, entry->d_name);
			}

			ret |= ffile_puts(fd, "</td><td>");
			ret |= ffile_write(fd, buff, mem_size_tostring_simple(st.st_size, buff, sizeof(buff)) - buff);

			if (localtime_r((time_t *) &st.st_mtime, &time) == NULL) {
				memset(&time, 0x00, sizeof(time));
			}

			ret |= ffile_printf(fd, "</td><td>%04d-%02d-%02d %02d:%02d:%02d</td>",
				time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

			ret |= ffile_printf(fd, "</tr>\r\n");

			if (ret < 0) {
				pr_red_info("ffile_printf");
				goto out_free_entries;
			}
		}

		ret |= ffile_puts(fd, "\t\t</table>\r\n");
	}

	if (ret < 0) {
		pr_red_info("ffile_puts");
		goto out_free_entries;
	}

	ret = cavan_http_flush_html_file(fd);
	if (ret < 0) {
		pr_red_info("cavan_http_flush_html_file: %d", ret);
		goto out_free_entries;
	}

	ret = cavan_http_send_html(client, fd);
	if (ret < 0) {
		pr_red_info("cavan_http_send_html");
	}

out_free_entries:
	if (count >= 0) {
		for (i = 0; i < count; i++) {
			free(entries[i]);
		}

		free(entries);
	}

out_close_fd:
	close(fd);
	return ret;
}

int cavan_http_process_get(struct network_client *client, struct cavan_http_request *req)
{
	int ret;
	struct stat st;

	ret = file_stat(req->url, &st);
	if (ret < 0) {
		if (errno == ENOENT) {
			cavan_http_send_replyf(client, 404, "File not found: %s", req->url);
			return ret;
		}

		return cavan_http_list_directory(client, req->url, NULL);
	}

	switch (st.st_mode & S_IFMT) {
	case S_IFREG:
		return cavan_http_send_file3(client, req->url, req->props, req->prop_used);

	case S_IFDIR:
		return cavan_http_list_directory(client, req->url, cavan_http_request_find_param_simple(req, "filter"));

	default:
		cavan_http_send_reply(client, 403, TEXT_PAIR("Invalid file type"));
		return -EINVAL;
	}
}

static const struct cavan_http_signin_info *cavan_http_signin_find(const char *app)
{
	const struct cavan_http_signin_info *p, *p_end;

	for (p = http_signin_infos, p_end = p + NELEM(http_signin_infos); p < p_end; p++) {
		if (strcasecmp(p->app, app) == 0) {
			return p;
		}
	}

	return NULL;
}

static u32 cavan_http_signin_get_delay(void)
{
	u64 time = clock_gettime_real_ms();

	return (time / 3600000 + 1) * 3600000 - time;
}

static int cavan_http_process_signin(struct network_client *client, struct cavan_http_request *req, const char *app)
{
	const struct cavan_http_signin_info *info;
	int ret;
	int fd;

	if (app == NULL) {
		app = cavan_http_request_find_param_simple(req, "app");
		if (app == NULL) {
			cavan_http_send_reply(client, 403, TEXT_PAIR("App not set"));
			return -ENOENT;
		}
	}

	info = cavan_http_signin_find(app);
	if (info == NULL) {
		cavan_http_send_replyf(client, 403, "App not found: %s", app);
		return -ENOENT;
	}

	fd = cavan_http_open_html_file(app, NULL);
	if (fd < 0) {
		pr_red_info("web_proxy_open_html_file");
		return fd;
	}

	ret = ffile_puts(fd, "\t\t<script type=\"text/javascript\">\r\n");
	ret |= ffile_printf(fd, "\t\t\tvar delay = %d;\r\n", cavan_http_signin_get_delay());
	ret |= ffile_puts(fd, "\t\t\tfunction doSignin() {\r\n");
	ret |= ffile_printf(fd, "\t\t\t\twindow.location = \"%s\";\r\n", info->url);
	ret |= ffile_puts(fd, "\t\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t\tfunction showDelay() {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tvar date = new Date();\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tdocument.all.time.innerHTML = \"Time: \" + date.toTimeString();\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tdocument.all.message.innerHTML = \"Delay: \" + delay + \" (ms)\";\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tif (delay > 1000) {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\t\tdelay -= 1000;\r\n");
	ret |= ffile_puts(fd, "\t\t\t\t} else {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\t\tdelay = 0;\r\n");
	ret |= ffile_puts(fd, "\t\t\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t\tvar date = new Date()\r\n");
	ret |= ffile_puts(fd, "\t\t\tif (date.getHours() < 23) {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tdoSignin()\r\n");
	ret |= ffile_puts(fd, "\t\t\t} else {\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tsetTimeout(\"doSignin()\", delay);\r\n");
	ret |= ffile_puts(fd, "\t\t\t\tsetInterval(\"showDelay()\", 1000);\r\n");
	ret |= ffile_puts(fd, "\t\t\t}\r\n");
	ret |= ffile_puts(fd, "\t\t</script>\r\n");
	ret |= ffile_puts(fd, "\t\t<h5 id=\"time\"></h5>\r\n");
	ret |= ffile_puts(fd, "\t\t<h5 id=\"message\"></h5>\r\n");
	ret |= ffile_printf(fd, "\t\t<button onclick=\"doSignin();\">Immediate Execution (%s)</button>\r\n", app);

	if (ret < 0) {
		pr_red_info("ffile_puts");
		goto out_close_fd;
	}

	ret = cavan_http_flush_html_file(fd);
	if (ret < 0) {
		pr_red_info("cavan_http_flush_html_file: %d", ret);
		goto out_close_fd;
	}

	ret = cavan_http_send_html(client, fd);
	if (ret < 0) {
		pr_red_info("cavan_http_send_html");
	}

out_close_fd:
	close(fd);
	return ret;
}

static int cavan_http_process_action(struct network_client *client, struct cavan_http_request *req, const char *action)
{
	if (text_lhcmp("signin", action) == 0) {
		if (action[6] == 0) {
			return cavan_http_process_signin(client, req, NULL);
		} else if (action[6] == '/') {
			return cavan_http_process_signin(client, req, action + 7);
		}
	} else if (strcmp("time", action) == 0) {
		time_t timstamp = time(NULL);
		struct tm *time;
		char buff[1024];
		int length;

		time = localtime(&timstamp);
		if (time == NULL) {
			return cavan_http_send_reply(client, 403, TEXT_PAIR("Failed to get localtime"));
		}

		length = strftime(buff, sizeof(buff), "%Y-%m-%d %T", time);

		return cavan_http_send_reply(client, 200, buff, length);
	} else if (strcmp("delay", action) == 0) {
		return cavan_http_send_replyf(client, 200, "%d", cavan_http_signin_get_delay());
	} else if (strcmp("timestamp", action) == 0) {
		return cavan_http_send_replyf(client, 200, "%" PRINT_FORMAT_INT64, clock_gettime_real_ms());
	} else if (strcmp("test", action) == 0) {
		return network_client_send_text(client, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
	}

	cavan_http_send_replyf(client, 403, "Invalid action: %s", action);

	return -EINVAL;
}

int cavan_http_read_multiform_header(struct cavan_fifo *fifo, struct cavan_http_request *header, const char *boundary)
{
	int ret;
	char *p;
	const char *disposition;

	p = cavan_fifo_read_line_strip(fifo, header->mem, header->mem_size);
	if (p == NULL) {
		pr_red_info("cavan_fifo_read_line_strip");
		return -EFAULT;
	}

	if (text_lhcmp("--", header->mem) || strcmp(header->mem + 2, boundary)) {
		pr_red_info("Invalid boundary: %s", header->mem);
		return -EINVAL;
	}

	header->mem_used = (p - header->mem) + 1;

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

char *cavan_http_basename(const char *pathname)
{
	const char *basename = pathname;

	while (1) {
		switch (*pathname++) {
		case '/':
		case '\\':
			basename = pathname;
			break;

		case 0:
			return (char *) basename;
		}
	}
}

int cavan_http_receive_file(struct cavan_fifo *fifo, struct cavan_http_request *header, const char *dirname, const char *boundary)
{
	int fd;
	int ret;
	size_t total;
	struct stat st;
	char pathname[1024];
	char *pathname_end;
	const char *mime_type;

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

	if (stat(dirname, &st) == 0 && S_ISDIR(st.st_mode)) {
		const char *filename = cavan_http_request_find_param_simple(header, "filename");
		if (filename == NULL) {
			pr_red_info("filename not found");
			return -EINVAL;
		}

		filename = cavan_http_basename(filename);
		if (filename[0] == 0) {
			pr_red_info("filename is empty");
			return -EINVAL;
		}

		pathname_end = cavan_path_cat(pathname, sizeof(pathname), dirname, filename, false);
	} else {
		pathname_end = cavan_path_copy(pathname, sizeof(pathname), dirname, false);
	}

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
		char *p;
		char buff[1024];

		p = cavan_fifo_read_line(fifo, buff, sizeof(buff));
		if (p == NULL) {
			ret = -EFAULT;
			pr_err_info("cavan_fifo_read_line");
			goto out_close_fd;
		}

		if (text_lhcmp("--", buff) == 0 &&
			text_lhcmp(boundary, buff + 2) == 0 &&
			text_lhcmp("--", p - 4) == 0) {
			break;
		}

		rdlen = p - buff;

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
	if (mime_type != NULL && strcmp(mime_type, http_mime_type_apk) == 0) {
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

int cavan_http_receive_file_simple(struct cavan_fifo *fifo, const char *pathname)
{
	int ret;
	int fd;

	pd_info("pathname = %s", pathname);

	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open: %s", pathname);
		return fd;
	}

	while (1) {
		char buff[1024];
		ssize_t rdlen;
		ssize_t wrlen;

		rdlen = cavan_fifo_read(fifo, buff, sizeof(buff));
		if (unlikely(rdlen <= 0)) {
			ret = rdlen;
			break;
		}

		wrlen = ffile_write(fd, buff, rdlen);
		if (wrlen < rdlen) {
			if (wrlen < 0) {
				ret = wrlen;
			} else {
				ret = -EFAULT;
			}

			break;
		}
	}

	close(fd);

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
	u64 time_start, time_consume;
	struct cavan_http_request *header;
	const char *boundary = cavan_http_get_boundary(req->props, req->prop_used);

	text = cavan_http_request_find_prop_simple(req, "Content-Length");
	if (text != NULL) {
		cavan_fifo_set_available(fifo, text2value_unsigned(text, NULL, 10));
	}

	header = cavan_http_request_alloc(1024, 10, 5);
	if (header == NULL) {
		pr_red_info("cavan_http_request_alloc");
		return -ENOMEM;
	}

	time_start = clock_gettime_mono_ms();

	if (boundary == NULL) {
		ret = cavan_http_receive_file_simple(fifo, req->url);
		if (ret < 0) {
			cavan_http_send_replyf(fifo->private_data, 403, "Failed to upload: %d", ret);
			goto out_cavan_http_request_free;
		}
	} else {
		while (1) {
			ret = cavan_http_receive_file(fifo, header, req->url, boundary);
			if (ret < 0) {
				cavan_http_send_replyf(fifo->private_data, 403, "Failed to upload: %d", ret);
				goto out_cavan_http_request_free;
			}

			if (cavan_fifo_get_remain(fifo) > 0) {
				cavan_http_request_reset(header);
			} else {
				break;
			}
		}
	}

	time_consume = clock_gettime_mono_ms() - time_start;
	if (time_consume == 0) {
		time_consume = 1;
	}

	if (fifo->available) {
		char speed_buff[128];
		char *p = speed_buff, *p_end = p + sizeof(speed_buff);
		double speed = (((double) fifo->available * 1000) / time_consume);

		p = mem_speed_tostring(speed, speed_buff, p_end - p);
		p = text_ncopy(p, " (", p_end - p);
		p = mem_size_tostring(fifo->available, p, p_end - p);
		p = text_ncopy(p, " in ", p_end - p);
		p = mem_time_tostring_ms(time_consume, p, p_end - p);
		p = text_ncopy(p, ")", p_end - p);

		ret = cavan_http_send_replyf(fifo->private_data, 200, "Upload successfull: %s", speed_buff);
	} else {
		ret = cavan_http_send_replyf(fifo->private_data, 200, "Upload successfull");
	}

out_cavan_http_request_free:
	cavan_http_request_free(header);
	return ret;
}

int cavan_http_process_propfind(struct cavan_fifo *fifo, struct cavan_http_request *req)
{
	int ret;
	char buff[1024];
	const char *text;

	text = cavan_http_request_find_prop_simple(req, "Content-Length");
	if (text != NULL) {
		cavan_fifo_set_available(fifo, text2value_unsigned(text, NULL, 10));
	}

	ret = cavan_fifo_fill(fifo, buff, sizeof(buff));
	if (ret < 0) {
		pr_red_info("cavan_fifo_fill: %d", ret);
		return ret;
	}

	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

	return 0;
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

	return network_service_accept(&http->service, conn, 0);
}

static bool cavan_http_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
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

	ret = cavan_fifo_init(&fifo, 4096, client);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		goto out_cavan_http_request_free;
	}

	fifo.read = network_client_fifo_read;

	while (1) {
		const char *keepalive;

		ret = cavan_http_read_request(&fifo, req);
		if (ret < 0) {
			goto out_cavan_fifo_deinit;
		}

#if CAVAN_HTTP_DEBUG
		cavan_http_dump_request(req);
#endif

		type = cavan_http_get_request_type2(req->type);
		switch (type) {
		case HTTP_REQ_GET:
			if (text_lhcmp("/action/", req->url) == 0) {
				cavan_http_process_action(client, req, req->url + 8);
				break;
			}

			cavan_http_process_get(client, req);
			break;

		case HTTP_REQ_POST:
			cavan_http_process_post(&fifo, req);
			break;

		case HTTP_REQ_PROPFIND:
			cavan_http_process_propfind(&fifo, req);
			break;

		case HTTP_REQ_HEAD:
			network_client_send_text(client, "HTTP/1.1 200 OK\r\n\r\n");
			goto out_cavan_fifo_deinit;

		default:
			goto out_cavan_fifo_deinit;
		}

		keepalive = cavan_http_request_find_prop_simple(req, "Connection");
		if (keepalive && strcasecmp(keepalive, "close") == 0) {
			break;
		}

		if (cavan_fifo_get_remain(&fifo)) {
			break;
		}

		// cavan_fifo_reset(&fifo);
		cavan_http_request_reset(req);
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

// ================================================================================

#if CONFIG_CAVAN_CURL
static size_t http_client_write_callback(char *buffer, size_t size, size_t nitems, void *outstream)
{
	size_t remain;
	struct cavan_http_stream *stream = outstream;

	remain = stream->size - stream->used;

	size *= nitems;
	if (size > remain) {
		size = remain;
	}

	memcpy(stream->buff + stream->used, buffer, size);
	stream->used += size;

	return size;
}

#if 0
static size_t http_client_read_callback(char *buffer, size_t size, size_t nitems, void *instream)
{
	size_t remain;
	struct cavan_http_stream *stream = instream;

	pr_pos_info();

	remain = stream->size - stream->used;

	size *= nitems;
	if (remain > size) {
		remain = size;
	}

	memcpy(buffer, stream->buff + stream->used, remain);
	stream->used += remain;

	return remain;
}
#endif

ssize_t http_client_send_request(const char *url, const char *post, const char *headers[], size_t header_size, void *rsp, size_t rsp_size)
{
	CURLcode err = 0;
	ssize_t length = -EFAULT;
	CURL *curl = curl_easy_init();
	struct curl_slist *slist = NULL;
	struct cavan_http_stream outstream;

	if(curl == NULL) {
		pr_red_info("curl_easy_init");
		return length;
	}

	if (headers && header_size > 0) {
		int i;

		for ( i = 0; i < (int) header_size; i++) {
			slist = curl_slist_append(slist, headers[i]);
		}

	    err |= curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	}

	if (post) {
	    err |= curl_easy_setopt(curl, CURLOPT_POST, 1);
	    err |= curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
	}

	outstream.used = 0;

	if (rsp && rsp_size > 0) {
		outstream.buff = rsp;
		outstream.size = rsp_size;
	    err |= curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outstream);
	    err |= curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_client_write_callback);
	}

	err |= curl_easy_setopt(curl, CURLOPT_URL, url);
    err |= curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

    err |= curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    err |= curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    err |= curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	err |= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	err |= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	if (err != CURLE_OK) {
		pr_red_info("curl_easy_setopt: %d", err);
		goto out_curl_easy_cleanup;
	}

	err = curl_easy_perform(curl);
	if (err != CURLE_OK) {
		pr_red_info("curl_easy_perform: %d", err);
		goto out_curl_easy_cleanup;
	}

	length = outstream.used;

out_curl_easy_cleanup:
	if (slist) {
		curl_slist_free_all(slist);
	}

    curl_easy_cleanup(curl);

    return length;
}
#else
ssize_t http_client_send_request(const char *url, const char *post, const char *headers[], size_t header_size, void *rsp, size_t rsp_size)
{
	pr_red_info("Need CONFIG_CAVAN_CURL");
	return -EFAULT;
}
#endif

// ================================================================================

void cavan_http_packet_init(struct cavan_http_packet *packet)
{
	int i;

	for (i = 0; i < HTTP_HEADER_COUNT; i++) {
		packet->headers[i].length = 0;
	}

	cavan_string_init(&packet->header, NULL, 0);
	cavan_string_init(&packet->body, NULL, 0);
	packet->lines = 0;
}

void cavan_http_packet_clear(struct cavan_http_packet *packet, bool depth)
{
	int i;

	for (i = 0; i < HTTP_HEADER_COUNT; i++) {
		packet->headers[i].length = 0;
	}

	cavan_string_clear(&packet->header, depth);
	cavan_string_clear(&packet->body, depth);
	packet->lines = 0;
}

struct cavan_http_packet *cavan_http_packet_alloc(void)
{
	struct cavan_http_packet *packet = malloc(sizeof(struct cavan_http_packet));

	if (packet != NULL) {
		cavan_http_packet_init(packet);
	}

	return packet;
}

void cavan_http_packet_free(struct cavan_http_packet *packet)
{
	cavan_http_packet_clear(packet, true);
	free(packet);
}

bool cavan_http_packet_content_printable(const struct cavan_http_packet *packet)
{
	char content_type[1024];

	if (cavan_http_packet_get_header(packet, HTTP_HEADER_CONTENT_TYPE, content_type, sizeof(content_type)) < 0) {
		return false;
	}

	if (text_lhcmp("text/", content_type) == 0) {
		return true;
	}

	if (strstr(content_type, "/json;") != NULL) {
		return true;
	}

	return false;
}

int cavan_http_packet_decode(const struct cavan_http_packet *packet, cavan_string_t *str)
{
	return 0;
}

void cavan_http_packet_dump(const struct cavan_http_packet *packet)
{
	const cavan_string_t *header = &packet->header;
	const cavan_string_t *body = &packet->body;

	cavan_stdout_write_string(header);

	if (body->length > 0 && cavan_http_packet_content_printable(packet)) {
		cavan_stdout_write_string(body);
	}
}

int cavan_http_packet_get_header(const struct cavan_http_packet *packet, int header, char *buff, int size)
{
	const struct cavan_http_header *p = packet->headers + header;
	u16 length = p->length;

	if (length > 0) {
		if (length < size) {
			buff[length] = 0;
		} else {
			length = size;
		}

		memcpy(buff, packet->header.text + p->offset, length);

		return length;
	}

	return -ENOENT;
}

int cavan_http_packet_add_line(struct cavan_http_packet *packet, const char *line, int size)
{
	int ret;
	int length;
	const char *line_end;
	const char *name, *value;
	cavan_string_t *header = &packet->header;
	int offset = header->length;

	ret = cavan_string_append_line_dos(header, line, size);
	if (ret < 0) {
		pr_red_info("cavan_string_append_line_dos");
		return ret;
	}

	if (++packet->lines < 2) {
		return 0;
	}

	name = line = header->text + offset;
	value = NULL;
	length = 0;

	for (line_end = line + size; line < line_end; line++) {
		switch (*line) {
		case ':':
			if (value == NULL) {
				length = line - name;
				value = line + 1;
			}
			break;

		case ' ':
		case '\t':
		case '\f':
			if (name == line) {
				name++;
			} else if (value == line) {
				value++;
			}
			break;

		default:
			if (value != NULL) {
				int type = cavan_http_get_header_type(name, length);
				if (type >= 0) {
					packet->headers[type].offset = value - header->text;
					packet->headers[type].length = line_end - value;
				}

				return 0;
			}
		}
	}

	return 0;
}

int cavan_http_packet_add_linef(struct cavan_http_packet *packet, const char *format, ...)
{
	char buff[4096];
	int length;
	va_list ap;

	va_start(ap, format);
	length = vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	return cavan_http_packet_add_line(packet, buff, length);
}

int cavan_http_packet_parse_file(const char *pathname, struct cavan_http_packet *packets[], int size)
{
	int ret;
	int count;
	int group;
	int group_len;
	size_t length;
	char *mem, *p, *p_end;
	char *line, *line_end;
	struct cavan_http_packet *packet;

	mem = file_read_all(pathname, 0, &length);
	if (mem == NULL) {
		pr_red_info("file_read_all: %s", pathname);
		return -EFAULT;
	}

	packet = cavan_http_packet_alloc();
	if (packet == NULL) {
		pr_red_info("cavan_http_packet_alloc");
		ret = -ENOMEM;
		goto out_free_mem;
	}

	group_len = 0;
	group = 0;
	count = 0;
	line = mem;
	line_end = NULL;

	for (p = mem, p_end = p + length; p < p_end; p++) {
		switch (*p) {
		case '\r':
			if (line_end == NULL) {
				line_end = p;
			}
			break;

		case '\n':
			if (line_end == NULL) {
				line_end = p;
			}

			if (line < line_end) {
				ret = cavan_http_packet_add_line(packet, line, line_end - line);
				if (ret < 0) {
					pr_red_info("cavan_string_append");
					goto out_free_packets;
				}
			} else if (packet->lines > 0) {
				ret = cavan_http_packet_add_line_end(packet);
				if (ret < 0) {
					pr_red_info("cavan_http_packet_add_line_end");
					goto out_free_packets;
				}

				packets[count++] = packet;
				packet->group = group;
				group_len++;

				packet = cavan_http_packet_alloc();
				if (packet == NULL) {
					pr_red_info("cavan_http_packet_alloc");
					ret = -ENOMEM;
					goto out_free_packets;
				}
			} else if (group_len > 0) {
				group_len = 0;
				group++;
			}

			line_end = NULL;
			line = p + 1;
			break;
		}
	}

	if (packet->lines > 0) {
		ret = cavan_http_packet_add_line_end(packet);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_add_line_end");
			goto out_free_packets;
		}

		packet->group = group;
		packets[count++] = packet;
	} else {
		cavan_http_packet_free(packet);
	}

	free(mem);

	return count;

out_free_packets:
	if (packet != NULL) {
		cavan_http_packet_free(packet);
	}

	while (count > 0) {
		cavan_http_packet_free(packets[--count]);
	}
out_free_mem:
	free(mem);
	return ret;
}

int cavan_http_packet_read_body(struct cavan_http_packet *packet, struct cavan_fifo *fifo)
{
	char content_length[32];

	if (cavan_http_packet_get_header(packet, HTTP_HEADER_CONTENT_LENGTH, content_length, sizeof(content_length)) > 0) {
		int length = text2value_unsigned(content_length, NULL, 10);
		cavan_string_t *body = &packet->body;
		int ret;

		ret = cavan_string_alloc(body, length, false);
		if (ret < 0) {
			pr_red_info("cavan_string_reinit");
			return ret;
		}

		if (length > 0) {
			ret = cavan_fifo_fill(fifo, body->text, length);
			if (ret < 0) {
				pr_red_info("cavan_fifo_fill");
				return ret;
			}
		}

		return length;
	}

	return 0;
}

int cavan_http_packet_read(struct cavan_http_packet *packet, struct cavan_fifo *fifo)
{
	cavan_http_packet_clear(packet, false);

	while (1) {
		cavan_string_t *line = cavan_fifo_read_line_string(fifo);
		int ret;

		if (line == NULL) {
			pr_red_info("cavan_fifo_read_line_string");
			return -EFAULT;
		}

		if (line->length > 0) {
			ret = cavan_http_packet_add_line(packet, line->text, line->length);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_add_line");
				return ret;
			}
		} else {
			ret = cavan_http_packet_add_line_end(packet);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_add_line_end");
				return ret;
			}

			ret = cavan_http_packet_read_body(packet, fifo);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_read_body");
				return ret;
			}

			break;
		}
	}

	return 0;
}

int cavan_http_packet_write(struct cavan_http_packet *packet, struct network_client *client)
{
	cavan_string_t *header = &packet->header;
	cavan_string_t *body = &packet->body;
	int ret;

	ret = network_client_send(client, header->text, header->length);
	if (ret < 0) {
		pr_red_info("network_client_send");
		return ret;
	}

	if (body->length > 0) {
		ret = network_client_send(client, body->text, body->length);
		if (ret < 0) {
			pr_red_info("network_client_send");
			return ret;
		}
	}

	return 0;
}
