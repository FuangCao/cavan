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

#define CAVAN_HTTP_DEBUG	1

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

char *cavan_http_find_prop(const char *req, const char *req_end, const char *name, size_t namelen)
{
	while ((size_t) (req_end - req) > namelen) {
		if (text_ncmp(req, name, namelen) == 0) {
			for (req += namelen; req < req_end && byte_is_space(*req); req++);

			if (*req == ':') {
				while (byte_is_space(*++req));

				return (char *) req;
			}
		}

		if (cavan_islf(*req)) {
			break;
		}

		while (req < req_end && *req++ != '\n');
	}

	return NULL;
}

char *cavan_http_set_prop(char *req, char *req_end, const char *name, size_t namelen, const char *value, size_t valuelen)
{
	char *prop;
	size_t proplen;
	char *prop_end;

	prop = cavan_http_find_prop(req, req_end, name, namelen);
	if (prop == NULL) {
		return req_end;
	}

	for (prop_end = prop; prop_end < req_end && *prop_end != '\n'; prop_end++);

	proplen = prop_end - prop;
	if (proplen != valuelen) {
		mem_move(prop + valuelen, prop_end, req_end - prop_end);
	}

	mem_copy(prop, value, valuelen);

	return req_end + (valuelen - proplen);
}

ssize_t cavan_http_read_request(struct network_client *client, char *buff, size_t size)
{
	int count = 0;
	char *p, *p_end;

	for (p = buff, p_end = p + size; p < p_end; p++) {
		int ret = network_client_recv(client, p, 1);
		if (ret < 1) {
			if (ret < 0) {
				return ret;
			}

			break;
		}

		switch (*p) {
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
	}

out_complete:
	return p - buff;
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

size_t cavan_http_parse_url_props(char *url, struct cavan_http_prop *props, size_t propc)
{
	char *p;
	struct cavan_http_prop *prop, *prop_end;

	if (propc == 0) {
		return 0;
	}

	prop = props;
	prop_end = prop + propc;

	p = url;
	prop->key = url;
	prop->value = NULL;

	while (1) {
		switch (*p) {
		case '=':
			*url = 0;
			prop->value = url + 1;
			break;

		case '&':
			*url = 0;
			if (prop->value) {
				if (++prop < prop_end) {
					prop->key = url + 1;
					prop->value = NULL;
				} else {
					goto out_complete;
				}
			}
			break;

		case 0:
			*url = 0;
			if (prop->value) {
				prop++;
			}
			goto out_complete;

		case '%':
			*url = text2byte(p + 1);
			p += 2;
			break;

		default:
			*url = *p;
		}

		url++;
		p++;
	}

out_complete:
	return prop - props;
}

char *cavan_http_pathname_decode(char *pathname, struct cavan_http_prop *props, size_t *size)
{
	size_t count = 0;
	char *p = pathname;

	while (1) {
		switch (*p) {
		case '%':
			*pathname = text2byte(p + 1);
			p += 3;
			break;

		case '?':
			if (props && size) {
				count = cavan_http_parse_url_props(p + 1, props, *size);
			}
		case 0:
			goto out_complete;

		default:
			*pathname = *p++;
		}

		pathname++;
	}

out_complete:
	*pathname = 0;

	if (size) {
		*size = count;
	}

	return pathname;
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

int cavan_http_process_get(struct network_client *client, const char *pathname, struct cavan_http_prop *props, size_t propc, struct cavan_http_prop *args, size_t argc)
{
	int ret;
	struct stat st;

	ret = file_stat(pathname, &st);
	if (ret < 0) {
		cavan_http_send_reply(client, 404, "File not found: %s", pathname);
		return ret;
	}

	switch (st.st_mode & S_IFMT) {
	case S_IFREG:
		return cavan_http_send_file3(client, pathname, props, propc);

	case S_IFDIR:
		return cavan_http_list_directory(client, pathname);

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

int cavan_http_process_post(struct network_client *client, const char *pathname, struct cavan_http_prop *props, size_t propc, struct cavan_http_prop *args, size_t argc)
{
	const char *boundary = cavan_http_get_boundary(props, propc);

	if (boundary == NULL) {
		cavan_http_send_reply(client, 403, "boundary not found");
		return -EINVAL;
	}

	cavan_http_file_receive(client, pathname, boundary, 0);

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
	int length;
	size_t argc;
	size_t propc;
	char request[1024];
	char *req, *req_end;
	struct cavan_http_prop args[10];
	struct cavan_http_prop props[20];
	struct network_client *client = conn;

	while (1) {
		int type;
		int space = 0;
		int req_length = 0;
		char *pathname = NULL;

		length = cavan_http_read_request(client, request, sizeof(request));
		if (length <= 0) {
			if (length < 0) {
				pr_red_info("cavan_http_request_read: %d", length);
			}

			break;
		}

		request[length] = 0;

#if CAVAN_HTTP_DEBUG
		println("request[%d]:\n%s", length, request);
#endif

		for (req = request, req_end = req + length; req < req_end; req++) {
			if (cavan_isspace_lf(*req)) {
				*req = 0;

				if (space > 0) {
					break;
				}

				req_length = req - request;
				pathname = req + 1;
				space++;
			}
		}

		if (pathname == NULL) {
			break;
		}

		argc = NELEM(args);
		cavan_http_pathname_decode(pathname, args, &argc);
		type = cavan_http_get_request_type(request, req_length);

		req = text_skip_line(req + 1, req_end);
		propc = cavan_http_parse_request(req, req_end, props, NELEM(props));

#if CAVAN_HTTP_DEBUG
		println("request[%d] = %s, pathname = %s", type, request, pathname);
		println("props:");
		cavan_http_dump_props(props, propc);
		println("args:");
		cavan_http_dump_props(args, argc);
#endif

		switch (type) {
		case HTTP_REQ_GET:
			ret = cavan_http_process_get(client, pathname, props, propc, args, argc);
			if (ret < 0) {
				pr_red_info("cavan_http_process_get: %d", ret);
				return ret;
			}
			break;

		case HTTP_REQ_POST:
			ret = cavan_http_process_post(client, pathname, props, propc, args, NELEM(args));
			if (ret < 0) {
				pr_red_info("cavan_http_process_post: %d", ret);
				return ret;
			}
			break;
		}
	}

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
