// Fuang.Cao <cavan.cfa@gmail.com> Fri Sep 16 15:25:24 CST 2011

#include <cavan.h>
#include <cavan/net_bridge.h>

#define FILE_CREATE_DATE	"Fri Sep 16 15:25:24 CST 2011"
#define DEFAULT_NET_DEVICE	"eth0"

static void show_usage(void)
{
	println("Usage:");
}

static int net_monitor_run(const char *net_dev, const char *text_src_ip, const char *text_dest_ip)
{
	int ret;
	struct cavan_net_bridge_descriptor desc;
	char buff[4096];
	struct mac_header *mac_hdr = (void *)buff;
	struct ip_header *ip_hdr = (void *)(mac_hdr + 1);
	struct arp_header *arp_hdr = (void *)(mac_hdr + 1);
	struct udp_header *udp_hdr = (void *)(ip_hdr + 1);
	struct tcp_header *tcp_hdr = (void *)(ip_hdr + 1);
	struct dhcp_header *dhcp_hdr = (void *)(udp_hdr + 1);
	struct icmp_header *icmp_hdr = (void *)(ip_hdr + 1);
	struct sockaddr_ll remote_addr;
	socklen_t addr_len;
	int sockfd;
	u32 src_ip, dest_ip;

	ret = cavan_net_bridge_init(&desc, net_dev);
	if (ret < 0)
	{
		pr_red_info("cavan_net_bridge_init");
		return ret;
	}

	sockfd = desc.sockfd;
	src_ip = inet_addr(text_src_ip);
	dest_ip = inet_addr(text_dest_ip);

	println("net device = %s, host mac = %s", net_dev, mac_address_tostring((char *)desc.host_hwaddr, MAC_ADDRESS_LEN));
	println("src_ip = %s", inet_ntoa(*(struct in_addr *)&src_ip));
	println("dest_ip = %s", inet_ntoa(*(struct in_addr *)&dest_ip));

	while (1)
	{
		addr_len = sizeof(remote_addr);
		ret = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&remote_addr, &addr_len);
		if (ret < 0)
		{
			print_error("recvfrom");
			break;
		}

		switch (ntohs(mac_hdr->protocol_type))
		{
		case ETH_P_IP:
			if (ip_hdr->src_ip != src_ip && ip_hdr->dest_ip != dest_ip)
			{
				continue;
			}

			show_ip_header(ip_hdr, 0);

			switch (ip_hdr->protocol_type)
			{
			case IPPROTO_UDP:
				show_udp_header(udp_hdr);
				if (udp_hdr->src_port == htons(67) || udp_hdr->src_port == htons(68))
				{
					show_dhcp_header(dhcp_hdr);
				}
				break;

			case IPPROTO_TCP:
				show_tcp_header(tcp_hdr);
				break;

			case IPPROTO_ICMP:
				show_icmp_header(icmp_hdr);
				break;
			}
			break;

		case ETH_P_ARP:
		case ETH_P_RARP:
			if (arp_hdr->src_ip != src_ip && arp_hdr->dest_ip != dest_ip)
			{
				continue;
			}
			show_arp_header(arp_hdr, 0);
			break;

		default:
			continue;
		}

		show_mac_header(mac_hdr);
		print_sep(60);
	}

	cavan_net_bridge_uninit(&desc);

	return -1;
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'h',
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
		},
	};

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	switch (argc - optind)
	{
	case 1:
		return net_monitor_run(DEFAULT_NET_DEVICE, argv[optind], argv[optind]);

	case 2:
		return net_monitor_run(argv[optind], argv[optind + 1], argv[optind + 1]);

	case 3:
		return net_monitor_run(argv[optind], argv[optind + 1], argv[optind + 2]);

	default:
		pr_red_info("argument falut");
	}

	return 0;
}
