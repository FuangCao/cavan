#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>

#define CAVAN_TCP_PROXY_PORT		9099

struct tcp_proxy_service
{
	struct network_service service;
	struct network_url url;
	struct network_url url_proxy;
};

int tcp_proxy_main_loop(struct network_client *client_local, struct network_client *client_remote);
int tcp_proxy_service_run(struct cavan_dynamic_service *service);
