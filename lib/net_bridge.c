// Fuang.Cao <cavan.cfa@gmail.com> Mon Sep 12 19:04:07 CST 2011

#include <cavan.h>
#include <cavan/net_bridge.h>
#include <cavan/network.h>

int cavan_create_socket_raw(const char *if_name)
{
	int ret;
	int sockfd;
	struct ifreq req;
	struct sockaddr_ll bind_addr;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0)
	{
		pr_error_info("socket PF_PACKET SOCK_RAW");
		return sockfd;
	}

	if (if_name == NULL)
	{
		return sockfd;
	}

	strcpy(req.ifr_name, if_name);

	ret = ioctl(sockfd, SIOCGIFINDEX, &req);
	if (ret < 0)
	{
		pr_error_info("ioctl SIOCGIFINDEX");
		goto out_close_socket;
	}

	bind_addr.sll_family = PF_PACKET;
	bind_addr.sll_ifindex = req.ifr_ifindex;
	bind_addr.sll_protocol = htons(ETH_P_ALL);

	ret = bind(sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));
	if (ret < 0)
	{
		pr_error_info("bind");
		goto out_close_socket;
	}

	return sockfd;

out_close_socket:
	close(sockfd);
	return ret;
}

int cavan_net_bridge_init(struct cavan_net_bridge_descriptor *desc, const char *if_name)
{
	int ret;
	int sockfd;
	struct ifreq req;
	struct sockaddr_ll bind_addr;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0)
	{
		print_error("socket");
		return sockfd;
	}

	strcpy(req.ifr_name, if_name);

	ret = ioctl(sockfd, SIOCGIFHWADDR, &req);
	if (ret < 0)
	{
		print_error("ioctl SIOCGIFHWADDR");
		goto out_close_socket;
	}

	memcpy(desc->host_hwaddr, req.ifr_ifru.ifru_hwaddr.sa_data, MAC_ADDRESS_LEN);

	ret = ioctl(sockfd, SIOCGIFINDEX, &req);
	if (ret < 0)
	{
		print_error("ioctl SIOCGIFINDEX");
		goto out_close_socket;
	}

	bind_addr.sll_family = PF_PACKET;
	bind_addr.sll_ifindex = req.ifr_ifindex;
	bind_addr.sll_protocol = htons(ETH_P_ALL);

	ret = bind(sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));
	if (ret < 0)
	{
		print_error("bind");
		goto out_close_socket;
	}

	ret = ioctl(sockfd, SIOCGIFADDR, &req);
	if (ret < 0)
	{
		print_error("ioctl SIOCGIFADDR");
		goto out_close_socket;
	}

	memcpy(&desc->host_ifaddr, &req.ifr_ifru.ifru_addr, sizeof(desc->host_ifaddr));

	ret = cavan_route_table_init(&desc->route_table, ROUTE_TABLE_SIZE);
	if (ret < 0)
	{
		error_msg("cavan_route_table_init");
		goto out_close_socket;
	}

	desc->sockfd = sockfd;

	return 0;

out_close_socket:
	close(sockfd);

	return ret;
}

void cavan_net_bridge_deinit(struct cavan_net_bridge_descriptor *desc)
{
	if (desc == NULL)
	{
		return;
	}

	cavan_route_table_deinit(&desc->route_table);
	close(desc->sockfd);
}

int cavan_net_bridge_recv_daemon(struct cavan_net_bridge_descriptor *desc, ssize_t (*write_handle)(void *data, const char *buff, size_t size))
{
	return 0;
}

int cavan_net_bridge_send_daemon(struct cavan_net_bridge_descriptor *desc, ssize_t (*read_handle)(void *data, const char *buff, size_t size))
{
	return 0;
}

