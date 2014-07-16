/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/ftp.h>
#include <cavan/tcp_proxy.h>

int tcp_proxy_main_loop(struct network_client *client_local, struct network_client *client_remote)
{
	int ret;
	ssize_t rwlen;
	char buff[2048];
	struct pollfd pfds[2] =
	{
		{
			.fd = client_local->sockfd,
			.events = POLLIN
		},
		{
			.fd = client_remote->sockfd,
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
			rwlen = client_local->recv(client_local, buff, sizeof(buff));
			if (rwlen <= 0 || client_remote->send(client_remote, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = client_remote->recv(client_remote, buff, sizeof(buff));
			if (rwlen <= 0 || client_local->send(client_local, buff, rwlen) < rwlen)
			{
				break;
			}
		}
	}

	return 0;
}

static int tcp_proxy_start_handler(struct cavan_dynamic_service *service)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	return network_service_open(&proxy->service, &proxy->url, 0);
}

static void tcp_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	network_service_close(&proxy->service);
}

static int tcp_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	return network_service_accept(&proxy->service, conn);
}

static void tcp_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int tcp_proxy_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	struct network_client client_proxy;
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	ret = network_client_open(&client_proxy, &proxy->url_proxy, 0);
	if (ret < 0)
	{
		pr_red_info("network_client_open");
		return ret;
	}

	ret = tcp_proxy_main_loop(conn, &client_proxy);

	network_client_close(&client_proxy);

	return ret;
}

int tcp_proxy_service_run(struct cavan_dynamic_service *service)
{
	struct tcp_proxy_service *proxy = cavan_dynamic_service_get_data(service);

	pr_bold_info("PROXY_URL = %s", network_url_tostring(&proxy->url_proxy, NULL, 0, NULL));

	service->name = "TCP_PROXY";
	service->conn_size = sizeof(struct network_client);
	service->start = tcp_proxy_start_handler;
	service->stop = tcp_proxy_stop_handler;
	service->run = tcp_proxy_service_run_handler;
	service->open_connect = tcp_proxy_open_connect;
	service->close_connect = tcp_proxy_close_connect;

	return cavan_dynamic_service_run(service);
}
