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

int web_proxy_get_request_type(const char *req)
{
	switch (req[0])
	{
	case 'C':
		if (strcmp(req + 1, "ONNECT") == 0)
		{
			return HTTP_REQ_CONNECT;
		}
		break;

	case 'D':
		if (strcmp(req + 1, "ELETE") == 0)
		{
			return HTTP_REQ_DELETE;
		}
		break;

	case 'G':
		if (strcmp(req + 1, "ET") == 0)
		{
			return HTTP_REQ_GET;
		}
		break;

	case 'H':
		if (strcmp(req + 1, "EAD") == 0)
		{
			return HTTP_REQ_HEAD;
		}
		break;

	case 'O':
		if (strcmp(req + 1, "PTIONS") == 0)
		{
			return HTTP_REQ_OPTIONS;
		}
		break;

	case 'P':
		if (strcmp(req + 1, "UT") == 0)
		{
			return HTTP_REQ_PUT;
		}
		else if (strcmp(req + 1, "OST") == 0)
		{
			return HTTP_REQ_POST;
		}
		break;

	case 'T':
		if (strcmp(req + 1, "RACE") == 0)
		{
			return HTTP_REQ_TRACE;
		}
		break;
	}

	return -EINVAL;
}

static int web_proxy_main_loop(int srcfd, int destfd, int timeout)
{
	int ret;
	ssize_t rwlen;
	char buff[2048];
	struct pollfd pfds[2] =
	{
		{
			.fd = srcfd,
			.events = POLLIN
		},
		{
			.fd = destfd,
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
			rwlen = inet_recv(srcfd, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(destfd, buff, rwlen) < rwlen)
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

static int web_proxy_ftp_send_http_reply(int sockfd, const char *filetype, size_t length, struct tm *time)
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

	return inet_send(sockfd, buff, p - buff);
}

static int web_proxy_ftp_read_file(int client_sockfd, int proxy_sockfd, const char *filename)
{
	int ret;
	int sockfd;
	size_t size;
	struct tm time;
	size_t rdlen, wrlen;
	char buff[2048];

	ret = ftp_client_send_command2(proxy_sockfd, buff, sizeof(buff), "MDTM %s\r\n", filename);
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

	ret = ftp_client_send_command2(proxy_sockfd, buff, sizeof(buff), "SIZE %s\r\n", filename);
	if (ret != 213)
	{
		pr_red_info("ftp_client_send_command2 SIZE");
		return -EFAULT;
	}

	size = text2value_unsigned(buff, NULL, 10);

	sockfd = ftp_client_create_pasv_link(proxy_sockfd);
	if (sockfd < 0)
	{
		pr_red_info("ftp_client_create_pasv_link");
		return sockfd;
	}

	ret = ftp_client_send_command2(proxy_sockfd, NULL, 0, "RETR %s\r\n", filename);
	if (ret != 125 && ret != 150)
	{
		ret = -EIO;
		pr_red_info("ftp_client_send_command2");
		goto out_close_sockfd;
	}

	ret = web_proxy_ftp_send_http_reply(client_sockfd, "text/plain", size, &time);
	if (ret < 0)
	{
		pr_red_info("web_proxy_ftp_send_http_reply");
		goto out_close_sockfd;
	}

	while (size)
	{
		rdlen = inet_recv(sockfd, buff, size < sizeof(buff) ? size : sizeof(buff));
		if (rdlen <= 0)
		{
			ret = -EIO;
			goto out_close_sockfd;
		}

		wrlen = inet_send(client_sockfd, buff, rdlen);
		if (wrlen < rdlen)
		{
			ret = -EIO;
			goto out_close_sockfd;
		}

		size -= wrlen;
	}

	ret = ftp_client_read_response(proxy_sockfd, NULL, 0);
	if (ret != 226)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
	}

out_close_sockfd:
	inet_close_tcp_socket(sockfd);
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

	text_copy(pathname, CAVAN_TEMP_PATH "/cavan-XXXXXX");
	fd = mkstemp(pathname);
	if (fd < 0)
	{
		pr_error_info("mkstemp `%s'", pathname);
		return fd;
	}

	unlink(buff);

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

static int web_proxy_send_file(int sockfd, int fd, const char *filetype)
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

	ret = web_proxy_ftp_send_http_reply(sockfd, filetype, st.st_size, NULL);
	if (ret < 0)
	{
		pr_red_info("web_proxy_ftp_send_http_reply");
		return ret;
	}

	return inet_tcp_send_file1(sockfd, fd);
}

static int web_proxy_ftp_list_directory(int client_sockfd, int proxy_sockfd, struct network_url *url, const char *dirname)
{
	int fd;
	int ret;
	int data_sockfd;
	struct stat st;
	char *p, buff[2048];

	data_sockfd = ftp_client_create_pasv_link(proxy_sockfd);
	if (data_sockfd < 0)
	{
		pr_red_info("ftp_client_create_pasv_link");
		return data_sockfd;
	}

	ret = ftp_client_send_command2(proxy_sockfd, NULL, 0, "LIST\r\n");
	if (ret != 150)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_send_command2 LIST");
		goto out_close_data_sockfd;
	}

	network_url_tostring(url, buff, sizeof(buff), &p);
	text_copy(p, dirname);

	fd = web_proxy_open_html_file(buff, NULL);
	if (fd < 0)
	{
		ret = fd;
		pr_red_info("web_proxy_open_html_file");
		goto out_close_data_sockfd;
	}

	*p = 0;

	ffile_puts(fd, "\t\t<h1>FTP Proxy Server (Fuang.Cao <a href=\"http://mail.google.com\">cavan.cfa@gmail.com</a>)</h1>\r\n");
	ffile_printf(fd, "\t\t<h2>Directory: <a href=\"%s%s/\">%s%s</a></h2>\r\n", buff, dirname, buff, dirname);

	p = text_dirname_base(buff, dirname);
	if (p == buff || p[-1] != '/')
	{
		*p++ = '/';
		*p = 0;
	}

	ffile_printf(fd, "\t\t<h2><a href=\"%s\">Parent directory</a> (<a href=\"/\">Root directory</a>)</h2>\r\n", buff);
	ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

	while (1)
	{
		int count;
		char *texts[16];

		ret = file_read_line(data_sockfd, buff, sizeof(buff));
		if (ret < 0)
		{
			pr_red_info("file_read_line");
			goto out_close_fd;
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

	ret = ftp_client_read_response(proxy_sockfd, NULL, 0);
	if (ret != 226)
	{
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
		goto out_close_fd;
	}

	ret = web_proxy_send_file(client_sockfd, fd, NULL);
	if (ret < 0)
	{
		pr_red_info("stat");
	}

out_close_fd:
	close(fd);
out_close_data_sockfd:
	close(data_sockfd);
	return ret;
}

static int web_proxy_send_connect_filed(int sockfd, struct network_url *url)
{
	int fd;
	int ret;
	char buff[1024];

	fd = web_proxy_open_html_file("connect failed", NULL);
	if (fd < 0)
	{
		pr_red_info("web_proxy_open_html_file");
		return fd;
	}

	ffile_printf(fd, "\t\t<h1>Can't connect to %s</h1>\r\n", network_url_tostring(url, buff, sizeof(buff), NULL));
	ffile_puts(fd, "\t\t<h2>Fuang.Cao <a href=\"http://mail.google.com\">cavan.cfa@gmail.com</a></h2>\r\n");

	ret = web_proxy_flush_html_file(fd);
	if (ret < 0)
	{
		pr_red_info("web_proxy_flush_html_file");
		goto out_close_fd;
	}

	ret = web_proxy_send_file(sockfd, fd, NULL);

out_close_fd:
	close(fd);
	return ret;
}

static void *web_proxy_open_connect(struct cavan_dynamic_service *service)
{
	struct inet_connect *conn;
	struct web_proxy_service *proxy;

	conn = malloc(sizeof(struct inet_connect));
	if (conn == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	proxy = cavan_dynamic_service_get_data(service);
	conn->sockfd = inet_accept(proxy->sockfd, &conn->addr, &conn->addrlen);
	if (conn->sockfd < 0)
	{
		pr_error_info("inet_accept");
		free(conn);
		return NULL;
	}

	inet_show_sockaddr(&conn->addr);

	return conn;
}

static void web_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct inet_connect *client = conn;

	inet_close_tcp_socket(client->sockfd);
	free(client);
}

static int web_proxy_start_handler(struct cavan_dynamic_service *service)
{
	int sockfd;
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	sockfd = inet_create_tcp_service(proxy->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	proxy->sockfd = sockfd;

	return 0;
}

static void web_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	inet_close_tcp_socket(proxy->sockfd);
}

static int web_proxy_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	int type;
	int count;
	ssize_t rwlen;
	size_t cmdlen;
	char *filename;
	bool ftp_login;
	int proxy_sockfd;
	char buff[2048], *buff_end, *req, *url_text;
	struct network_url urls[2], *url, *url_bak;
	const struct network_protocol *protocol = NULL;
	struct inet_connect *client = conn;

	count = 0;
	ftp_login = false;
	proxy_sockfd = -1;
	url = urls;
	url_bak = url + 1;

	while (1)
	{
		rwlen = inet_recv(client->sockfd, buff, sizeof(buff) - 1);
		if (rwlen <= 0)
		{
			break;
		}

		buff[rwlen] = 0;
		buff_end = buff + rwlen;

#if WEB_PROXY_DEBUG
		println("request is:\n%s", buff);
#endif

		for (url_text = buff; url_text < buff_end && *url_text != ' '; url_text++);

		cmdlen = url_text - buff;
		*url_text++ = 0;

		type = web_proxy_get_request_type(buff);
		if (type < 0)
		{
			pr_red_info("invalid request %s", buff);
			break;
		}

		req = network_parse_url(url_text, url);
		if (req == NULL)
		{
			pr_red_info("web_proxy_parse_url:\n%s", url_text);
			break;
		}

		pr_std_info("%s[%d](%d) => %s", buff, type, count, network_url_tostring(url, NULL, 0, NULL));

		if (proxy_sockfd < 0 || network_url_equals(url_bak, url) == false)
		{
			if (proxy_sockfd >= 0)
			{
				inet_close_tcp_socket(proxy_sockfd);
			}

			protocol = network_get_protocol_by_name(url->protocol);
			ret = network_get_port_by_url(url, protocol);
			if (ret < 0)
			{
				pr_red_info("network_get_port_by_url");
				break;
			}

			proxy_sockfd = inet_create_tcp_link2(url->hostname, ret);
			if (proxy_sockfd < 0)
			{
				pr_red_info("inet_create_tcp_link2");
				web_proxy_send_connect_filed(client->sockfd, url);
				break;
			}

			count = 0;

			if (url == urls)
			{
				url_bak = urls;
				url = url_bak + 1;
			}
			else
			{
				url = urls;
				url_bak = url + 1;
			}

			ftp_login = false;
		}
		else
		{
			pr_green_info("Don't need connect");
			count++;
		}

		if (protocol == NULL)
		{
			pr_red_info("invalid protocol");
			goto out_close_client_sockfd;
		}

		switch (protocol->type)
		{
		case NETWORK_PROTOCOL_HTTP:
		case NETWORK_PROTOCOL_HTTPS:
			switch (type)
			{
			case HTTP_REQ_CONNECT:
				rwlen = inet_send_text(client->sockfd, "HTTP/1.1 200 Connection established\r\n\r\n");
				if (rwlen < 0)
				{
					pr_error_info("inet_send");
					goto out_close_client_sockfd;
				}

				tcp_proxy_main_loop(client->sockfd, proxy_sockfd);
				goto out_close_client_sockfd;

			default:
				req -= cmdlen + 1;
				memcpy(req, buff, cmdlen);
				req[cmdlen] = ' ';

#if WEB_PROXY_DEBUG
				println("New request is:\n%s", req);
#endif

				rwlen = inet_send(proxy_sockfd, req, buff_end - req);
				if (rwlen < 0)
				{
					pr_error_info("inet_send");
					goto out_close_client_sockfd;
				}

				ret = web_proxy_main_loop(client->sockfd, proxy_sockfd, 60 * 1000);
				if (ret < 0)
				{
					goto out_close_client_sockfd;
				}

				ret = web_proxy_main_loop(proxy_sockfd, client->sockfd, 60 * 1000);
				if (ret < 0)
				{
					goto out_close_client_sockfd;
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
				ret = ftp_client_login(proxy_sockfd, NULL, NULL);
				if (ret < 0)
				{
					pr_red_info("ftp_client_login");
					goto out_close_client_sockfd;
				}

				ftp_login = true;
			}

			ret = ftp_client_send_command2(proxy_sockfd, NULL, 0, "TYPE I\r\n");
			if (ret != 200)
			{
				pr_red_info("ftp_client_send_command2 TYPE A");
				goto out_close_client_sockfd;
			}

			for (filename = req; filename < buff_end && text_lhcmp("HTTP", filename); filename++);
			for (filename -= 2, filename[1] = 0; *filename != '/'; filename--);

			*filename++ = 0;

label_change_dir:
#if WEB_PROXY_DEBUG
			println("dirname = %s, filename = %s", req, filename);
#endif

			ret = ftp_client_send_command2(proxy_sockfd, NULL, 0, "CWD %s\r\n", req[0] ? req : "/");
			if (ret != 250)
			{
				pr_red_info("ftp_client_send_command2 CWD");
				goto out_close_client_sockfd;
			}

			switch (type)
			{
			case HTTP_REQ_GET:
				if (filename && filename[0])
				{
					ret = web_proxy_ftp_read_file(client->sockfd, proxy_sockfd, filename);
					if (ret < 0)
					{
						pr_red_info("web_proxy_ftp_read_file `%s'", filename);

						if (errno == EISDIR)
						{
							filename[-1] = '/';
							filename = NULL;
							goto label_change_dir;
						}

						goto out_close_client_sockfd;
					}
				}
				else
				{
					ret = web_proxy_ftp_list_directory(client->sockfd, proxy_sockfd, url_bak, req);
					if (ret < 0)
					{
						pr_red_info("web_proxy_ftp_list_directory `%s'", req);
						goto out_close_client_sockfd;
					}
				}
				break;

			case HTTP_REQ_PUT:
				break;
			}
			break;

		default:
			pr_red_info("unsupport network protocol %s", protocol->name);
			goto out_close_client_sockfd;
		}
	}

out_close_client_sockfd:
	if (proxy_sockfd >= 0)
	{
		if (ftp_login)
		{
			ftp_client_send_command2(proxy_sockfd, NULL, 0, "QUIT\r\n");
		}

		inet_close_tcp_socket(proxy_sockfd);
	}

	return 0;
}

int web_proxy_service_run(struct cavan_dynamic_service *service)
{
	service->name = "WEB_PROXY";
	service->open_connect = web_proxy_open_connect;
	service->close_connect = web_proxy_close_connect;
	service->start = web_proxy_start_handler;
	service->stop = web_proxy_stop_handler;
	service->run = web_proxy_run_handler;

	return cavan_dynamic_service_run(service);
}
