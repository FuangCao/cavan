// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 21 10:08:25 CST 2011

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/file.h>

ssize_t sendto_select(int sockfd, int retry, const void *buff, size_t len, const struct sockaddr_in *remote_addr)
{
	while (retry--)
	{
		ssize_t sendlen;

		sendlen = inet_sendto(sockfd, buff, len, remote_addr);
		if (sendlen < 0)
		{
			print_error("send data failed");
			return sendlen;
		}

		if (file_poll_input(sockfd, NETWORK_TIMEOUT_VALUE))
		{
			return sendlen;
		}

		println("timeout retry = %d", retry);
	}

	return -ETIMEDOUT;
}

ssize_t sendto_receive(int sockfd, long timeout, int retry, const void *send_buff, ssize_t sendlen, void *recv_buff, ssize_t recvlen, struct sockaddr_in *remote_addr, socklen_t *addr_len)
{
	sendlen = sendto_select(sockfd, retry, send_buff, sendlen, remote_addr);
	if (sendlen < 0)
	{
		error_msg("send data timeout");
		return sendlen;
	}

	return inet_recvfrom(sockfd, recv_buff, recvlen, remote_addr, addr_len);
}

const char *mac_protocol_type_tostring(int type)
{
	switch (type)
	{
	case ETH_P_LOOP:
		return "ETH_P_LOOP";
	case ETH_P_PUP:
		return "ETH_P_PUP";
	case ETH_P_PUPAT:
		return "ETH_P_PUPAT";
	case ETH_P_IP:
		return "ETH_P_IP";
	case ETH_P_X25:
		return "ETH_P_X25";
	case ETH_P_ARP:
		return "ETH_P_ARP";
	case ETH_P_BPQ:
		return "ETH_P_BPQ";
	case ETH_P_IEEEPUP:
		return "ETH_P_IEEEPUP";
	case ETH_P_IEEEPUPAT:
		return "ETH_P_IEEEPUPAT";
	case ETH_P_DEC:
		return "ETH_P_DEC";
	case ETH_P_DNA_DL:
		return "ETH_P_DNA_DL";
	case ETH_P_DNA_RC:
		return "ETH_P_DNA_RC";
	case ETH_P_DNA_RT:
		return "ETH_P_DNA_RT";
	case ETH_P_LAT:
		return "ETH_P_LAT";
	case ETH_P_DIAG:
		return "ETH_P_DIAG";
	case ETH_P_CUST:
		return "ETH_P_CUST";
	case ETH_P_SCA:
		return "ETH_P_SCA";
	case ETH_P_RARP:
		return "ETH_P_RARP";
	case ETH_P_ATALK:
		return "ETH_P_ATALK";
	case ETH_P_AARP:
		return "ETH_P_AARP";
	case ETH_P_8021Q:
		return "ETH_P_8021Q";
	case ETH_P_IPX:
		return "ETH_P_IPX";
	case ETH_P_IPV6:
		return "ETH_P_IPV6";
	case ETH_P_SLOW:
		return "ETH_P_SLOW";
	case ETH_P_WCCP:
		return "ETH_P_WCCP";
	case ETH_P_PPP_DISC:
		return "ETH_P_PPP_DISC";
	case ETH_P_PPP_SES:
		return "ETH_P_PPP_SES";
	case ETH_P_MPLS_UC:
		return "ETH_P_MPLS_UC";
	case ETH_P_MPLS_MC:
		return "ETH_P_MPLS_MC";
	case ETH_P_ATMMPOA:
		return "ETH_P_ATMMPOA";
	case ETH_P_ATMFATE:
		return "ETH_P_ATMFATE";
	case ETH_P_AOE:
		return "ETH_P_AOE";
	case ETH_P_TIPC:
		return "ETH_P_TIPC";
#if 0
	case ETH_P_TEB:
		return "ETH_P_TEB";
	case ETH_P_PAUSE:
		return "ETH_P_PAUSE";
	case ETH_P_LINK_CTL:
		return "ETH_P_LINK_CTL";
	case ETH_P_EDSA:
		return "ETH_P_EDSA";
	case ETH_P_FIP:
		return "ETH_P_FIP";
	case ETH_P_1588:
		return "ETH_P_1588";
	case ETH_P_FCOE:
		return "ETH_P_FCOE";
	case ETH_P_PAE:
		return "ETH_P_PAE";
#endif
	default:
		return "unknown";
	}
}

const char *ip_protocol_type_tostring(int type)
{
	switch (type)
	{
	case IPPROTO_IP:
		return "IPPROTO_IP";
	case IPPROTO_ICMP:
		return "IPPROTO_ICMP";
	case IPPROTO_IGMP:
		return "IPPROTO_IGMP";
	case IPPROTO_IPIP:
		return "IPPROTO_IPIP";
	case IPPROTO_TCP:
		return "IPPROTO_TCP";
	case IPPROTO_EGP:
		return "IPPROTO_EGP";
	case IPPROTO_PUP:
		return "IPPROTO_PUP";
	case IPPROTO_UDP:
		return "IPPROTO_UDP";
	case IPPROTO_IDP:
		return "IPPROTO_IDP";
	case IPPROTO_RSVP:
		return "IPPROTO_RSVP";
	case IPPROTO_GRE:
		return "IPPROTO_GRE";
	case IPPROTO_IPV6:
		return "IPPROTO_IPV6";
	case IPPROTO_ESP:
		return "IPPROTO_ESP";
	case IPPROTO_AH:
		return "IPPROTO_AH";
	case IPPROTO_COMP:
		return "IPPROTO_COMP";
	case IPPROTO_SCTP:
		return "IPPROTO_SCTP";
	case IPPROTO_RAW:
		return "IPPROTO_RAW";
	case IPPROTO_PIM:
		return "IPPROTO_PIM";
#if 0
	case IPPROTO_DCCP:
		return "IPPROTO_DCCP";
	case IPPROTO_BEETPH:
		return "IPPROTO_BEETPH";
	case IPPROTO_UDPLITE:
		return "IPPROTO_UDPLITE";
#endif
	default:
		return "unknown";
	}
}

void show_mac_header(struct mac_header *hdr)
{
	println("dest_mac = %s", mac_address_tostring((char *)hdr->dest_mac, sizeof(hdr->dest_mac)));
	println("src_mac = %s", mac_address_tostring((char *)hdr->src_mac, sizeof(hdr->src_mac)));
	pr_bold_info("protocol_type = %s", mac_protocol_type_tostring(ntohs(hdr->protocol_type)));
}

void show_ip_header(struct ip_header *hdr, int simple)
{
	if (simple == 0)
	{
		println("version = %d", hdr->version);
		println("header_length = %d", hdr->header_length);
		println("service_type = %d", hdr->service_type);
		println("total_length = %d", ntohs(hdr->total_length));
		println("flags = %d", hdr->flags);
		println("piece_offset = %d", hdr->piece_offset);
		println("ttl = %d", hdr->ttl);
		println("header_checksum = 0x%04x", ntohs(hdr->header_checksum));
	}

	pr_bold_info("protocol_type = %s", ip_protocol_type_tostring(hdr->protocol_type));
	println("src_ip = %s", inet_ntoa(*(struct in_addr *)&hdr->src_ip));
	println("dest_ip = %s", inet_ntoa(*(struct in_addr *)&hdr->dest_ip));
}

void show_tcp_header(struct tcp_header *hdr)
{
	println("src_port = %d", ntohs(hdr->src_port));
	println("dest_port = %d", ntohs(hdr->dest_port));
	println("sequence = %d", ntohl(hdr->sequence));
	println("sck_sequence = %d", ntohl(hdr->sck_sequence));
	println("window_size = %d", ntohs(hdr->window_size));
	println("reserve = %d", hdr->reserve);
	println("TCP_URG = %d", hdr->TCP_URG);
	println("TCP_ACK = %d", hdr->TCP_ACK);
	println("TCP_PSH = %d", hdr->TCP_PSH);
	println("TCP_RST = %d", hdr->TCP_RST);
	println("TCP_SYN = %d", hdr->TCP_SYN);
	println("TCP_FIN = %d", hdr->TCP_FIN);
	println("checksum = 0x%04x", ntohs(hdr->checksum));
	println("urgent_pointer = %d", ntohs(hdr->urgent_pointer));
}

void show_udp_header(struct udp_header *hdr)
{
	println("src_port = %d", ntohs(hdr->src_port));
	println("dest_port = %d", ntohs(hdr->dest_port));
	println("udp_length = %d", ntohs(hdr->udp_length));
	println("udp_checksum = 0x%04x", hdr->udp_checksum);
}

void show_arp_header(struct arp_header *hdr, int simple)
{
	if (simple == 0)
	{
		println("hardware_type = %d", ntohs(hdr->hardware_type));
		println("protocol_type = %d", ntohs(hdr->protocol_type));
		println("hardware_addrlen = %d", hdr->hardware_addrlen);
		println("protocol_addrlen = %d", hdr->protocol_addrlen);
		println("op_code = %d", ntohs(hdr->op_code));
	}

	println("src_mac = %s", mac_address_tostring((char *)hdr->src_mac, sizeof(hdr->src_mac)));
	println("src_ip = %s", inet_ntoa(*(struct in_addr *)&hdr->src_ip));
	println("dest_mac = %s", mac_address_tostring((char *)hdr->dest_mac, sizeof(hdr->dest_mac)));
	println("dest_ip = %s", inet_ntoa(*(struct in_addr *)&hdr->dest_ip));
}

void show_dhcp_header(struct dhcp_header *hdr)
{
	println("opcode = %d", hdr->opcode);
	println("htype = %d", hdr->htype);
	println("hlen = %d", hdr->hlen);
	println("hops = %d", hdr->hops);
	println("transction_id = 0x%08x", ntohl(hdr->transction_id));
	println("seconds = %d", ntohs(hdr->seconds));
	println("flags = 0x%04x", ntohs(hdr->flags));
	println("ciaddr = %s", inet_ntoa(*(struct in_addr *)&hdr->ciaddr));
	println("yiaddr = %s", inet_ntoa(*(struct in_addr *)&hdr->yiaddr));
	println("siaddr = %s", inet_ntoa(*(struct in_addr *)&hdr->siaddr));
	println("giaddr = %s", inet_ntoa(*(struct in_addr *)&hdr->giaddr));
	println("chaddr = %s", mac_address_tostring((char *)hdr->chaddr, MAC_ADDRESS_LEN));
	println("sname = %s", hdr->sname);
}

void show_icmp_header(struct icmp_header *hdr)
{
	println("type = %d", hdr->type);
	println("code = %d", hdr->code);
	println("checksum = 0x%04x", hdr->checksum);
}

int cavan_route_table_init(struct cavan_route_table *table, size_t table_size)
{
	struct cavan_route_node **pp, **pp_end;

	pp = malloc(table_size * sizeof(void *));
	if (pp == NULL)
	{
		return -ENOMEM;
	}

	table->route_table = pp;

	for (pp_end = pp + table_size; pp < pp_end; pp++)
	{
		*pp = NULL;
	}

	table->table_size = table_size;

	return 0;
}

void cavan_route_table_uninit(struct cavan_route_table *table)
{
	if (table)
	{
		free(table->route_table);
		table->table_size = 0;
	}
}

int cavan_route_table_insert_node(struct cavan_route_table *table, struct cavan_route_node *node)
{
	struct cavan_route_node **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end && *pp; pp++);

	if (pp < pp_end)
	{
		*pp = node;
		return 0;
	}

	return -ENOMEM;
}

struct cavan_route_node **cavan_find_route_by_mac(struct cavan_route_table *table, u8 *mac)
{
	struct cavan_route_node *p, **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end; pp++)
	{
		p = *pp;

		if (p && memcmp(mac, p->mac_addr, sizeof(MAC_ADDRESS_LEN)))
		{
			return pp;
		}
	}

	return NULL;
}

struct cavan_route_node **cavan_find_route_by_ip(struct cavan_route_table *table, u32 ip)
{
	struct cavan_route_node *p, **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end; pp++)
	{
		p = *pp;

		if (p && p->ip_addr == ip)
		{
			return pp;
		}
	}

	return NULL;
}

int cavan_route_table_delete_node(struct cavan_route_table *table, struct cavan_route_node *node)
{
	struct cavan_route_node **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end && *pp != node; pp++);

	if (pp < pp_end)
	{
		*pp = NULL;
		return 0;
	}

	return -ENOENT;
}

int cavan_route_table_delete_by_mac(struct cavan_route_table *table, u8 *mac)
{
	struct cavan_route_node **pp;

	pp = cavan_find_route_by_mac(table, mac);
	if (pp)
	{
		*pp = NULL;
		return 0;
	}

	return -ENOENT;
}

int cavan_route_table_delete_by_ip(struct cavan_route_table *table, u32 ip)
{
	struct cavan_route_node **pp;

	pp = cavan_find_route_by_ip(table, ip);
	if (pp)
	{
		*pp = NULL;
		return 0;
	}

	return -ENOENT;
}

u16 udp_checksum(struct ip_header *ip_hdr)
{
	struct udp_pseudo_header udp_pseudo_hdr;
	struct udp_header *udp_hdr;
	u32 checksum;

	udp_hdr = (void *)(ip_hdr + 1);

	udp_pseudo_hdr.src_ip = ip_hdr->src_ip;
	udp_pseudo_hdr.dest_ip = ip_hdr->dest_ip;
	udp_pseudo_hdr.protocol = ip_hdr->protocol_type;
	udp_pseudo_hdr.zero = 0;
	udp_pseudo_hdr.udp_length = udp_hdr->udp_length;
	udp_hdr->udp_checksum = 0;

	checksum = checksum16((u16 *)&udp_pseudo_hdr, sizeof(udp_pseudo_hdr));
	checksum += checksum16((u16 *)udp_hdr, ntohs(udp_hdr->udp_length));

	return ~((checksum + (checksum >> 16)) & 0xFFFF);
}

void inet_sockaddr_init(struct sockaddr_in *addr, const char *ip, u16 port)
{
	println("IP = %s, port = %d", ip ? ip : "INADDR_ANY", port);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
}

int inet_create_tcp_link1(const struct sockaddr_in *addr)
{
	int ret;
	int sockfd;

	sockfd = inet_socket(SOCK_STREAM);
	if (sockfd < 0)
	{
		print_error("socket");
		return sockfd;
	}

	ret = inet_connect(sockfd, addr);
	if (ret < 0)
	{
		print_error("inet_connect");
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int inet_create_tcp_link2(const char *ip, u16 port)
{
	struct sockaddr_in addr;

	inet_sockaddr_init(&addr, ip, port);

	return inet_create_tcp_link1(&addr);
}

int inet_create_service(int type, u16 port)
{
	int ret;
	int sockfd;
	struct sockaddr_in addr;

	sockfd = inet_socket(type);
	if (sockfd < 0)
	{
		print_error("socket");
		return sockfd;
	}

	inet_sockaddr_init(&addr, NULL, port);

	ret = inet_bind(sockfd, &addr);
	if (ret < 0)
	{
		print_error("bind to port %d failed", port);
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int inet_create_tcp_service(u16 port)
{
	int ret;
	int sockfd;

	sockfd = inet_create_service(SOCK_STREAM, port);
	if (sockfd < 0)
	{
		return sockfd;
	}

	ret = inet_listen(sockfd);
	if (ret < 0)
	{
		print_error("listen to port %d failed", port);
		close(sockfd);
		return ret;
	}

	return sockfd;
}

ssize_t inet_tcp_sendto(struct sockaddr_in *addr, const void *buff, size_t size)
{
	int sockfd;
	ssize_t sendlen;

	sockfd = inet_create_tcp_link1(addr);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link1");
		return sockfd;
	}

	sendlen = inet_send(sockfd, buff, size);

	close(sockfd);

	return sendlen;
}

u32 get_rand_value(void)
{
	static int seeded;

	if (seeded == 0)
	{
		struct timeval tv;

		gettimeofday(&tv, NULL);
		srand(tv.tv_usec);
		seeded = 1;
	}

	return rand();
}

int inet_bind_rand(int sockfd, int retry)
{
	int ret;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	do {
		addr.sin_port = get_rand_value() & 0xFFFF;

		ret = inet_bind(sockfd, &addr);
	} while (ret < 0 && retry--);

	return ret < 0 ? ret : (int)ntohs(addr.sin_port);
}

int inet_tcp_send_file1(int sockfd, int fd)
{
	ssize_t readlen, sendlen;
	char buff[1024];

	while (1)
	{
		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		sendlen = inet_send(sockfd, buff, readlen);
		if (sendlen < 0)
		{
			print_error("inet_send");
			return sendlen;
		}
	}

	return 0;
}

int inet_tcp_send_file2(int sockfd, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s failed", filename);
		return fd;
	}

	ret = inet_tcp_send_file1(sockfd, fd);

	close(fd);

	return ret;
}

int inet_tcp_receive_file1(int sockfd, int fd)
{
	ssize_t recvlen, writelen;
	char buff[1024];

	while (1)
	{
		recvlen = inet_recv(sockfd, buff, sizeof(buff));
		if (recvlen < 0)
		{
			print_error("inet_recv");
			return recvlen;
		}

		if (recvlen == 0)
		{
			break;
		}

		writelen = write(fd, buff, recvlen);
		if (writelen < 0)
		{
			print_error("write");
			return writelen;
		}
	}

	return 0;
}

int inet_tcp_receive_file2(int sockfd, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_WRONLY | O_CREAT, 0777);
	if (fd < 0)
	{
		print_error("open file %s failed", filename);
		return fd;
	}

	ret = inet_tcp_receive_file1(sockfd, fd);

	close(fd);

	return ret;
}

int inet_get_sockaddr(int sockfd, const char *devname, struct sockaddr_in *sin_addr)
{
	int ret;
	struct ifreq ifr;

	text_copy(ifr.ifr_ifrn.ifrn_name, devname);

	ret = ioctl(sockfd, SIOCGIFADDR, &ifr);
	if (ret < 0)
	{
		print_error("get deivce %s sockaddr", devname);
		return ret;
	}

	*sin_addr = *(struct sockaddr_in *)&ifr.ifr_addr;

	return 0;
}

int inet_get_devname(int sockfd, int index, char *devname)
{
	int ret;
	struct ifreq req;

	req.ifr_ifru.ifru_ivalue = index;
	ret = ioctl(sockfd, SIOCGIFNAME, &req);
	if (ret < 0)
	{
		print_error("get devices name");
		return ret;
	}

	text_copy(devname, req.ifr_ifrn.ifrn_name);

	return 0;
}

char *cavan_get_server_ip(char *buff)
{
	const char *ip;

	ip = getenv(CAVAN_IP_ENV_NAME);
	if (ip == NULL)
	{
		ip = CAVAN_DEFAULT_IP;
	}

	return text_copy(buff, ip);
}

u16 cavan_get_server_port(u16 default_port)
{
	const char *port;

	port = getenv(CAVAN_PORT_ENV_NAME);
	if (port == NULL)
	{
		return default_port;
	}

	return text2value_unsigned(port, NULL, 10);
}

int inet_tcp_transmit_loop(int src_sockfd, int dest_sockfd)
{
	char buff[1024];
	ssize_t rwlen;

	while (1)
	{
		rwlen = inet_recv(src_sockfd, buff, sizeof(buff));
		if (rwlen < 0)
		{
			pr_red_info("inet_recv");
			return rwlen;
		}

		if (rwlen == 0)
		{
			break;
		}

		buff[rwlen] = 0;
		println("buff = %s", buff);

		rwlen = inet_send(dest_sockfd, buff, rwlen);
		if (rwlen < 0)
		{
			pr_red_info("inet_send");
			return rwlen;
		}
	}

	return 0;
}
