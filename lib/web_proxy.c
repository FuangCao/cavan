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
#include <cavan/http.h>
#include <cavan/tcp_proxy.h>
#include <cavan/web_proxy.h>

#define WEB_PROXY_DEBUG		0
#define WEB_PROXY_OVERTIME	(60000)

char *web_proxy_find_prop(const char *req, const char *req_end, const char *name, size_t namelen)
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

char *web_proxy_set_prop(char *req, char *req_end, const char *name, size_t namelen, const char *value, size_t valuelen)
{
	char *prop;
	size_t proplen;
	char *prop_end;

	prop = web_proxy_find_prop(req, req_end, name, namelen);
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

ssize_t web_proxy_read_request(struct network_client *client, char *buff, size_t size)
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
				p++;
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

// ================================================================================

static void web_proxy_show_request(const struct cavan_http_packet *req, const struct cavan_http_packet *rsp)
{
	cavan_lock_acquire(&cavan_stdout_lock);
	print_sep(80);
	cavan_http_packet_dump(req);
	cavan_http_packet_dump(rsp);
	cavan_lock_release(&cavan_stdout_lock);
}

int web_proxy_main_loop(struct network_client *reader, struct network_client *writer, int timeout)
{
	struct pollfd pfds[2] = {
		{
			.fd = reader->sockfd,
			.events = POLLIN
		}, {
			.fd = writer->sockfd,
			.events = POLLIN
		}
	};

	while (1) {
		int ret;
		ssize_t rwlen;
		char buff[2048];

		ret = poll(pfds, NELEM(pfds), timeout);
		if (ret <= 0) {
			if (ret < 0) {
				return ret;
			}

			return -ETIMEDOUT;
		}

		if (pfds[0].revents) {
			rwlen = reader->recv(reader, buff, sizeof(buff));
			if (rwlen <= 0 || writer->send(writer, buff, rwlen) < rwlen) {
				return -EIO;
			}
		}

		if (pfds[1].revents) {
			break;
		}
	}

	return 0;
}

int web_proxy_main_loop_cached(struct cavan_fifo *reader, struct network_client *writer, int timeout)
{
	int ret;

	ret = network_client_fifo_read_cache(reader, writer);
	if (ret < 0) {
		pr_red_info("network_client_fifo_read_cache");
		return ret;
	}

	return web_proxy_main_loop(reader->private_data, writer, timeout);
}

int web_proxy_monitor_loop(struct network_client *local, struct network_client *remote)
{
	struct cavan_http_packet req;
	struct cavan_http_packet rsp;
	struct cavan_fifo fifo_remote;
	struct cavan_fifo fifo_local;
	int ret;

	ret = network_client_fifo_init(&fifo_local, 4096, local);
	if (ret < 0) {
		pr_red_info("network_client_fifo_init");
		return ret;
	}

	ret = network_client_fifo_init(&fifo_remote, 4096, remote);
	if (ret < 0) {
		pr_red_info("network_client_fifo_init");
		goto out_fifo_deinit_local;
	}

	cavan_http_packet_init(&req);
	cavan_http_packet_init(&rsp);

	while (1) {
		ret = cavan_http_packet_read(&req, &fifo_local);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_read");
			break;
		}

		ret = cavan_http_packet_write(&req, remote);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_write");
			break;
		}

		ret = web_proxy_main_loop_cached(&fifo_local, remote, WEB_PROXY_OVERTIME);
		if (ret < 0) {
			pr_red_info("web_proxy_main_loop");
			break;
		}

		ret = cavan_http_packet_read(&rsp, &fifo_remote);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_read");
			break;
		}

		ret = cavan_http_packet_write(&rsp, local);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_write");
			break;
		}

		web_proxy_show_request(&req, &rsp);

		ret = web_proxy_main_loop_cached(&fifo_remote, local, WEB_PROXY_OVERTIME);
		if (ret < 0) {
			pr_red_info("web_proxy_main_loop");
			break;
		}
	}

	cavan_fifo_deinit(&fifo_remote);
out_fifo_deinit_local:
	cavan_fifo_deinit(&fifo_local);
	return ret;
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
	if (ret != 213) {
		if (ret == 550) {
			errno = EISDIR;
		}

		pr_red_info("ftp_client_send_command2 MDTM");
		return -EFAULT;
	}

	if (strptime(buff, "%Y%m%d%H%M%S", &time) == NULL) {
		pr_red_info("invalid response %s", buff);
		return -EFAULT;
	}

	ret = ftp_client_send_command2(client_proxy, buff, sizeof(buff), "SIZE %s\r\n", filename);
	if (ret != 213) {
		pr_red_info("ftp_client_send_command2 SIZE");
		return -EFAULT;
	}

	size = text2value_unsigned(buff, NULL, 10);

	ret = ftp_client_create_pasv_link(client_proxy, &client_data);
	if (ret < 0) {
		pr_red_info("ftp_client_create_pasv_link");
		return ret;
	}

	ret = ftp_client_send_command2(client_proxy, NULL, 0, "RETR %s\r\n", filename);
	if (ret != 125 && ret != 150) {
		ret = -EIO;
		pr_red_info("ftp_client_send_command2");
		goto out_close_sockfd;
	}

	ret = cavan_http_send_file_header(client, NULL, &time, 0, 0, size);
	if (ret < 0) {
		pr_red_info("web_proxy_ftp_send_http_reply");
		goto out_close_sockfd;
	}

	while (size) {
		rdlen = client_data.recv(&client_data, buff, size < sizeof(buff) ? size : sizeof(buff));
		if (rdlen <= 0) {
			ret = -EIO;
			goto out_close_sockfd;
		}

		wrlen = client->send(client, buff, rdlen);
		if (wrlen < rdlen) {
			ret = -EIO;
			goto out_close_sockfd;
		}

		size -= wrlen;
	}

	ret = ftp_client_read_response(client_proxy, NULL, 0);
	if (ret != 226) {
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
	}

out_close_sockfd:
	network_client_close(&client_data);
	return ret;
}

static int web_proxy_ftp_list_directory(struct network_client *client, struct network_client *client_proxy, const struct network_url *url)
{
	int fd;
	int ret;
	char *p, buff[2048];
	struct cavan_fifo fifo;
	struct network_client client_data;

	ret = ftp_client_create_pasv_link(client_proxy, &client_data);
	if (ret < 0) {
		pr_red_info("ftp_client_create_pasv_link");
		return ret;
	}

	ret = ftp_client_send_command2(client_proxy, NULL, 0, "LIST\r\n");
	if (ret != 150) {
		ret = -EFAULT;
		pr_red_info("ftp_client_send_command2 LIST");
		goto out_close_data_sockfd;
	}

	network_url_tostring(url, buff, sizeof(buff), NULL);

	fd = cavan_http_open_html_file(buff, NULL);
	if (fd < 0) {
		ret = fd;
		pr_red_info("web_proxy_open_html_file");
		goto out_close_data_sockfd;
	}

	ffile_puts(fd, "\t\t<h1>FTP Proxy Server (Fuang.Cao <a href=\"http://mail.foxmail.com\">cavan.cao@foxmail.com</a>)</h1>\r\n");
	ffile_printf(fd, "\t\t<h2>Directory: <a href=\"%s/\">%s</a></h2>\r\n", buff, buff);

	p = cavan_path_dirname_base(buff, url->pathname);
	if (p == buff || p[-1] != '/') {
		*p++ = '/';
		*p = 0;
	}

	ffile_printf(fd, "\t\t<h2><a href=\"%s\">Parent directory</a> (<a href=\"/\">Root directory</a>)</h2>\r\n", buff);
	ffile_puts(fd, "\t\t<table id=\"dirlisting\" summary=\"Directory Listing\">\r\n");
	ffile_puts(fd, "\t\t\t<tr><td><b>type</b></td><td><b>filename</b></td><td><b>size</b></td><td><b>date</b></td></tr>\r\n");

	ret = cavan_fifo_init(&fifo, sizeof(buff), &client_data);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		goto out_close_fd;
	}

	fifo.read = network_client_fifo_read;

	while (1) {
		int count;
		char *texts[16];

		if (cavan_fifo_read_line_strip(&fifo, buff, sizeof(buff)) == NULL) {
			break;
		}

		count = text_split_by_space(buff, texts, NELEM(texts));
		if (count < 9) {
			pr_red_info("invalid line %s", buff);
			continue;
		}

		ffile_puts(fd, "\t\t\t<tr class=\"entry\">");

		switch (texts[0][0]) {
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

		if (texts[0][0] == 'd') {
			ffile_printf(fd, "<a href=\"%s/\">%s</a>", texts[8], texts[8]);
		} else {
			ffile_printf(fd, "<a href=\"%s\">%s</a>", texts[8], texts[8]);
		}

		if (texts[0][0] == 'l' && count > 10) {
			ffile_printf(fd, " -> <a href=\"%s\">%s</a>", texts[10], texts[10]);
		}

		ffile_printf(fd, "</td><td class=\"size\">%s</td>", texts[4]);
		ffile_printf(fd, "<td class=\"date\">%s %s %s</td>", texts[5], texts[6], texts[7]);
		ffile_printf(fd, "</tr>\r\n");
	}

	ffile_puts(fd, "\t\t</table>\r\n");
	cavan_http_flush_html_file(fd);

	ret = ftp_client_read_response(client_proxy, NULL, 0);
	if (ret != 226) {
		ret = -EFAULT;
		pr_red_info("ftp_client_read_response");
		goto out_cavan_fifo_deinit;
	}

	ret = cavan_http_send_html(client, fd);
	if (ret < 0) {
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

	fd = cavan_http_open_html_file("connection failure", NULL);
	if (fd < 0) {
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

	ret = cavan_http_flush_html_file(fd);
	if (ret < 0) {
		pr_red_info("web_proxy_flush_html_file");
		goto out_close_fd;
	}

	ret = cavan_http_send_html(client, fd);

out_close_fd:
	close(fd);
	return ret;
}

static int web_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	return network_service_accept(&proxy->service, conn, 0);
}

static bool web_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
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

static void web_proxy_close_proxy_client(struct network_client *client, bool ftp_login)
{
	if (client->sockfd < 0) {
		return;
	}

	if (ftp_login) {
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
	struct network_client remote;
	struct network_client *client = conn;
	struct network_url urls[2], *url, *url_prev;
	char buff[2048], *buff_end, *req, *url_text;
	struct web_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	count = 0;
	url = NULL;
	ftp_login = false;
	remote.sockfd = -1;

	while (1) {
		ret = web_proxy_read_request(client, buff, sizeof(buff) - proxy->proxy_hostlen - 1);
		if (ret <= 0) {
			goto out_network_client_close_proxy;
		}

		buff_end = buff + ret;

#if WEB_PROXY_DEBUG
		*buff_end = 0;
		println("request is:\n%s", buff);
#endif

		for (url_text = buff; url_text < buff_end && *url_text != ' '; url_text++);

		cmdlen = url_text - buff;
		url_text++;

		type = cavan_http_get_request_type(buff, cmdlen);
		if (type < 0) {
			pr_red_info("invalid request[%" PRINT_FORMAT_SIZE "] `%s'", cmdlen, buff);

			if (remote.sockfd < 0) {
				return -EINVAL;
			}

			ret = network_client_send(&remote, buff, buff_end - buff);
			if (ret <= 0) {
				pr_red_info("network_client_send");
				goto out_network_client_close_proxy;
			}

			goto label_web_proxy_main_loop;
		}

		url_prev = url;
		url = (url == urls) ? urls + 1 : urls;

		if (url_text[0] == '/') {
			req = NULL;
			url = &proxy->url_proxy;
		} else {
			req = network_url_parse(url, url_text);
			if (req == NULL) {
				pr_red_info("web_proxy_parse_url:\n%s", url_text);
				ret = -EINVAL;
				goto out_network_client_close_proxy;
			}

			if (url->protocol[0] == 0) {
				if (url->port == NETWORK_PORT_INVALID) {
					pr_red_info("invalid url %s", network_url_tostring(url, NULL, 0, NULL));
					ret = -EINVAL;
					goto label_web_proxy_main_loop;
				}

				url->protocol = "http";
			}

			pr_info("%s[%d](%d)", cavan_http_request_type_tostring(type), type, count);
		}

		if (url_prev == NULL || network_url_equals(url_prev, url) == false) {
			web_proxy_close_proxy_client(&remote, ftp_login);

			ret = network_client_open(&remote, url, CAVAN_NET_FLAG_WAIT);
			if (ret < 0) {
				pr_red_info("network_client_open");
				web_proxy_send_connect_failed(client, url);
				return ret;
			}

			count = 0;
			ftp_login = false;
		} else {
			count++;
			pr_green_info("Don't need connect %d", count);
		}

		switch (remote.type) {
		case NETWORK_PROTOCOL_TCP:
		case NETWORK_PROTOCOL_HTTP:
		case NETWORK_PROTOCOL_HTTPS:
			switch (type) {
			case HTTP_REQ_CONNECT:
				ret = network_client_send_text(client, "HTTP/1.1 200 Connection established\r\n\r\n");
				if (ret < 0) {
					pr_red_info("network_client_send_text");
					goto out_network_client_close_proxy;
				}

#if CONFIG_CAVAN_SSL
				if (proxy->monitor) {
					ret = network_client_ssl_attach(client, true);
					if (ret < 0) {
						pr_red_info("network_client_ssl_attach");
						goto out_network_client_close_proxy;
					}

					ret = network_client_ssl_attach(&remote, false);
					if (ret < 0) {
						pr_red_info("network_client_ssl_attach");
						goto out_network_client_close_proxy;
					}

					web_proxy_monitor_loop(client, &remote);
				} else
#endif
				{
					tcp_proxy_main_loop(client, &remote);
				}

				goto out_network_client_close_proxy;

			default:
				if (req == NULL) {
					buff_end = web_proxy_set_prop(buff, buff_end, "Host", 4, proxy->url_proxy.hostname, proxy->proxy_hostlen);
#if WEB_PROXY_DEBUG
					*buff_end = 0;
#endif
					req = buff;
				} else {
					req -= cmdlen + 1;
					memcpy(req, buff, cmdlen);
					req[cmdlen] = ' ';
				}

#if WEB_PROXY_DEBUG
				println("New request is:\n%s", req);
#endif

				ret = network_client_send(&remote, req, buff_end - req);
				if (ret < 0) {
					pr_error_info("network_client_send");
					goto out_network_client_close_proxy;
				}

label_web_proxy_main_loop:
				if (web_proxy_main_loop(client, &remote, WEB_PROXY_OVERTIME) < 0) {
					goto out_network_client_close_proxy;
				}

				if (web_proxy_main_loop(&remote, client, WEB_PROXY_OVERTIME) < 0) {
					goto out_network_client_close_proxy;
				}
			}
			break;

		case NETWORK_PROTOCOL_FTP:
			if (ftp_login) {
				pr_green_info("Don't need login");
			} else {
				ret = ftp_client_login(&remote, NULL, NULL);
				if (ret < 0) {
					pr_red_info("ftp_client_login");
					goto out_network_client_close_proxy;
				}

				ftp_login = true;
			}

			ret = ftp_client_send_command2(&remote, NULL, 0, "TYPE I\r\n");
			if (ret != 200) {
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

			ret = ftp_client_send_command2(&remote, NULL, 0, "CWD %s\r\n", req[0] ? req : "/");
			if (ret != 250) {
				pr_red_info("ftp_client_send_command2 CWD");
				ret = -EFAULT;
				goto out_network_client_close_proxy;
			}

			switch (type) {
			case HTTP_REQ_GET:
				if (filename && filename[0]) {
					ret = web_proxy_ftp_read_file(client, &remote, filename);
					if (ret < 0) {
						pr_red_info("web_proxy_ftp_read_file `%s'", filename);

						if (errno == EISDIR) {
							filename[-1] = '/';
							filename = NULL;
							goto label_change_dir;
						}

						goto out_network_client_close_proxy;
					}
				} else {
					ret = web_proxy_ftp_list_directory(client, &remote, url);
					if (ret < 0) {
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
			pr_red_info("unsupport network protocol %s", network_protocol_tostring(remote.type));
			ret = -EINVAL;
			goto out_network_client_close_proxy;
		}
	}

out_network_client_close_proxy:
	web_proxy_close_proxy_client(&remote, ftp_login);
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
