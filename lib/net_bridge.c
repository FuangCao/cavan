// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/net_bridge.h>

static int cavan_net_bridge_thread_handler(struct cavan_thread *thread, void *data)
{
	struct cavan_net_bridge_service *service = data;
	struct pollfd pfds[service->port_count + 1], *pfd;
	struct cavan_net_bridge_port *port;

	pfds[0] = thread->pfd;
	pfd = pfds + 1;

	double_link_foreach(&service->port_table, port)
	{
		pfd->fd = port->conn.sockfd;
		pfd->events = POLLIN;
		pfd->revents = 0;
		port->pfd = pfd;
		pfd++;
	}
	end_link_foreach(&service->port_table);

	while (1)
	{
		int ret;

		ret = poll(pfds, NELEM(pfds), -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		if (pfds[0].revents)
		{
			u32 event;
			cavan_thread_recv_event(thread, &event);
			break;
		}

		double_link_foreach(&service->port_table, port)
		{
			if (port->pfd->revents)
			{
				char buff[4096];
				ssize_t rdlen;
				struct mac_header *mac = (struct mac_header *)buff;

				rdlen = port->conn.recv(&port->conn, buff, sizeof(buff));
				if (rdlen < 0)
				{
					pr_red_info("port->conn.recv");
					return rdlen;
				}

				memcpy(port->mac_addr, mac->src_mac, sizeof(port->mac_addr));
			}
		}
		end_link_foreach(&service->port_table);
	}

	return 0;
}

int cavan_net_bridge_init(struct cavan_net_bridge_service *service)
{
	int ret;
	struct cavan_thread *thread;

	thread = &service->thread;
	thread->name = "NET_BRIDGE";
	thread->wake_handker = NULL;
	thread->handler = cavan_net_bridge_thread_handler;

	ret = cavan_thread_init(&service->thread, service);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		return ret;
	}

	ret = double_link_init(&service->port_table, MOFS(struct cavan_net_bridge_port, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_cavan_thread_deinit;
	}

	service->port_count = 0;

	return 0;

out_cavan_thread_deinit:
	cavan_thread_deinit(&service->thread);
	return ret;
}

void cavan_net_bridge_deinit(struct cavan_net_bridge_service *service)
{
	double_link_free_all(&service->port_table);
	double_link_deinit(&service->port_table);
}

int cavan_net_bridge_register_port(struct cavan_net_bridge_service *service, const char *url)
{
	int ret;
	struct cavan_net_bridge_port *port;

	port = malloc(sizeof(*port));
	if (port == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = network_connect_open(&port->conn, url);
	if (ret < 0)
	{
		pr_red_info("network_connect_open");
		goto out_free_port;
	}

	double_link_append(&service->port_table, &port->node);
	service->port_count++;

	return 0;

out_free_port:
	free(port);
	return ret;
}
