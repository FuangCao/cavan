/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/ftp.h>
#include <cavan/tcp_proxy.h>

int tcp_proxy_main_loop(int localfd, int remotefd)
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
			if (ret < 0)
			{
				return ret;
			}

			return -ETIMEDOUT;
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

static int tcp_proxy_start_handler(struct cavan_dynamic_service *service)
{
	int sockfd;
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	sockfd = inet_create_tcp_service(proxy->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	proxy->sockfd = sockfd;

	return 0;
}

static void tcp_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	inet_close_tcp_socket(proxy->sockfd);
}

static int tcp_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	socklen_t addrlen;
	struct tcp_proxy_service *proxy;
	struct inet_connect *client = conn;

	proxy = cavan_dynamic_service_get_data(service);
	client->sockfd = inet_accept(proxy->sockfd, &client->addr, &addrlen);
	if (client->sockfd < 0)
	{
		pr_error_info("inet_accept");
		return client->sockfd;
	}

	inet_show_sockaddr(&client->addr);

	return 0;
}

static void tcp_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct inet_connect *client = conn;

	inet_close_tcp_socket(client->sockfd);
}

static int tcp_proxy_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	int proxy_sockfd;
	struct inet_connect *client = conn;
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	proxy_sockfd = proxy->open_connect(proxy->proxy_host, proxy->proxy_port);
	if (proxy_sockfd < 0)
	{
		return proxy_sockfd;
	}

	ret = tcp_proxy_main_loop(client->sockfd, proxy_sockfd);

	inet_close_tcp_socket(proxy_sockfd);

	return ret;
}

int tcp_proxy_service_run(struct cavan_dynamic_service *service)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	println("PROXY_HOST = %s, PROXY_PORT = %d", proxy->proxy_host, proxy->proxy_port);

	if (proxy->open_connect == NULL)
	{
		proxy->open_connect = inet_create_tcp_link2;
	}

	if (proxy->close_connect == NULL)
	{
		proxy->close_connect = inet_close_tcp_socket;
	}

	service->name = "TCP_PROXY";
	service->conn_size = sizeof(struct inet_connect);
	service->start = tcp_proxy_start_handler;
	service->stop = tcp_proxy_stop_handler;
	service->run = tcp_proxy_service_run_handler;
	service->open_connect = tcp_proxy_open_connect;
	service->close_connect = tcp_proxy_close_connect;

	return cavan_dynamic_service_run(service);
}
