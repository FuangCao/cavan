#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/pool.h>
#include <cavan/thread.h>
#include <cavan/network.h>

#define NET_BRIDGE_ADDR_TIMEOUT		(5 * 60)

struct cavan_net_bridge_addr
{
	time_t time;
	u8 addr[MAC_ADDRESS_LEN];
	struct cavan_data_pool_node node;
};

struct cavan_net_bridge_port
{
	struct pollfd *pfd;
	struct network_client client;
	struct double_link addr_list;
	struct cavan_data_pool addr_pool;

	struct cavan_net_bridge_port *next;
	struct cavan_net_bridge_port *prev;
};

struct cavan_net_bridge
{
	struct cavan_thread thread;
	pthread_mutex_t lock;
	bool port_changed;
	struct cavan_net_bridge_port *head;
};

int cavan_net_bridge_port_init(struct cavan_net_bridge_port *port, const char *url);
void cavan_net_bridge_port_deinit(struct cavan_net_bridge_port *port);

int cavan_net_bridge_init(struct cavan_net_bridge *bridge);
void cavan_net_bridge_deinit(struct cavan_net_bridge *bridge);
int cavan_net_bridge_register_port(struct cavan_net_bridge *bridge, const char *url);

static inline void cavan_net_bridge_lock(struct cavan_net_bridge *bridge)
{
	pthread_mutex_lock(&bridge->lock);
}

static inline void cavan_net_bridge_unlock(struct cavan_net_bridge *bridge)
{
	pthread_mutex_unlock(&bridge->lock);
}
