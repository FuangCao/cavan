// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/net_bridge.h>

static struct cavan_net_bridge_addr *cavan_net_bridge_port_addr_get(struct cavan_net_bridge_port *port, const u8 *addr, bool auto_remove)
{
	time_t second;
	struct cavan_net_bridge_addr *node;

	second = time(NULL);

	double_link_foreach(&port->addr_list, node)
	{
		if (auto_remove && second - node->time > NET_BRIDGE_ADDR_TIMEOUT)
		{
			pr_bold_info("[REMOVE] %s => %d", mac_address_tostring((char *)node->addr, MAC_ADDRESS_LEN), port->client->sockfd);
			double_link_remove_base(&node->node.node);
			cavan_data_pool_node_free(&port->addr_pool, node);
		}

		if (memcmp(node->addr, addr, MAC_ADDRESS_LEN) == 0)
		{
			link_foreach_return(&port->addr_list, node);
		}
	}
	double_link_foreach_end(&port->addr_list);

	return NULL;
}

static int cavan_net_bridge_port_addr_put(struct cavan_net_bridge_port *port, const u8 *addr)
{
	struct cavan_net_bridge_addr *node;

	node = cavan_net_bridge_port_addr_get(port, addr, false);
	if (node == NULL)
	{
		node = cavan_data_pool_alloc(&port->addr_pool);
		if (node == NULL)
		{
			pr_red_info("cavan_data_pool_alloc");
			return -ENOMEM;
		}

		memcpy(node->addr, addr, MAC_ADDRESS_LEN);
		double_link_append(&port->addr_list, &node->node.node);

		pr_bold_info("[ADD] %s => %d", mac_address_tostring((char *)addr, MAC_ADDRESS_LEN), port->client->sockfd);
	}

	node->time = time(NULL);

	return 0;
}

static void cavan_net_bridge_port_addr_free_all(struct cavan_net_bridge_port *port)
{
	struct cavan_net_bridge_addr *node;

	double_link_foreach(&port->addr_list, node)
	{
		cavan_data_pool_node_free(&port->addr_pool, node);
	}
	double_link_foreach_end(&port->addr_list);
}

int cavan_net_bridge_port_init(struct cavan_net_bridge_port *port, const char *url)
{
	int ret;

	port->client = network_client_open3(url);
	if (port->client == NULL)
	{
		pr_red_info("network_client_open3");
		return -EFAULT;
	}

	ret = CAVAN_DATA_POOL_INIT(&port->addr_pool, struct cavan_net_bridge_addr, node, 50);
	if (ret < 0)
	{
		pr_red_info("CAVAN_DATA_POOL_INIT");
		goto out_network_client_close;
	}

	ret = DOUBLE_LINK_INIT(&port->addr_list, struct cavan_net_bridge_addr, node.node);
	if (ret < 0)
	{
		pr_red_info("DOUBLE_LINK_INIT");
		goto out_cavan_data_pool_deinit;
	}

	port->next = port->prev = port;

	return 0;

out_cavan_data_pool_deinit:
	cavan_data_pool_deinit(&port->addr_pool);
out_network_client_close:
	network_client_close(port->client);
	return ret;
}

void cavan_net_bridge_port_deinit(struct cavan_net_bridge_port *port)
{
	cavan_net_bridge_port_addr_free_all(port);
	double_link_deinit(&port->addr_list);
	cavan_data_pool_deinit(&port->addr_pool);
	network_client_close(port->client);
}

static int cavan_net_bridge_transfer(struct cavan_net_bridge_port *port)
{
	ssize_t rdlen;
	char buff[4096];
	struct mac_header *mac;
	struct cavan_net_bridge_port *head;

	rdlen = port->client->recv(port->client, buff, sizeof(buff));
	if (rdlen < 0)
	{
		pr_red_info("port->client.recv");
		return rdlen;
	}

	mac = (struct mac_header *)buff;
	cavan_net_bridge_port_addr_put(port, mac->src_mac);

	if (cavan_net_bridge_port_addr_get(port, mac->dest_mac, true))
	{
		return 0;
	}

	head = port;

	for (port = head->next; port != head; port = port->next)
	{
		if (cavan_net_bridge_port_addr_get(port, mac->dest_mac, true))
		{
			port->client->send(port->client, buff, rdlen);
			return 0;
		}
	}

	for (port = head->next; port != head; port = port->next)
	{
		port->client->send(port->client, buff, rdlen);
	}

	return 0;
}

static int cavan_net_bridge_thread_handler(struct cavan_thread *thread, void *data)
{
	int count;
	struct cavan_net_bridge *bridge = data;
	struct pollfd *pfds, *pfd;
	struct cavan_net_bridge_port *port;

	cavan_net_bridge_lock(bridge);

	if (bridge->head == NULL)
	{
		pr_red_info("bridge->head == NULL");
		cavan_thread_suspend(thread);
		goto out_cavan_net_bridge_unlock;
	}

	for (count = 2, port = bridge->head; port->next != bridge->head; port = port->next, count++);

	println("count = %d", count);

	pfds = alloca(sizeof(*pfds) * count);
	if (pfds == NULL)
	{
		pr_error_info("alloca");
		goto out_cavan_net_bridge_unlock;
	}

	pfd = pfds;
	*pfd++ = thread->pfd;
	port = bridge->head;

	while (1)
	{
		port->pfd = pfd;
		pfd->fd = port->client->sockfd;
		pfd->events = POLLIN;
		pfd->revents = 0;

		port = port->next;
		if (port == bridge->head)
		{
			break;
		}

		pfd++;
	}

	while (1)
	{
		int ret;

		cavan_net_bridge_unlock(bridge);
		ret = poll(pfds, count, -1);
		cavan_net_bridge_lock(bridge);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		if (pfds[0].revents)
		{
			u32 event;
			cavan_thread_recv_event(thread, &event);
			bridge->port_changed = true;
		}

		if (bridge->port_changed)
		{
			bridge->port_changed = false;
			break;
		}

		port = bridge->head;

		while (1)
		{
			if (port->pfd->revents)
			{
				cavan_net_bridge_transfer(port);
			}

			if (port->next == bridge->head)
			{
				break;
			}

			port = port->next;
		}
	}

out_cavan_net_bridge_unlock:
	cavan_net_bridge_unlock(bridge);
	return 0;
}

int cavan_net_bridge_init(struct cavan_net_bridge *bridge)
{
	int ret;
	struct cavan_thread *thread;

	ret = pthread_mutex_init(&bridge->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthead_mutex_init");
		return ret;
	}

	thread = &bridge->thread;
	thread->name = "NET_BRIDGE";
	thread->wake_handker = NULL;
	thread->handler = cavan_net_bridge_thread_handler;

	ret = cavan_thread_init(&bridge->thread, bridge);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_pthread_mutex_destroy;
	}

	bridge->head = NULL;
	bridge->port_changed = false;

	ret = cavan_thread_start(thread);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_start");
		goto out_cavan_thread_deinit;
	}

	return 0;

out_cavan_thread_deinit:
	cavan_thread_deinit(thread);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&bridge->lock);
	return ret;
}

void cavan_net_bridge_deinit(struct cavan_net_bridge *bridge)
{
	struct cavan_net_bridge_port *port, *head;

	head = bridge->head;
	if (head)
	{
		port = head;

		while (1)
		{
			struct cavan_net_bridge_port *next = port->next;

			free(port);

			if (next == head)
			{
				break;
			}

			port = next;
		}
	}

	cavan_thread_deinit(&bridge->thread);
	pthread_mutex_destroy(&bridge->lock);
}

int cavan_net_bridge_register_port(struct cavan_net_bridge *bridge, const char *url)
{
	int ret;
	struct cavan_net_bridge_port *port;

	port = malloc(sizeof(*port));
	if (port == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = cavan_net_bridge_port_init(port, url);
	if (ret < 0)
	{
		pr_red_info("cavan_net_bridge_port_init");
		goto out_free_port;
	}

	cavan_net_bridge_lock(bridge);

	if (bridge->head)
	{
		struct cavan_net_bridge_port *head = bridge->head;

		port->next = head;
		port->prev = head->prev;
		head->prev->next = port;
		head->prev = port;
	}
	else
	{
		bridge->head = port;
	}

	bridge->port_changed = true;
	cavan_net_bridge_unlock(bridge);

	cavan_thread_resume(&bridge->thread);

	return 0;

out_free_port:
	free(port);
	return ret;
}
