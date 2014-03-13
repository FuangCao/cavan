#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/network.h>

#define ROUTE_TABLE_SIZE	16

struct cavan_net_bridge_descriptor
{
	u8 host_hwaddr[MAC_ADDRESS_LEN];
	struct sockaddr_in host_ifaddr;
	struct cavan_route_table route_table;
	int sockfd;
};

int cavan_create_socket_raw(const char *if_name);
int cavan_net_bridge_init(struct cavan_net_bridge_descriptor *desc, const char *if_name);
void cavan_net_bridge_deinit(struct cavan_net_bridge_descriptor *desc);
int cavan_net_bridge_recv_daemon(struct cavan_net_bridge_descriptor *desc, ssize_t (*write_handle)(void *data, const char *buff, size_t size));
int cavan_net_bridge_send_daemon(struct cavan_net_bridge_descriptor *desc, ssize_t (*read_handle)(void *data, const char *buff, size_t size));

