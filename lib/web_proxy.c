/*
 * File:		web_proxy.c
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
#include <cavan/ftp.h>
#include <cavan/network.h>
#include <cavan/tcp_proxy.h>
#include <cavan/web_proxy.h>

#define WEB_PROXY_DEBUG		0

int web_proxy_get_request_type(const char *req, size_t length)
{
	switch (req[0])
	{
	case 'C':
		if (length == 7 && text_lhcmp("ONNECT", req + 1) == 0)
		{
			return HTTP_REQ_CONNECT;
		}
		break;

	case 'D':
		if (length == 6 && text_lhcmp("ELETE", req + 1) == 0)
		{
			return HTTP_REQ_DELETE;
		}
		break;

	case 'G':
		if (length == 3 && text_lhcmp("ET", req + 1) == 0)
		{
			return HTTP_REQ_GET;
		}
		break;

	case 'H':
		if (length == 4 && text_lhcmp("EAD", req + 1) == 0)
		{
			return HTTP_REQ_HEAD;
		}
		break;

	case 'O':
		if (length == 7 && text_lhcmp("PTIONS", req + 1) == 0)
		{
			return HTTP_REQ_OPTIONS;
		}
		break;

	case 'P':
		if (length == 3 && text_lhcmp("UT", req + 1) == 0)
		{
			return HTTP_REQ_PUT;
		}
		else if (length == 4 && text_lhcmp("OST", req + 1) == 0)
		{
			return HTTP_REQ_POST;
		}
		break;

	case 'T':
		if (length == 5 && text_lhcmp("RACE", req + 1) == 0)
		{
			return HTTP_REQ_TRACE;
		}
		break;
	}

	return -EINVAL;
}

const char *web_proxy_request_type_tostring(int type)
{
	switch (type)
	{
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

static int web_proxy_main_loop(struct network_client *client_src, struct network_client *client_dest, int timeout)
{
	int ret;
	ssize_t rwlen;
	char buff[2048];
	struct pollfd pfds[2] =
	{
		{
			.fd = client_src->sockfd,
			.events = POLLIN
		},
		{
			.fd = client_dest->sockfd,
			.events = POLLIN
		}
	};

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), timeout);
		if (ret <= 0)
		{
			if (ret < 0)
			{
				return ret;
			}

			return -ETIMEDOUT;
		}

		if (pfds[0].revents)
		{
			rwlen = client_src->recv(client_src, buff, sizeof(buff));
			if (rwlen <= 0 || client_dest->send(client_dest, buff, rwlen) < rwlen)
			{
				return -EIO;
			}
		}

		if (pfds[1].revents)
		{
			break;
		}
	}

	return 0;
}

static int web_proxy_ftp_send_http_reply(struct network_client *client, const char *filetype, size_t length, struct tm *time)
{
	char buff[2048], *p;

	p = text_copy(buff, "HTTP/1.1 200 Gatewaying\r\n"
		"Server: cavan-web_proxy\r\n"
		"Mime-Version: 1.0\r\n"
		"X-Cache: MISS from server\r\n"
		"Via: 1.1 server (cavan-web_proxy)\r\n");

	if (filetype == NULL)
	{
		filetype = "text/html";
	}

	p += sprintf(p, "Content-Type: %s\r\n", filetype);

#if __WORDSIZE == 64
	p += sprintf(p, "Content-Length: %ld\r\n", length) - 1;
#else
	p += sprintf(p, "Content-Length: %d\r\n", length) - 1;
#endif

	if (time)
	{
		p += sprintf(p, "Last-Modified: %s\r\n", asctime(time)) - 1;
	}

	p = text_copy(p, "Connection: keep-alive\r\n\r\n");

	return client->send(client, buff, p - buff);
}

static int web_proxy_ftp_read_file(struct network_client *client, struct network_client *client_proxy, const char *filename)
{
	int ret;
	size_t size;
	struct tm time;
	char buff[2048];
	size_t rdlen, wrlen;
	struct network_client client_data;

	ret = ftp_client_send_command2(client_proxy, buff, sizeof(buff), "MDTM %s\r\n", filename);
	if (ret != 213)
	{
		if (ret == 550)
		{
			errno = EISDIR;
		}

		pr_red_info("ftp_client_send_command2 MDTM");
		return -EFAULT;
	}

	if (strptime(buff, "%Y%m%d%H%M%S", &time) == NULL)
	{
		pr_red_info("invalid response %s", buff);
		return -EFAULT;
	}

	ret = ftp_client_send_command2(client_proxy, buff, sizeof(buff), "SIZE %s\r\n", filename);
	if (ret != 213)
	{
		pr_red_info("ftp_client_send_command2 SIZE");
		return -EFAULT;
	}

	size = text2value_unsigned(buff, NULL, 10);

	ret = ftp_client_create_pasv_link(client_proxy, &client_data);
	if (ret < 0)
	{
		pr_red_info("ftp_client_create_pasv_link");
		return ret;
	}

	ret = ftp_client_send_command2(client_proxy, NULL, 0, "RETR %s\r\n", filename);
	if (ret != 125 && ret != 150)
	{
		ret = -EIO;
		pr_red_info("ftp_client_send_command2");
		goto out_close_sockfd;
	}

	ret = web_proxy_ftp_send_http_reply(client, "text/plain", size, &time);
	if (ret < 0)
	{
		pr_red_info("web_proxy_ftp_send_http_reply");
		goto out_close_sockfd;
	}

	while (size)
	{
		rdlen = client_data.recv(&client_data, buff, size < sizeof(buff) ? size : sizeof(buff));
		if (rdlen <= 0)
		{
			ret = -EIO;
			goto out_close_sockfd;
		}

		wrlen = client->send(client, buff, rdlen);
		if (wrlen < rdlen)
		{
			ret = -EIO;
			goto out_close_sockfd;
		}

		size -= wrlen;
	}

	ret = ftp_client_read_response(client_proxy, NULL, 0);
	if (ret != 226)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
	}

out_close_sockfd:
	network_client_close(&client_data);
	return ret;
}

static int web_proxy_open_html_file(const char *title, char *pathname)
{
	int fd;
	char buff[1024];

	if (pathname == NULL)
	{
		pathname = buff;
	}

	cavan_build_temp_path("cavan-XXXXXX", pathname, sizeof(buff));
	fd = mkstemp(pathname);
	if (fd < 0)
	{
		pr_error_info("mkstemp `%s'", pathname);
		return fd;
	}

	unlink(pathname);

	println("pathname = %s, title = %s", pathname, title);

	ffile_puts(fd, "<!-- This file is automatic generate by Fuang.Cao -->\r\n\r\n");
	ffile_printf(fd, "<html>\r\n\t<head>\r\n\t\t<title>%s</title>\r\n\t</head>\r\n\t<body>\r\n", title);

	return fd;
}

static int web_proxy_flush_html_file(int fd)
{
	int ret;

	ret = ffile_puts(fd, "\t</body>\r\n</html>");
	if (ret < 0)
	{
		pr_red_info("ffile_puts");
		return ret;
	}

	return fsync(fd);
}

static int web_proxy_send_file(struct network_client *client, int fd, const char *filetype)
{
	int ret;
	struct stat st;

	if (lseek(fd, 0, SEEK_SET) != 0)
	{
		pr_error_info("lseek");
		return -EFAULT;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		pr_red_info("stat");
		return ret;
	}

	ret = web_proxy_ftp_send_http_reply(client, filetype, st.st_size, NULL);
	if (ret < 0)
	{
		pr_red_info("web_proxy_ftp_send_http_reply");
		return ret;
	}

	return network_client_send_file(client, fd, st.st_size);
}

static int web_proxy_ftp_list_directory(struct network_client *client, struct network_client *client_proxy, const struct network_url *url)
{
	int fd;
	int ret;
	char *p, buff[2048];
	struct cavan_fifo fifo;
	struct network_client client_data;

	ret = ftp_client_create_pasv_link(client_proxy, &client_data);
	if (ret < 0)
	{
		pr_red_info("ftp_client_create_pasv_link");
		return ret;
	}

	ret = ftp_client_send_command2(client_proxy, NULL, 0, "LIST\r\n");
	if (ret != 150)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_send_command2 LIST");
		goto out_close_data_sockfd;
	}

	network_url_tostring(url, buff, sizeof(buff), NULL);

	fd = web_proxy_open_html_file(buff, NULL);
	if (fd < 0)
	{
		ret = fd;
		pr_red_info("web_proxy_open_html_file");
		goto out_close_data_sockfd;
	}

	ffile_puts(fd, "\t\t<h1>FTP Proxy Server (Fuang.Cao <a href=\"http://mail.google.com\">cavan.cfa@gmail.com</a>)</h1>\r\n");
	ffile_printf(fd, "\t\t<h2>Directory: <a href=\"%s/\">%s</a></h2>\r\n", buff, buff);

	p = text_dirname_base(buff, url->pathname);
	if (p == buff || p[-1] != '/')
	{
		*p++ = '/';
		*p = 0;
	}

	ffile_printf(fd, "\t\t<h2><a href=\"%s\">Parent directory</a> (<a href=\"/\">Root directory</a>)</h2>\r\n", buff);
	ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

	ret = cavan_fifo_init(&fifo, sizeof(buff), &client_data);
	if (ret < 0)
	{
		pr_red_info("cavan_fifo_init");
		goto out_close_fd;
	}

	fifo.read = network_client_fifo_read;

	while (1)
	{
		int count;
		char *texts[16];

		ret = cavan_fifo_read_line(&fifo, buff, sizeof(buff));
		if (ret < 0)
		{
			pr_red_info("file_read_line");
			goto out_cavan_fifo_deinit;
		}

		if (ret == 0)
		{
			break;
		}

		count = text_split_by_space(buff, texts, NELEM(texts));
		if (count < 9)
		{
			pr_red_info("invalid line %s", buff);
			continue;
		}

		ffile_puts(fd, "\t\t\t<tr class=\"entry\">");

		switch (texts[0][0])
		{
			case 'l':
				p = "LINK";
				break;

			case 'f':
				p = "FIFO";
				break;

			case 'c':
				p = "CHR";
				break;

			case 'b':
				p = "BLK";
				break;

			case 'd':
				p = "DIR";
				break;

			default:
				p = "FILE";
		}

		ffile_printf(fd, "<td class=\"type\">[%s]</td><td class=\"filename\">", p);

		if (texts[0][0] == 'd')
		{
			ffile_printf(fd, "<a href=\"%s/\">%s</a>", texts[8], texts[8]);
		}
		else
		{
			ffile_printf(fd, "<a href=\"%s\">%s</a>", texts[8], texts[8]);
		}

		if (texts[0][0] == 'l' && count > 10)
		{
			ffile_printf(fd, " -> <a href=\"%s\">%s</a>", texts[10], texts[10]);
		}

		ffile_printf(fd, "</td><td class=\"size\">%s</td>", texts[4]);
		ffile_printf(fd, "<td class=\"date\">%s %s %s</td>", texts[5], texts[6], texts[7]);
		ffile_printf(fd, "</tr>\r\n");
	}

	ffile_puts(fd, "\t\t</table>\r\n");
	web_proxy_flush_html_file(fd);

	ret = ftp_client_read_response(client_proxy, NULL, 0);
	if (ret != 226)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
		goto out_cavan_fifo_deinit;
	}

	ret = web_proxy_send_file(client, fd, NULL);
	if (ret < 0)
	{
		pr_red_info("stat");
	}

out_cavan_fifo_deinit:
	cavan_fifo_deinit(&fifo);
out_close_fd:
	close(fd);
out_close_data_sockfd:
	network_client_close(&client_data);
	return ret;
}

static int web_proxy_send_connect_failed(struct network_client *client, struct network_url *url)
{
	int fd;
	int ret;
	char buff[1024];

	fd = web_proxy_open_html_file("connection failure", NULL);
	if (fd < 0)
	{
		pr_red_info("web_proxy_open_html_file");
		return fd;
	}

	ffile_printf(fd, "\t\t<h1>Unable to connect to %s</h1>\r\n", network_url_tostring(url, buff, sizeof(buff), NULL));
	ffile_puts(fd, "\t\t<h1>Please contact with Fuang.Cao</h1>\r\n");
	ffile_puts(fd, "\t\t<table>\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><strong>E-mail: </strong></td><td><a href=\"http://mail.google.com\">cavan.cfa@gmail.com</a>; <a href=\"http://mail.hipad.hk:6080\">caofuang@hipad.hk</a></td></tr>\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><strong>QQ: </strong></td><td><a href=\"http://web2.qq.com/\">870863613</a></td></tr>\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><strong>Website: </strong></td><td><a href=\"http://www.hipad.hk\">http://www.hipad.hk</a></td></tr>\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><strong>Source: </strong></td><td><a href=\"https://github.com/FuangCao/cavan.git\">https://github.com/FuangCao/cavan.git</a></td></tr>\r\n");
	ffile_puts(fd, "\t\t</table>\r\n");

	ret = web_proxy_flush_html_file(fd);
	if (ret < 0)
	{
		pr_red_info("web_proxy_flush_html_file");
		goto out_close_fd;
	}

	ret = web_proxy_send_file(client, fd, NULL);

out_close_fd:
	close(fd);
	return ret;
}

static int web_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	return network_service_accept(&proxy->service, conn);
}

static void web_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int web_proxy_start_handler(struct cavan_dynamic_service *service)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	return network_service_open(&proxy->service, &proxy->url, 0);
}

static void web_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	network_service_close(&proxy->service);
}

char *web_proxy_find_http_prop(const char *req, const char *req_end, const char *name, size_t namelen)
{
	while ((size_t) (req_end - req) > namelen)
	{
		if (text_ncmp(req, name, namelen) == 0)
		{
			for (req += namelen; req < req_end && byte_is_space(*req); req++);

			if (*req == ':')
			{
				while (byte_is_space(*++req));

				return (char *) req;
			}
		}

		if (byte_is_lf(*req))
		{
			break;
		}

		while (req < req_end && *req++ != '\n');
	}

	return NULL;
}

char *web_proxy_set_http_prop(char *req, char *req_end, const char *name, int namelen, const char *value, int valuelen)
{
	char *prop;
	int proplen;
	char *prop_end;

	prop = web_proxy_find_http_prop(req, req_end, name, namelen);
	if (prop == NULL)
	{
		return req_end;
	}

	for (prop_end = prop; prop_end < req_end && *prop_end != '\n'; prop_end++);

	proplen = prop_end - prop;
	if (proplen != valuelen)
	{
		mem_move(prop + valuelen, prop_end, req_end - prop_end);
	}

	mem_copy(prop, value, valuelen);

	return req_end + (valuelen - proplen);
}

ssize_t web_proxy_read_http_request(struct network_client *client, char *buff, size_t size)
{
	int count = 0;
	char *buff_bak = buff;
	char *buff_end = buff + size;

	while (buff < buff_end)
	{
		char *p;
		ssize_t rdlen;

		rdlen = client->recv(client, buff, buff_end - buff);
		if (rdlen <= 0)
		{
			return rdlen;
		}

		p = buff;
		buff += rdlen;

		while (p < buff)
		{
			switch (*p)
			{
			case '\n':
				if (count > 0)
				{
					goto out_return;
				}
				count++;
			case '\r':
				break;

			default:
				count = 0;
			}

			p++;
		}
	}

out_return:
	return buff - buff_bak;
}

static void web_proxy_close_proxy_client(struct network_client *client, bool ftp_login)
{
	if (client->sockfd < 0)
	{
		return;
	}

	if (ftp_login)
	{
		ftp_client_send_command2(client, NULL, 0, "QUIT\r\n");
	}

	network_client_close(client);
}

static int web_proxy_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	int type;
	int count;
	size_t cmdlen;
	char *filename;
	bool ftp_login;
	struct network_client client_proxy;
	struct network_client *client = conn;
	struct network_url urls[2], *url, *url_prev;
	char buff[2048], *buff_end, *req, *url_text;
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	count = 0;
	url = NULL;
	ftp_login = false;
	client_proxy.sockfd = -1;

	while (1)
	{
		bool tcp_proxy;

		ret = web_proxy_read_http_request(client, buff, sizeof(buff) - proxy->proxy_hostlen - 1);
		if (ret <= 0)
		{
			goto out_network_client_close_proxy;
		}

		buff_end = buff + ret;

#if WEB_PROXY_DEBUG
		*buff_end = 0;
		println("request is:\n%s", buff);
#endif

		for (url_text = buff; url_text < buff_end && *url_text != ' '; url_text++);

		cmdlen = url_text++ - buff;
		type = web_proxy_get_request_type(buff, cmdlen);
		if (type < 0)
		{
			pr_red_info("invalid request[%" PRINT_FORMAT_SIZE "] `%s'", cmdlen, buff);

			if (client_proxy.sockfd < 0)
			{
				ret = -EINVAL;
				goto out_return;
			}

			ret = network_client_send(&client_proxy, buff, buff_end - buff);
			if (ret <= 0)
			{
				pr_red_info("network_client_send");
				goto out_network_client_close_proxy;
			}

			goto label_web_proxy_main_loop;
		}

		url_prev = url;
		url = (url == urls) ? urls + 1 : urls;

		req = network_url_parse(url, url_text);
		if (req == NULL)
		{
			pr_red_info("web_proxy_parse_url:\n%s", url_text);
			ret = -EINVAL;
			goto out_network_client_close_proxy;
		}

		if (url->hostname[0])
		{
			if (url->protocol[0] == 0)
			{
				if (url->port == NETWORK_PORT_INVALID)
				{
					pr_red_info("invalid url %s", network_url_tostring(url, NULL, 0, NULL));
					ret = -EINVAL;
					goto label_web_proxy_main_loop;
				}

				url->protocol = "http";
			}

			tcp_proxy = false;
			pr_info("%s[%d](%d)", web_proxy_request_type_tostring(type), type, count);
		}
		else
		{
			tcp_proxy = true;
			url = &proxy->url_proxy;
		}

		if (url_prev == NULL || network_url_equals(url_prev, url) == false)
		{
			web_proxy_close_proxy_client(&client_proxy, ftp_login);

			ret = network_client_open(&client_proxy, url, 0);
			if (ret < 0)
			{
				pr_red_info("network_client_open");
				web_proxy_send_connect_failed(client, url);
				goto out_return;
			}

			count = 0;
			ftp_login = false;
		}
		else
		{
			count++;
			pr_green_info("Don't need connect %d", count);
		}

		switch (client_proxy.type)
		{
		case NETWORK_PROTOCOL_HTTP:
		case NETWORK_PROTOCOL_HTTPS:
			switch (type)
			{
			case HTTP_REQ_CONNECT:
				ret = network_client_send_text(client, "HTTP/1.1 200 Connection established\r\n\r\n");
				if (ret < 0)
				{
					pr_error_info("network_client_send_text");
					goto out_network_client_close_proxy;
				}

				tcp_proxy_main_loop(client, &client_proxy);
				goto out_network_client_close_proxy;

			default:
				if (tcp_proxy)
				{
					buff_end = web_proxy_set_http_prop(buff, buff_end, "Host", 4, proxy->url_proxy.hostname, proxy->proxy_hostlen);
#if WEB_PROXY_DEBUG
					*buff_end = 0;
#endif
					req = buff;
				}
				else
				{
					req -= cmdlen + 1;
					memcpy(req, buff, cmdlen);
					req[cmdlen] = ' ';
				}

#if WEB_PROXY_DEBUG
				println("New request is:\n%s", req);
#endif

				ret = network_client_send(&client_proxy, req, buff_end - req);
				if (ret < 0)
				{
					pr_error_info("network_client_send");
					goto out_network_client_close_proxy;
				}

label_web_proxy_main_loop:
				if (web_proxy_main_loop(client, &client_proxy, 60 * 1000) < 0)
				{
					goto out_network_client_close_proxy;
				}

				if (web_proxy_main_loop(&client_proxy, client, 60 * 1000) < 0)
				{
					goto out_network_client_close_proxy;
				}
			}
			break;

		case NETWORK_PROTOCOL_FTP:
			if (ftp_login)
			{
				pr_green_info("Don't need login");
			}
			else
			{
				ret = ftp_client_login(&client_proxy, NULL, NULL);
				if (ret < 0)
				{
					pr_red_info("ftp_client_login");
					goto out_network_client_close_proxy;
				}

				ftp_login = true;
			}

			ret = ftp_client_send_command2(&client_proxy, NULL, 0, "TYPE I\r\n");
			if (ret != 200)
			{
				pr_red_info("ftp_client_send_command2 TYPE A");
				ret = -EFAULT;
				goto out_network_client_close_proxy;
			}

			for (filename = req; filename < buff_end && text_lhcmp("HTTP", filename); filename++);
			for (filename -= 2, filename[1] = 0; *filename != '/'; filename--);

			*filename++ = 0;

label_change_dir:
#if WEB_PROXY_DEBUG
			println("dirname = %s, filename = %s", req, filename);
#endif

			ret = ftp_client_send_command2(&client_proxy, NULL, 0, "CWD %s\r\n", req[0] ? req : "/");
			if (ret != 250)
			{
				pr_red_info("ftp_client_send_command2 CWD");
				ret = -EFAULT;
				goto out_network_client_close_proxy;
			}

			switch (type)
			{
			case HTTP_REQ_GET:
				if (filename && filename[0])
				{
					ret = web_proxy_ftp_read_file(client, &client_proxy, filename);
					if (ret < 0)
					{
						pr_red_info("web_proxy_ftp_read_file `%s'", filename);

						if (errno == EISDIR)
						{
							filename[-1] = '/';
							filename = NULL;
							goto label_change_dir;
						}

						goto out_network_client_close_proxy;
					}
				}
				else
				{
					ret = web_proxy_ftp_list_directory(client, &client_proxy, url);
					if (ret < 0)
					{
						pr_red_info("web_proxy_ftp_list_directory `%s'", req);
						goto out_network_client_close_proxy;
					}
				}
				break;

			case HTTP_REQ_PUT:
				break;
			}
			break;

		default:
			pr_red_info("unsupport network protocol %s", network_protocol_tostring(client_proxy.type));
			ret = -EINVAL;
			goto out_network_client_close_proxy;
		}
	}

out_network_client_close_proxy:
	web_proxy_close_proxy_client(&client_proxy, ftp_login);
out_return:
	return ret;
}

int web_proxy_service_run(struct cavan_dynamic_service *service)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	pd_bold_info("PROXY_URL = %s", network_url_tostring(&proxy->url_proxy, NULL, 0, NULL));

	service->name = "WEB_PROXY";
	service->conn_size = sizeof(struct network_client);
	service->open_connect = web_proxy_open_connect;
	service->close_connect = web_proxy_close_connect;
	service->start = web_proxy_start_handler;
	service->stop = web_proxy_stop_handler;
	service->run = web_proxy_run_handler;

	proxy->proxy_hostlen = text_len(proxy->url_proxy.hostname);

	return cavan_dynamic_service_run(service);
}
