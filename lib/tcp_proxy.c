/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_proxy.h>

static int tcp_proxy_service_handle(int index, cavan_shared_data_t data)
{
	int ret;
	socklen_t addrlen;
	struct sockaddr_in addr;
	struct tcp_proxy_service *proxy_service = data.type_void;
	int client_sockfd, proxy_sockfd;
	int server_sockfd = proxy_service->sockfd;
	ssize_t rwlen;
	char buff[1024];
	struct pollfd pfds[2];

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

	pfds[0].events = POLLIN;
	pfds[0].fd = client_sockfd;

	pfds[1].events = POLLIN;
	pfds[1].fd = proxy_sockfd;

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			goto out_close_proxy_sockfd;
		}

		if (pfds[0].revents)
		{
			rwlen = inet_recv(client_sockfd, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(proxy_sockfd, buff, rwlen) < rwlen)
			{
				break;
			}
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

	ret = 0;
out_close_proxy_sockfd:
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
