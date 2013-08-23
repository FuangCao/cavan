/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_proxy.h>

#define WEB_PROXY_DEBUG		1

static struct network_protocol protocol_map[] =
{
	{"http", 80},
	{"ftp", 21}
};

static int tcp_proxy_main_loop(int localfd, int remotefd)
{
	int ret;
	ssize_t rwlen;
	char buff[2048];
	struct pollfd pfds[2] =
	{
		{
			.fd = localfd,
			.events = POLLIN
		},
		{
			.fd = remotefd,
			.events = POLLIN
		}
	};

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			return ret;
		}

		if (pfds[0].revents)
		{
			rwlen = inet_recv(localfd, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(remotefd, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = inet_recv(remotefd, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(localfd, buff, rwlen) < rwlen)
			{
				break;
			}
		}
	}

	return 0;
}

static int tcp_proxy_service_handle(struct cavan_service_description *service, int index, cavan_shared_data_t data)
{
	int ret;
	socklen_t addrlen;
	struct sockaddr_in addr;
	struct tcp_proxy_service *proxy_service = data.type_void;
	int client_sockfd, proxy_sockfd;
	int server_sockfd = proxy_service->sockfd;

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	inet_show_sockaddr(&addr);

	proxy_sockfd = proxy_service->open_connect(proxy_service->proxy_ip, proxy_service->proxy_port);
	if (proxy_sockfd < 0)
	{
		ret = proxy_sockfd;
		pr_red_info("inet_connect");
		goto out_close_client_sockfd;
	}

	ret = tcp_proxy_main_loop(client_sockfd, proxy_sockfd);

	close(proxy_sockfd);
out_close_client_sockfd:
	close(client_sockfd);
	return ret;
}

int tcp_proxy_service_run(struct tcp_proxy_service *proxy_service)
{
	int ret;
	int sockfd;
	struct cavan_service_description *service;

	println("PROXY_IP = %s, PROXY_PORT = %d", proxy_service->proxy_ip, proxy_service->proxy_port);

	sockfd = inet_create_tcp_service(proxy_service->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	proxy_service->sockfd = sockfd;

	if (proxy_service->open_connect == NULL)
	{
		proxy_service->open_connect = inet_create_tcp_link2;
	}

	if (proxy_service->close_connect == NULL)
	{
		proxy_service->close_connect = inet_close_tcp_socket;
	}

	service = &proxy_service->service;
	service->data.type_void = proxy_service;
	service->handler = tcp_proxy_service_handle;
	service->threads = NULL;
	ret = cavan_service_run(service);
	cavan_service_stop(service);
	inet_close_tcp_socket(sockfd);

	return ret;
}

// ================================================================================

char *web_proxy_parse_url(const char *url, char *protocol, size_t protlen, char *hostname, size_t namelen)
{
	int step = 0;
	char *p = protocol;
	char *p_end = p + protlen;

	while (p < p_end)
	{
		switch (*url)
		{
		case 0 ... 31:
		case ' ':
		case '/':
			if (step < 1)
			{
				return NULL;
			}

			*p = 0;
			return (char *)url;

		case ':':
			*p = 0;

			if (step == 0 && text_lhcmp("//", url + 1) == 0)
			{
				url += 3;
				p = hostname;
				p_end = p + namelen;

			}
			else if (IS_NUMBER(url[1]))
			{
				url++;
				p = protocol;
				p_end = p + protlen;

				if (step == 0)
				{
					text_copy(hostname, protocol);
				}
			}
			else
			{
				return NULL;
			}

			step++;
			break;

		default:
			*p++ = *url++;
		}
	}

	return NULL;
}

int web_proxy_protocol2port(const char *protocol)
{
	struct network_protocol *p, *p_end;

	if (text_is_number(protocol))
	{
		return text2value_unsigned(protocol, NULL, 10);
	}

	for (p = protocol_map, p_end = p + NELEM(protocol_map); p < p_end; p++)
	{
		if (text_cmp(protocol, p->name) == 0)
		{
			return p->port;
		}
	}

	return -EINVAL;
}

http_request_type_t web_proxy_get_request_type(const char *req)
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

static int web_proxy_send_data(int srcfd, int destfd, int timeout)
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
			break;
		}

		if (pfds[0].revents)
		{
			rwlen = inet_recv(srcfd, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(destfd, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			break;
		}
	}

	return ret;
}

static int web_proxy_service_handle(struct cavan_service_description *service, int index, cavan_shared_data_t data)
{
	int ret = 0;
	ssize_t rwlen;
	size_t cmdlen;
	socklen_t addrlen;
	http_request_type_t type;
	struct sockaddr_in addr, proxy_addr;
	int client_sockfd, proxy_sockfd;
	int server_sockfd = service->data.type_int;
	char buff[2048], *buff_end, *req, *url;
	char protocol[512];
	char hostname[512];

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	cavan_service_set_busy(service, index, true);
	inet_show_sockaddr(&addr);

	while (1)
	{
		rwlen = inet_recv(client_sockfd, buff, sizeof(buff) - 1);
		if (rwlen <= 0)
		{
			break;
		}

		buff[rwlen] = 0;
		buff_end = buff + rwlen;

#if WEB_PROXY_DEBUG
		println("request is:\n%s", buff);
#endif

		for (url = buff; url < buff_end && *url != ' '; url++);

		cmdlen = url - buff;
		*url++ = 0;

		type = web_proxy_get_request_type(buff);
		if (type < 0)
		{
			ret = type;
			pr_red_info("invalid request %s", buff);
			break;
		}

#if WEB_PROXY_DEBUG
		pr_green_info("%s = %d, service = %d", buff, type, index);
#endif

		req = web_proxy_parse_url(url, protocol, sizeof(protocol), hostname, sizeof(hostname));
		if (req == NULL)
		{
			ret = -EINVAL;
			pr_red_info("web_proxy_parse_url:\n%s", url);
			break;
		}

#if WEB_PROXY_DEBUG
		println("protocol = %s, hostname = %s", protocol, hostname);
#endif

		if (inet_hostname2sockaddr(hostname, &proxy_addr) < 0)
		{
			ret = -EINVAL;
			pr_red_info("inet_hostname2sockaddr of %s failed", hostname);
			break;
		}

		ret = web_proxy_protocol2port(protocol);
		if (ret < 0)
		{
			pr_red_info("invalid protocol %s", protocol);
			break;
		}

		proxy_addr.sin_port = htons((u16)ret);
		inet_show_sockaddr(&proxy_addr);

		proxy_sockfd = inet_create_tcp_link1(&proxy_addr);
		if (proxy_sockfd < 0)
		{
			ret = proxy_sockfd;
			pr_red_info("inet_create_tcp_link1");
			break;
		}

		if (type == HTTP_REQ_CONNECT)
		{
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

		rwlen = inet_send(proxy_sockfd, req, buff_end - req);
		if (rwlen < 0)
		{
			ret = rwlen;
			pr_error_info("inet_send");
			goto label_close_proxy_sockfd;
		}

		switch (type)
		{
		case HTTP_REQ_GET:
		case HTTP_REQ_HEAD:
			web_proxy_send_data(proxy_sockfd, client_sockfd, -1);
			break;

		case HTTP_REQ_PUT:
		case HTTP_REQ_POST:
		case HTTP_REQ_CONNECT:
			tcp_proxy_main_loop(client_sockfd, proxy_sockfd);
			break;

		default:
			pr_red_info("unsupport http request %d", type);
		}

label_close_proxy_sockfd:
		close(proxy_sockfd);
	}

	close(client_sockfd);
	cavan_service_set_busy(service, index, false);

	return ret;
}

int web_proxy_service_run(struct cavan_service_description *proxy_service, u16 port)
{
	int ret;
	int sockfd;

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	proxy_service->data.type_int = sockfd;
	proxy_service->handler = web_proxy_service_handle;
	proxy_service->threads = NULL;
	ret = cavan_service_run(proxy_service);
	cavan_service_stop(proxy_service);
	inet_close_tcp_socket(sockfd);

	return ret;
}
