#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/thread.h>
#include <cavan/network.h>

#define ROUTE_TABLE_SIZE	16

struct cavan_net_bridge_port
{
	u8 mac_addr[MAC_ADDRESS_LEN];
	struct pollfd *pfd;
	struct network_connect conn;
	struct double_link_node node;
};

struct cavan_net_bridge_service
{
	struct cavan_thread thread;
	int port_count;
	struct double_link port_table;
};

int cavan_net_bridge_init(struct cavan_net_bridge_service *service);
void cavan_net_bridge_deinit(struct cavan_net_bridge_service *service);
int cavan_net_bridge_register_port(struct cavan_net_bridge_service *service, const char *url);
