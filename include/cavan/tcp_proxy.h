#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>

struct tcp_proxy_service
{
	int sockfd;
	u16 port;
	u16 proxy_port;
	const char *proxy_host;

	int (*open_connect)(const char *hostname, u16 port);
	void (*close_connect)(int sockfd);
};

int tcp_proxy_main_loop(int localfd, int remotefd);
int tcp_proxy_service_run(struct cavan_dynamic_service *service);
