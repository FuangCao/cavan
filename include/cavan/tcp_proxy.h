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
	struct cavan_service_description service;
	int sockfd;
	u16 port;
	u16 proxy_port;
	char proxy_ip[32];

	int (*open_connect)(const char *ip, u16 port);
	void (*close_connect)(int sockfd);
};

int tcp_proxy_service_run(struct tcp_proxy_service *proxy_service);
