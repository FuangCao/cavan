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

static int web_proxy_service_handle(struct cavan_service_description *service, int index, cavan_shared_data_t data)
{
	int ret = 0;
	ssize_t rwlen;
	size_t cmdlen;
	socklen_t addrlen;
	struct sockaddr_in addr, proxy_addr;
	int client_sockfd, proxy_sockfd;
	int server_sockfd = service->data.type_int;
	char buff[2048], *buff_end, *req, *url;
	char protocol[512];
	char hostname[512];
	struct pollfd pfds[2];

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	inet_show_sockaddr(&addr);

	pfds[0].fd = client_sockfd;
	pfds[0].events = pfds[1].events = POLLIN;

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
		url++;

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

		if (text_lhcmp("CONNECT", buff) == 0)
		{
			tcp_proxy_main_loop(client_sockfd, proxy_sockfd);
			goto label_close_proxy_sockfd;
		}

		pfds[1].fd = proxy_sockfd;
		req -= cmdlen + 1;
		memcpy(req, buff, cmdlen);
		req[cmdlen] = ' ';

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

		while (1)
		{
			ret = poll(pfds, NELEM(pfds), -1);
			if (ret <= 0)
			{
				break;
			}

			if (pfds[0].revents)
			{
				break;
			}

			if (pfds[1].revents)
			{
				rwlen = inet_recv(proxy_sockfd, buff, sizeof(buff));
				if (rwlen <= 0 || inet_send(client_sockfd, buff, rwlen) < rwlen)
				{
					break;
				}
			}
		}

label_close_proxy_sockfd:
		close(proxy_sockfd);
	}

	close(client_sockfd);
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
