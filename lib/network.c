// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 21 10:08:25 CST 2011

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/file.h>
#include <cavan/command.h>
#include <cavan/android.h>
#include <cavan/network.h>
#include <cavan/progress.h>
#include <cavan/permission.h>

#if CONFIG_CAVAN_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define CAVAN_NETWORK_DEBUG				0
#define CAVAN_IFCONFIG_DEBUG			0
#define CAVAN_NETWORK_TRANSMIT_THREAD	0

const char *network_get_socket_pathname(void)
{
#if 0
	static char pathname[1024];

	if (pathname[0]) {
		return pathname;
	}

	return cavan_path_build_tmp_path("cavan/network/socket", pathname, sizeof(pathname));
#else
	return "/dev/cavan/network/socket";
#endif
}

char *network_get_hostname(char *buff, size_t size)
{
	const char *hostname;

	hostname = getenv("HOSTNAME");
	if (hostname != NULL) {
		strncpy(buff, hostname, size);
		return buff;
	}

	if (android_get_hostname(buff, sizeof(buff)) > 0) {
		return buff;
	}

	if (file_read_text("/proc/sys/kernel/hostname", buff, size) > 0) {
		return buff;
	}

	strncpy(buff, "unknown", size);

	return buff;
}

const char *inet_get_special_address(const char *hostname)
{
	if (hostname == NULL || hostname[0] == 0) {
		return "127.0.0.1";
	}

	switch (*hostname++) {
	case 'l':
		if (strcmp(hostname, "ocalhost") == 0) {
			return "127.0.0.1";
		}

		if (strcmp(hostname, "oopback") == 0) {
			return "127.0.0.1";
		}
		break;

	case 'b':
		if (strcmp(hostname, "roadcast") == 0) {
			return "255.255.255.255";
		}
		break;

	case 'a':
		if (strcmp(hostname, "ny") == 0) {
			return "0.0.0.0";
		}

		if (text_lhcmp("ll", hostname) == 0) {
			hostname += 2;
			if (*hostname == '-') {
				hostname++;
			}

			if (strcmp(hostname, "hosts") == 0) {
				return "224.0.0.1";
			}

			if (strcmp(hostname, "rtrs") == 0) {
				return "224.0.0.2";
			}
		}
		break;

	case 'u':
		if (strcmp(hostname, "nspec") == 0) {
			return "224.0.0.0";
		}
		break;
	}

	return NULL;
}


bool inet_addr_is_broadcast(struct sockaddr_in *addr)
{
	uint32_t value = ntohl(addr->sin_addr.s_addr);

	if ((value & 0xFF) == 0xFF) {
		return true;
	}

	if (value == CAVAN_BUILD_IP_ADDR(224, 0, 0, 1)) {
		return true;
	}

	return false;
}

const char *inet_check_hostname(const char *hostname, char *buff, size_t size)
{
	const char *special = inet_get_special_address(hostname);

	if (special) {
		hostname = special;
	} else if (buff && size > 0) {
		int count;
		int values[4];
		const char *last;

		count = text2value_array2(hostname, &last, '.', values, NELEM(values), 10);
		if (*last == 0 && count < 4) {
			int length = android_get_wifi_ipaddress(buff, size);

			if (length > 0) {
				char *last_dot;

				for (last_dot = buff + length - 1; last_dot > buff && *last_dot != '.'; last_dot--);

				strncpy(last_dot + 1, hostname, size - length);
			} else {
				int i, j;
				struct cavan_inet_route route;
				u8 *addr = (u8 *) &route.gateway.sin_addr.s_addr;

				if (cavan_inet_get_default_route(&route) < 0) {
					addr[0] = 192;
					addr[1] = 168;
					addr[2] = 0;
					addr[3] = 1;
				}

				for (i = count - 1, j = 3; i >= 0; i--, j--) {
					values[j] = values[i];
				}

				for (i = 4 - count - 1; i >= 0; i--) {
					values[i] = addr[i];
				}

				value2text_array2(values, NELEM(values), '.', buff, size, 10);
			}

			hostname = buff;
		}
	}

#if CAVAN_NETWORK_DEBUG
	println("hostname = %s", hostname);
#endif

	return hostname;
}

ssize_t sendto_select(int sockfd, int retry, const void *buff, size_t len, const struct sockaddr_in *remote_addr)
{
	while (retry--) {
		ssize_t sendlen;

		sendlen = inet_sendto(sockfd, buff, len, remote_addr);
		if (sendlen < 0) {
			pr_err_info("send data failed");
			return sendlen;
		}

		if (file_poll_input(sockfd, NETWORK_TIMEOUT_VALUE)) {
			return sendlen;
		}

		println("timeout retry = %d", retry);
	}

	return -ETIMEDOUT;
}

ssize_t sendto_receive(int sockfd, long timeout, int retry, const void *send_buff, ssize_t sendlen, void *recv_buff, ssize_t recvlen, struct sockaddr_in *remote_addr, socklen_t *addr_len)
{
	sendlen = sendto_select(sockfd, retry, send_buff, sendlen, remote_addr);
	if (sendlen < 0) {
		pr_err_info("send data timeout");
		return sendlen;
	}

	return inet_recvfrom(sockfd, recv_buff, recvlen, remote_addr, addr_len);
}

const char *mac_protocol_type_tostring(int type)
{
	switch (type) {
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
	switch (type) {
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
	pr_bold_info("MAC Header:");
	println("dest_mac = %s", mac_address_tostring((char *) hdr->dest_mac, sizeof(hdr->dest_mac)));
	println("src_mac = %s", mac_address_tostring((char *) hdr->src_mac, sizeof(hdr->src_mac)));
	println("protocol_type = %s", mac_protocol_type_tostring(ntohs(hdr->protocol_type)));
}

void show_ip_header(struct ip_header *hdr, int simple)
{
	pr_bold_info("IP Header:");

	if (simple == 0) {
		println("version = %d", hdr->version);
		println("header_length = %d", hdr->header_length);
		println("service_type = %d", hdr->service_type);
		println("total_length = %d", ntohs(hdr->total_length));
		println("flags = %d", hdr->flags);
		println("piece_offset = %d", hdr->piece_offset);
		println("ttl = %d", hdr->ttl);
		println("header_checksum = 0x%04x", ntohs(hdr->header_checksum));
	}

	println("protocol_type = %s", ip_protocol_type_tostring(hdr->protocol_type));
	println("src_ip = %s", inet_ntoa(*(struct in_addr *) &hdr->src_ip));
	println("dest_ip = %s", inet_ntoa(*(struct in_addr *) &hdr->dest_ip));
}

void show_tcp_header(struct tcp_header *hdr)
{
	pr_bold_info("TCP Header:");
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
	pr_bold_info("UDP Header:");
	println("src_port = %d", ntohs(hdr->src_port));
	println("dest_port = %d", ntohs(hdr->dest_port));
	println("udp_length = %d", ntohs(hdr->udp_length));
	println("udp_checksum = 0x%04x", hdr->udp_checksum);
}

void show_arp_header(struct arp_header *hdr, int simple)
{
	pr_bold_info("ARP Header:");

	if (simple == 0) {
		println("hardware_type = %d", ntohs(hdr->hardware_type));
		println("protocol_type = %d", ntohs(hdr->protocol_type));
		println("hardware_addrlen = %d", hdr->hardware_addrlen);
		println("protocol_addrlen = %d", hdr->protocol_addrlen);
		println("op_code = %d", ntohs(hdr->op_code));
	}

	println("src_mac = %s", mac_address_tostring((char *) hdr->src_mac, sizeof(hdr->src_mac)));
	println("src_ip = %s", inet_ntoa(*(struct in_addr *) &hdr->src_ip));
	println("dest_mac = %s", mac_address_tostring((char *) hdr->dest_mac, sizeof(hdr->dest_mac)));
	println("dest_ip = %s", inet_ntoa(*(struct in_addr *) &hdr->dest_ip));
}

void show_dhcp_header(struct dhcp_header *hdr)
{
	pr_bold_info("DHCP Header:");

	println("opcode = %d", hdr->opcode);
	println("htype = %d", hdr->htype);
	println("hlen = %d", hdr->hlen);
	println("hops = %d", hdr->hops);
	println("transction_id = 0x%08x", ntohl(hdr->transction_id));
	println("seconds = %d", ntohs(hdr->seconds));
	println("flags = 0x%04x", ntohs(hdr->flags));
	println("ciaddr = %s", inet_ntoa(*(struct in_addr *) &hdr->ciaddr));
	println("yiaddr = %s", inet_ntoa(*(struct in_addr *) &hdr->yiaddr));
	println("siaddr = %s", inet_ntoa(*(struct in_addr *) &hdr->siaddr));
	println("giaddr = %s", inet_ntoa(*(struct in_addr *) &hdr->giaddr));
	println("chaddr = %s", mac_address_tostring((char *) hdr->chaddr, MAC_ADDRESS_LEN));
	println("sname = %s", hdr->sname);
}

void show_icmp_header(struct icmp_header *hdr)
{
	pr_bold_info("ICMP Header:");

	println("type = %d", hdr->type);
	println("code = %d", hdr->code);
	println("checksum = 0x%04x", hdr->checksum);
}

void show_ping_header(struct ping_header *hdr)
{
	pr_bold_info("PING Header:");

	println("id = 0x%04x", hdr->id);
	println("seq = 0x%04x", hdr->seq);
}

void inet_show_sockaddr(const struct sockaddr_in *addr)
{
	u16 port = ntohs(addr->sin_port);

	if (port && port != NETWORK_PORT_INVALID) {
		println("IP = %s, PORT = %d", inet_ntoa(addr->sin_addr), port);
	} else {
		println("IP = %s", inet_ntoa(addr->sin_addr));
	}
}

int cavan_route_table_init(struct cavan_route_table *table, size_t table_size)
{
	struct cavan_route_node **pp, **pp_end;

	pp = malloc(table_size * sizeof(void *));
	if (pp == NULL) {
		return -ENOMEM;
	}

	table->route_table = pp;

	for (pp_end = pp + table_size; pp < pp_end; pp++) {
		*pp = NULL;
	}

	table->table_size = table_size;

	return 0;
}

void cavan_route_table_deinit(struct cavan_route_table *table)
{
	if (table) {
		free(table->route_table);
		table->table_size = 0;
	}
}

int cavan_route_table_insert_node(struct cavan_route_table *table, struct cavan_route_node *node)
{
	struct cavan_route_node **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end && *pp; pp++);

	if (pp < pp_end) {
		*pp = node;
		return 0;
	}

	return -ENOMEM;
}

struct cavan_route_node **cavan_find_route_by_mac(struct cavan_route_table *table, u8 *mac)
{
	struct cavan_route_node *p, **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end; pp++) {
		p = *pp;

		if (p && memcmp(mac, p->mac_addr, sizeof(p->mac_addr))) {
			return pp;
		}
	}

	return NULL;
}

struct cavan_route_node **cavan_find_route_by_ip(struct cavan_route_table *table, u32 ip)
{
	struct cavan_route_node *p, **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end; pp++) {
		p = *pp;

		if (p && p->ip_addr == ip) {
			return pp;
		}
	}

	return NULL;
}

int cavan_route_table_delete_node(struct cavan_route_table *table, struct cavan_route_node *node)
{
	struct cavan_route_node **pp, **pp_end;

	for (pp = table->route_table, pp_end = pp + table->table_size; pp < pp_end && *pp != node; pp++);

	if (pp < pp_end) {
		*pp = NULL;
		return 0;
	}

	return -ENOENT;
}

int cavan_route_table_delete_by_mac(struct cavan_route_table *table, u8 *mac)
{
	struct cavan_route_node **pp;

	pp = cavan_find_route_by_mac(table, mac);
	if (pp) {
		*pp = NULL;
		return 0;
	}

	return -ENOENT;
}

int cavan_route_table_delete_by_ip(struct cavan_route_table *table, u32 ip)
{
	struct cavan_route_node **pp;

	pp = cavan_find_route_by_ip(table, ip);
	if (pp) {
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

	udp_hdr = (void *) (ip_hdr + 1);

	udp_pseudo_hdr.src_ip = ip_hdr->src_ip;
	udp_pseudo_hdr.dest_ip = ip_hdr->dest_ip;
	udp_pseudo_hdr.protocol = ip_hdr->protocol_type;
	udp_pseudo_hdr.zero = 0;
	udp_pseudo_hdr.udp_length = udp_hdr->udp_length;
	udp_hdr->udp_checksum = 0;

	checksum = mem_checksum16_simple((u16 *) &udp_pseudo_hdr, sizeof(udp_pseudo_hdr));
	checksum += mem_checksum16_simple((u16 *) udp_hdr, ntohs(udp_hdr->udp_length));

	return ~((checksum + (checksum >> 16)) & 0xFFFF);
}

void inet_sockaddr_init(struct sockaddr_in *addr, const char *ip, u16 port)
{
	pd_bold_info("IP = %s, PORT = %d", ip ? ip : "INADDR_ANY", port);

	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
}

void unix_sockaddr_init(struct sockaddr_un *addr, const char *pathname)
{
	pd_bold_info("SUN_PATH = %s", pathname);

	addr->sun_family = AF_UNIX;
	strncpy(addr->sun_path, pathname, sizeof(addr->sun_path));
}

int inet_create_tcp_link1(const struct sockaddr_in *addr)
{
	int ret;
	int sockfd;

	sockfd = inet_socket(SOCK_STREAM);
	if (sockfd < 0) {
		pr_err_info("socket");
		return sockfd;
	}

	ret = inet_connect(sockfd, addr);
	if (ret < 0) {
		pr_err_info("inet_connect");
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int unix_create_tcp_link(const char *hostname, u16 port)
{
	int ret;
	int sockfd;
	struct sockaddr_un addr;

	sockfd = unix_socket(SOCK_STREAM);
	if (sockfd < 0) {
		pr_error_info("socket");
		return sockfd;
	}

	unix_sockaddr_init(&addr, hostname);

	ret = unix_connect(sockfd, &addr);
	if (ret < 0) {
		pr_err_info("inet_connect");
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int inet_create_tcp_link_by_addrinfo(struct addrinfo *info, u16 port, struct sockaddr_in *addr)
{
	int sockfd;

	sockfd = inet_socket(SOCK_STREAM);
	if (sockfd < 0) {
		pr_error_info("inet_socket");
		return sockfd;
	}

	while (info) {
		if (info->ai_family == AF_INET) {
			struct sockaddr_in *p = (struct sockaddr_in *) info->ai_addr;

			p->sin_port = htons(port);
			if (inet_connect(sockfd, p) == 0) {
				if (addr) {
					addr->sin_addr.s_addr = p->sin_addr.s_addr;
				}

				return sockfd;
			}
		}

		info = info->ai_next;
	}

	close(sockfd);

	return -ENOENT;
}

int inet_create_tcp_link2(const char *hostname, u16 port)
{
	int ret;
	int sockfd;
	char buff[64];
	struct sockaddr_in addr;

	hostname = inet_check_hostname(hostname, buff, sizeof(buff));

	if (inet_aton(hostname, &addr.sin_addr)) {
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		sockfd = inet_create_tcp_link1(&addr);
	} else {
		struct addrinfo *info;
		struct addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		ret = getaddrinfo(hostname, NULL, &hints, &info);
		if (ret < 0 || info == NULL) {
			pr_error_info("getaddrinfo");
			return -ENOENT;
		}

		sockfd = inet_create_tcp_link_by_addrinfo(info, port, &addr);
		freeaddrinfo(info);
	}

	if (sockfd < 0) {
		return sockfd;
	}

	pd_info("%s => %s:%d", hostname, inet_ntoa(addr.sin_addr), port);

	return sockfd;
}

int inet_create_service(int type, u16 port)
{
	int ret;
	int sockfd;
	struct sockaddr_in addr;

	sockfd = inet_socket(type);
	if (sockfd < 0) {
		pr_err_info("socket");
		return sockfd;
	}

	ret = socket_set_reuse_addr(sockfd);
	if (ret < 0) {
		pr_err_info("socket_set_reuse_addr");
		goto out_close_sockfd;
	}

	inet_sockaddr_init(&addr, NULL, port);

	ret = inet_bind(sockfd, &addr);
	if (ret < 0) {
		pr_err_info("bind to port %d failed", port);
		goto out_close_sockfd;
	}

	return sockfd;

out_close_sockfd:
	close(sockfd);
	return ret;
}

int unix_create_service(int type, const char *pathname)
{
	int ret;
	int sockfd;
	socklen_t addrlen;
	struct sockaddr_un addr;

	sockfd = unix_socket(type);
	if (sockfd < 0) {
		pr_err_info("socket");
		return sockfd;
	}

	if (pathname && pathname[0]) {
		if (file_access_e(pathname)) {
			unlink(pathname);
		} else {
			ret = mkdir_parent_hierarchy(pathname, 0777);
			if (ret < 0) {
				pr_red_info("mkdir_hierarchy");
				goto out_close_sockfd;
			}
		}

		unix_sockaddr_init(&addr, pathname);
		addrlen = sizeof(struct sockaddr_un);
	} else {
		addr.sun_family = AF_UNIX;
		addrlen = sizeof(addr.sun_family);
	}

	ret = bind(sockfd, (struct sockaddr *) &addr, addrlen);
	if (ret < 0) {
		pr_err_info("bind");
		goto out_close_sockfd;
	}

	return sockfd;

out_close_sockfd:
	close(sockfd);
	return ret;
}

int inet_create_tcp_service(u16 port)
{
	int ret;
	int sockfd;

	sockfd = inet_create_service(SOCK_STREAM, port);
	if (sockfd < 0) {
		return sockfd;
	}

	ret = inet_listen(sockfd);
	if (ret < 0) {
		pr_error_info("listen to port %d failed", port);
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int unix_create_tcp_service(const char *pathname)
{
	int ret;
	int sockfd;

	sockfd = unix_create_service(SOCK_STREAM, pathname);
	if (sockfd < 0) {
		return sockfd;
	}

	ret = inet_listen(sockfd);
	if (ret < 0) {
		pr_error_info("listen to socket %s failed", pathname);
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
	if (sockfd < 0) {
		pr_err_info("inet_create_tcp_link1");
		return sockfd;
	}

	sendlen = inet_send(sockfd, buff, size);

	close(sockfd);

	return sendlen;
}

u32 get_rand_value(void)
{
	static int seeded;

	if (seeded == 0) {
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
	socklen_t addrlen;
	struct sockaddr_in addr;

	addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

#if 0
	while (1) {
		addr.sin_port = get_rand_value() & 0xFFFF;

		ret = bind(sockfd, (struct sockaddr *) &addr, addrlen);
		if (ret == 0) {
			break;
		}

		if (retry < 1) {
			return ret;
		}

		retry--;
	}
#else
	addr.sin_port = 0;

	ret = bind(sockfd, (struct sockaddr *) &addr, addrlen);
	if (ret < 0) {
		pr_error_info("inet_bind");
		return ret;
	}

	ret = getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
	if (ret < 0) {
		pr_error_info("inet_getsockname");
		return ret;
	}
#endif

	inet_show_sockaddr(&addr);

	return (int) ntohs(addr.sin_port);
}

ssize_t inet_send(int sockfd, const char *buff, size_t size)
{
#if 0
	ssize_t wrlen;
	const char *buff_end;

	for (buff_end = buff + size; buff < buff_end; buff += wrlen) {
		wrlen = send(sockfd, buff, buff_end - buff, MSG_NOSIGNAL);
		if (wrlen <= 0) {
			return wrlen;
		}
	}

	return size;
#else
	int retry = 0;
	size_t size_bak = size;

	while (1) {
		ssize_t wrlen;

		wrlen = send(sockfd, buff, size, MSG_NOSIGNAL);
		if (likely(wrlen >= (ssize_t) size)) {
			break;
		}

		if (wrlen > 0) {
			retry = 0;
			buff += wrlen;
			size -= wrlen;
		} else {
			if (wrlen < 0) {
				if (ERRNO_NOT_RETRY()) {
					return wrlen;
				}

				msleep(100);
			}

			if (++retry > 10) {
				return -EFAULT;
			}
		}
	}

	return size_bak;
#endif
}

ssize_t inet_recv(int sockfd, char *buff, size_t size)
{
	int retry = 0;

	while (1) {
		ssize_t rdlen;

		rdlen = recv(sockfd, buff, size, MSG_NOSIGNAL);
		if (rdlen >= 0 || ERRNO_NOT_RETRY()) {
			return rdlen;
		}

		if (++retry > 10) {
			break;
		}

		msleep(100);
	}

	return -EFAULT;
}

int inet_tcp_send_file1(int sockfd, int fd)
{
	char buff[1024];
	ssize_t rdlen, wrlen;

	while (1) {
		rdlen = ffile_read(fd, buff, sizeof(buff));
		if (rdlen <= 0) {
			if (rdlen == 0) {
				break;
			}

			pr_err_info("ffile_read");
			return rdlen;
		}

		wrlen = inet_send(sockfd, buff, rdlen);
		if (wrlen < 0) {
			pr_err_info("inet_send");
			return wrlen;
		}
	}

	fsync(sockfd);

	return 0;
}

int inet_tcp_send_file2(int sockfd, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		pr_err_info("open file %s failed", filename);
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

	while (1) {
		recvlen = inet_recv(sockfd, buff, sizeof(buff));
		if (recvlen < 0) {
			pr_err_info("inet_recv");
			return recvlen;
		}

		if (recvlen == 0) {
			break;
		}

		writelen = write(fd, buff, recvlen);
		if (writelen < 0) {
			pr_err_info("write");
			return writelen;
		}
	}

	return 0;
}

int inet_tcp_receive_file2(int sockfd, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open file %s failed", filename);
		return fd;
	}

	ret = inet_tcp_receive_file1(sockfd, fd);

	close(fd);

	return ret;
}

int inet_tcp_transfer(int src_sockfd, int dest_sockfd, size_t size)
{
	ssize_t rdlen;
	char buff[size];

	rdlen = inet_recv(src_sockfd, buff, sizeof(buff));
	if (rdlen <= 0) {
		return rdlen;
	}

	return inet_send(dest_sockfd, buff, rdlen);
}

int inet_get_sockaddr(int sockfd, const char *devname, struct sockaddr_in *sin_addr)
{
	int ret;
	struct ifreq ifr;

	text_copy(ifr.ifr_ifrn.ifrn_name, devname);

	ret = ioctl(sockfd, SIOCGIFADDR, &ifr);
	if (ret < 0) {
		pr_err_info("get deivce %s sockaddr", devname);
		return ret;
	}

	*sin_addr = *(struct sockaddr_in *) &ifr.ifr_addr;

	return 0;
}

int inet_get_devname(int sockfd, int index, char *devname)
{
	int ret;
	struct ifreq req;

	req.ifr_ifru.ifru_ivalue = index;
	ret = ioctl(sockfd, SIOCGIFNAME, &req);
	if (ret < 0) {
		pr_err_info("get devices name");
		return ret;
	}

	text_copy(devname, req.ifr_ifrn.ifrn_name);

	return 0;
}

int inet_tcp_transmit_loop(int src_sockfd, int dest_sockfd)
{
	char buff[1024];
	ssize_t rwlen;

	while (1) {
		rwlen = inet_recv(src_sockfd, buff, sizeof(buff));
		if (rwlen < 0) {
			pr_red_info("inet_recv");
			return rwlen;
		}

		if (rwlen == 0) {
			break;
		}

		buff[rwlen] = 0;
		println("buff = %s", buff);

		rwlen = inet_send(dest_sockfd, buff, rwlen);
		if (rwlen < 0) {
			pr_red_info("inet_send");
			return rwlen;
		}
	}

	return 0;
}

int inet_hostname2sockaddr(const char *hostname, struct sockaddr_in *addr)
{
	int ret;
	char buff[64];
	struct addrinfo *res, *p;
	struct addrinfo hints;

	addr->sin_family = AF_INET;

	hostname = inet_check_hostname(hostname, buff, sizeof(buff));

	if (inet_aton(hostname, &addr->sin_addr)) {
		return 0;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	ret = getaddrinfo(hostname, NULL, &hints, &res);
	if (ret < 0) {
		pr_error_info("getaddrinfo");
		return ret;
	}

	if (res == NULL) {
		pr_red_info("res == NULL");
		return -ENOENT;
	}

	for (p = res; p; p = p->ai_next) {
		if (p->ai_family == AF_INET) {
			res = p;
			break;
		}
	}

	memcpy(addr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

	inet_show_sockaddr(addr);

	return 0;
}

int inet_create_link(const struct sockaddr_in *addr, int socktype, int protocol)
{
	int ret;
	int sockfd;

	sockfd = socket(PF_INET, socktype, protocol);
	if (sockfd < 0) {
		pr_error_info("socket");
		return sockfd;
	}

	ret = inet_connect(sockfd, addr);
	if (ret < 0) {
		pr_error_info("inet_connect");
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int network_create_link(const char *hostname, u16 port, int socktype, int protocol)
{
	int ret;
	int sockfd;
	char buff[64];
	struct addrinfo hints;
	struct addrinfo *res, *p;

	hostname = inet_check_hostname(hostname, buff, sizeof(buff));

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = socktype;
	hints.ai_protocol = protocol;

	ret = getaddrinfo(hostname, NULL, &hints, &res);
	if (ret < 0 || res == NULL) {
		struct sockaddr_in addr;

		ret = inet_aton(hostname, &addr.sin_addr);
		if (ret < 0) {
			pr_error_info("inet_aton");
			return ret;
		}

		addr.sin_port = htons(port);

		return inet_create_link(&addr, socktype, protocol);
	}

	for (p = res; p; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd < 0) {
			continue;
		}

		if (p->ai_family == AF_INET && port != NETWORK_PORT_INVALID) {
			struct sockaddr_in *addr = (struct sockaddr_in *) p->ai_addr;

			addr->sin_port = htons(port);
		}

		ret = connect(sockfd, p->ai_addr, p->ai_addrlen);
		if (ret < 0) {
			close(sockfd);
		} else {
#if CAVAN_NETWORK_DEBUG
			pr_info("HOST = %s, %s", hostname, network_sockaddr_tostring(p->ai_addr, NULL, 0));
#endif
			goto out_freeaddrinfo;
		}
	}

	sockfd = -EFAULT;

out_freeaddrinfo:
	freeaddrinfo(res);
	return sockfd;
}

static int network_sockaddr_in_tostring(const struct sockaddr_in *addr, char *buff, size_t size)
{
	u16 port = ntohs(addr->sin_port);

	if (port && port != NETWORK_PORT_INVALID) {
		return snprintf(buff, size, "IP = %s, PORT = %d", inet_ntoa(addr->sin_addr), port);
	} else {
		return snprintf(buff, size, "IP = %s", inet_ntoa(addr->sin_addr));
	}
}

static int network_sockaddr_un_tostring(const struct sockaddr_un *addr, char *buff, size_t size)
{
	return snprintf(buff, size, "SUN_PATH = %s", addr->sun_path);
}

char *network_sockaddr_tostring(const struct sockaddr *addr, char *buff, size_t size)
{
	if (buff == NULL || size == 0) {
		static char static_buff[128];

		buff = static_buff;
		size = sizeof(static_buff);
	}

	switch (addr->sa_family) {
	case AF_INET:
		network_sockaddr_in_tostring((const struct sockaddr_in *) addr, buff, size);
		break;

	case AF_UNIX:
		network_sockaddr_un_tostring((const struct sockaddr_un *) addr, buff, size);
		break;

	default:
		*buff = 0;
	}

	return buff;
}

void network_url_init(struct network_url *url, const char *protocol, const char *hostname, u16 port, const char *pathname)
{
	url->port = port;
	url->pathname = pathname ? pathname : "";
	url->protocol = protocol ? protocol : CAVAN_DEFAULT_PROTOCOL;
	url->hostname = hostname ? hostname : cavan_get_server_hostname();
}

char *network_url_get_pathname(const struct network_url *url, char *buff, size_t size)
{
	const char *p;
	char *buff_end = buff + size;

	for (p = url->pathname; buff < buff_end; p++, buff++) {
		switch (*p) {
		case '\0':
		case '\r':
		case '\n':
		case '\f':
			*buff = 0;
			return buff;

		default:
			*buff = *p;
		}

	}

	return buff;
}

char *network_url_tostring(const struct network_url *url, char *buff, size_t size, char **tail)
{
	char *buff_bak, *buff_end;

	if (buff == NULL || size == 0) {
		static char static_buff[1024];

		buff = static_buff;
		size = sizeof(static_buff);
	}

	buff_bak = buff;
	buff_end = buff + size;

	if (url->protocol && url->protocol[0]) {
		buff += snprintf(buff, buff_end - buff, "%s:", url->protocol);
	}

	if (url->hostname && url->hostname[0]) {
		buff += snprintf(buff, buff_end - buff, "//%s", url->hostname);
	} else {
		buff = text_ncopy(buff, "//", buff_end - buff);
	}

	if (url->port != NETWORK_PORT_INVALID) {
		buff += snprintf(buff, buff_end - buff, ":%d", url->port);
	}

	if (url->pathname && url->pathname[0]) {
		buff = network_url_get_pathname(url, buff, buff_end - buff);
	}

	if (tail) {
		*tail = buff;
	}

	return buff_bak;
}

char *network_url_parse(struct network_url *url, const char *text)
{
	int slash = 0;
	int colon = 0;
	const char *port = NULL;
	char *p = url->memory;
	char *p_end = p + sizeof(url->memory);

	url->hostname = p;
	url->protocol = NULL;

	while (p < p_end) {
		switch (*text) {
		case '/':
			slash++;
			if (text[1] == '/' && slash == 1 && port == NULL) {
				if (p > url->memory && url->protocol == NULL) {
					pr_red_info("Invalid `/' at %s", text);
					return NULL;
				}

				slash = 2;
				text += 2;
				break;
			}
		case 0 ... 31:
		case ' ':
			*p = 0;

			if (slash == 0 && colon == 1 && url->protocol && p > url->hostname) {
				port = url->hostname;
				url->hostname = url->protocol;
				url->protocol = NULL;
			}

			url->port = port ? text2value_unsigned(port, NULL, 10) : NETWORK_PORT_INVALID;

			if (url->protocol == NULL) {
				url->protocol = p;
			}

			if (url->hostname[0] == 0 || strcmp(url->hostname, "localhost") == 0) {
				url->hostname = LOCAL_HOST_IP;
			}

			url->pathname = text;

			return (char *) text;

		case ':':
			colon++;
			if (colon > 2) {
				pr_red_info("Too much `:' at %s", text);
				return NULL;
			}

			text++;
			*p = 0;

			if (colon == 1 && slash == 0 && url->protocol == NULL && url->hostname < p) {
				url->protocol = url->hostname;
				url->hostname = ++p;
			} else {
				for (port = ++p; p < p_end && IS_NUMBER(*text); p++, text++) {
					*p = *text;
				}

				if (p == port) {
					pr_red_info("Please give the port");
					return NULL;
				}
			}

			break;

		default:
			*p++ = *text++;
		}
	}

	return NULL;
}

bool network_url_equals(const struct network_url *url1, const struct network_url *url2)
{
	if (url1 == url2) {
		return true;
	}

	if (url1->port != url2->port) {
		return false;
	}

	if (text_cmp(url1->hostname, url2->hostname)) {
		return false;
	}

	if (text_cmp(url1->protocol, url2->protocol)) {
		return false;
	}

	return true;
}

static void network_url_show_usage(const char *command)
{
	println("Usage: %s [OPTION]", command);
	println("-H, -h, --help\t\t\t%s", cavan_help_message_help);
	println("-S, -s, --super\t\t\t%s", cavan_help_message_super);
	println("-d, --daemon\t\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t%s", cavan_help_message_daemon_max);
	println("-V, -v, --verbose\t\t%s", cavan_help_message_verbose);
	println("--user USERNAME\t\t\t%s", cavan_help_message_user);
	println("--group GROUPNAME\t\t%s", cavan_help_message_group);
	println("-I, -i, --ip IP\t\t\t%s", cavan_help_message_ip);
	println("--host [HOSTNAME]\t\t%s", cavan_help_message_hostname);
	println("-L, -l, ---local\t\t%s", cavan_help_message_local);
	println("-p, --port PORT\t\t\t%s", cavan_help_message_port);
	println("-A, -a, --adb\t\t\t%s", cavan_help_message_adb);
	println("--udp\t\t\t\t%s", cavan_help_message_udp);
	println("--udp\t\t\t\t%s", cavan_help_message_tcp);
	println("--unix, --unix-tcp [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t%s", cavan_help_message_unix_udp);
	println("-P, --pt, --protocol PROTOCOL\t%s", cavan_help_message_protocol);
	println("-U, -u, --url [URL]\t\t%s", cavan_help_message_url);
}

int network_url_parse_cmdline(struct network_url *url, struct cavan_dynamic_service *service, int argc, char *argv[])
{
	int c;
	int option_index;
	static const struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		}, {
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		}, {
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		}, {
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SUPER,
		}, {
			.name = "user",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_USER,
		}, {
			.name = "group",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_GROUP,
		}, {
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		}, {
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		}, {
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		}, {
			.name = "udp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UDP,
		}, {
			.name = "tcp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TCP,
		}, {
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		}, {
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		}, {
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HOST,
		}, {
			.name = "unix",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX,
		}, {
			.name = "unix-tcp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_TCP,
		}, {
			.name = "unix-udp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_UDP,
		}, {
			.name = "protocol",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		}, {
			.name = "pt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		}, {
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "hHvVds:S:c:C:m:M:aA:i:I:p:P:lLu:U:", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			network_url_show_usage(argv[0]);
			return -EFAULT;

		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERBOSE:
			if (service != NULL) {
				service->verbose = true;
			}
			break;

		case 'd':
		case CAVAN_COMMAND_OPTION_DAEMON:
			if (service != NULL) {
				service->as_daemon = true;
			}
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			if (service != NULL) {
				service->min = text2value_unsigned(optarg, NULL, 10);
			}
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			if (service != NULL) {
				service->max = text2value_unsigned(optarg, NULL, 10);
			}
			break;

		case 's':
		case 'S':
		case CAVAN_COMMAND_OPTION_SUPER:
			if (service != NULL) {
				service->super_permission = text2bool(optarg);
			}
			break;

		case CAVAN_COMMAND_OPTION_USER:
			if (service != NULL) {
				cavan_parse_user_text(optarg, &service->user, &service->group);
			}
			break;

		case CAVAN_COMMAND_OPTION_GROUP:
			if (service != NULL) {
				service->group = optarg;
			}
			break;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			url->protocol = "adb";
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
		case CAVAN_COMMAND_OPTION_HOST:
			url->hostname = optarg;
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			url->protocol = "udp";
			break;

		case CAVAN_COMMAND_OPTION_TCP:
			url->protocol = "tcp";
			break;

		case 'p':
		case CAVAN_COMMAND_OPTION_PORT:
			url->port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(url, optarg) == NULL) {
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url->protocol = "unix-tcp";
			if (optarg) {
				url->pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url->protocol = "unix-udp";
			if (optarg) {
				url->pathname = optarg;
			}
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PROTOCOL:
			url->protocol = optarg;
			break;

		default:
			network_url_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	return 0;
}

int network_create_socket_mac(const char *if_name, int protocol)
{
	int ret;
	int sockfd;
	struct ifreq req;
	struct sockaddr_ll bind_addr;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(protocol ? protocol : ETH_P_ALL));
	if (sockfd < 0) {
		pr_error_info("socket PF_PACKET SOCK_RAW");
		return sockfd;
	}

	if (if_name == NULL || if_name[0] == 0) {
		return sockfd;
	}

	strcpy(req.ifr_name, if_name);

	ret = ioctl(sockfd, SIOCGIFINDEX, &req);
	if (ret < 0) {
		pr_error_info("ioctl SIOCGIFINDEX");
		goto out_close_socket;
	}

	bind_addr.sll_family = PF_PACKET;
	bind_addr.sll_ifindex = req.ifr_ifindex;
	bind_addr.sll_protocol = htons(ETH_P_ALL);

	ret = bind(sockfd, (struct sockaddr *) &bind_addr, sizeof(bind_addr));
	if (ret < 0) {
		pr_error_info("bind");
		goto out_close_socket;
	}

	return sockfd;

out_close_socket:
	close(sockfd);
	return ret;
}

int network_create_socket_uevent(void)
{
	int ret;
	int buffsize;
	int sockfd;
	struct sockaddr_nl addr;

	sockfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sockfd < 0) {
		pr_err_info("socket");
		return sockfd;
	}

	buffsize = KB(64);
	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUFFORCE, &buffsize, sizeof(buffsize));
	if (ret < 0) {
		pr_err_info("setsockopt");
		goto out_close_sockfd;
	}

	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xFFFFFFFF;
	addr.nl_pad = 0;

	ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		pr_err_info("bind");
		goto out_close_sockfd;
	}

	return sockfd;

out_close_sockfd:
	close(sockfd);
	return ret;
}

// ============================================================

static void network_client_close_dummy(struct network_client *client)
{
	close(client->sockfd);
}

static void network_client_tcp_close(struct network_client *client)
{
	inet_close_tcp_socket(client->sockfd);
}

static int network_client_flush_dummy(struct network_client *client)
{
	return fsync(client->sockfd);
}

static ssize_t network_client_send_dummy(struct network_client *client, const void *buff, size_t size)
{
	return inet_send(client->sockfd, buff, size);
}

static ssize_t network_client_recv_dummy(struct network_client *client, void *buff, size_t size)
{
	return inet_recv(client->sockfd, buff, size);
}

static ssize_t network_client_sendto_dummy(struct network_client *client, const void *buff, size_t size, const struct sockaddr *addr)
{
	return sendto(client->sockfd, buff, size, 0, addr, client->addrlen);
}

static ssize_t network_client_recvfrom_dummy(struct network_client *client, void *buff, size_t size, struct sockaddr *addr)
{
	return recvfrom(client->sockfd, buff, size, 0, addr, &client->addrlen);
}

static int network_protocol_open_client(const struct network_protocol_desc *desc, struct network_client *client, const struct network_url *url, int flags)
{
	client->type = desc->type;
	client->flush = network_client_flush_dummy;
	client->close = network_client_close_dummy;
	client->send = network_client_send_dummy;
	client->recv = network_client_recv_dummy;
	client->sendto = network_client_sendto_dummy;
	client->recvfrom = network_client_recvfrom_dummy;

	return desc->open_client(client, url, network_get_port_by_url(url, desc), flags);
}

static int network_service_open_dummy(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	pr_red_info("No implement");

	return -EINVAL;
}

static ssize_t network_service_sendto_dummy(struct network_service *service, const void *buff, size_t size, const struct sockaddr *addr)
{
	return sendto(service->sockfd, buff, size, 0, addr, service->addrlen);
}

static ssize_t network_service_recvfrom_dummy(struct network_service *service, void *buff, size_t size, struct sockaddr *addr)
{
	return recvfrom(service->sockfd, buff, size, 0, addr, &service->addrlen);
}

static int network_service_accept_dummy(struct network_service *service, struct network_client *conn)
{
	struct sockaddr *addr;

	addr = alloca(service->addrlen);
	if (addr == NULL) {
		pr_error_info("alloca");
		return -ENOMEM;
	}

	conn->addrlen = service->addrlen;
	conn->sockfd = accept(service->sockfd, addr, &conn->addrlen);
	if (conn->sockfd < 0) {
		pr_error_info("accept");
		return conn->sockfd;
	}

	pd_info("%s", network_sockaddr_tostring(addr, NULL, 0));

	conn->close = network_client_tcp_close;
	conn->send = network_client_send_dummy;
	conn->recv = network_client_recv_dummy;
	conn->flush = network_client_flush_dummy;
	conn->sendto = network_client_sendto_dummy;
	conn->recvfrom = network_client_recvfrom_dummy;

	return 0;
}

static void network_service_close_dummy(struct network_service *service)
{
	close(service->sockfd);
}

static void network_service_tcp_close(struct network_service *service)
{
	inet_close_tcp_socket(service->sockfd);
}

static int network_protocol_open_service(const struct network_protocol_desc *desc, struct network_service *service, const struct network_url *url, int flags)
{
	service->type = desc->type;
	service->sendto = network_service_sendto_dummy;
	service->recvfrom = network_service_recvfrom_dummy;
	service->accept = network_service_accept_dummy;
	service->close = network_service_close_dummy;

	return desc->open_service(service, url, network_get_port_by_url(url, desc), flags);
}

// ============================================================

static ssize_t network_client_send_sync(struct network_client *client, const void *buff, size_t size)
{
	ssize_t length;
	const void *buff_end = ADDR_ADD(buff, size);
	struct network_client_sync_data *data = network_client_get_data(client);
	struct pollfd pfd = {
		.fd = client->sockfd,
		.events = POLLIN,
	};

	while (1) {
		pthread_mutex_lock(&data->lock);

		if (data->send_pending == 0) {
			break;
		}

		pthread_mutex_unlock(&data->lock);

		usleep(100);
	}

	data->send_pending++;

	while (buff < buff_end) {
		struct cavan_sync_package *package;
		size_t datalen = ADDR_SUB2(buff_end, buff);

		package = alloca(datalen + sizeof(struct cavan_sync_package));
		if (package == NULL) {
			pr_error_info("alloca");
			length = -ENOMEM;
			goto out_clean_pending;
		}

		while (1) {
			int retry;
			ssize_t rdlen, wrlen;

			package->index = data->send_index;
			package->type = CAVAN_SYNC_TYPE_DATA;
			package->length = datalen;
			mem_copy(package->data, buff, datalen);

			for (retry = CAVAN_NET_UDP_RETRY; ; retry--) {
				int ret;

				wrlen = data->send(client, package, datalen + sizeof(struct cavan_sync_package));
				if (wrlen < (ssize_t) sizeof(struct cavan_sync_package)) {
					pr_red_info("data->send");
					length = -EFAULT;
					goto out_clean_pending;
				}

				ret = poll(&pfd, 1, CAVAN_NET_UDP_TIMEOUT);
				if (ret > 0) {
					break;
				}

				pr_red_info("retry = %d", retry);

				if (ret < 0 || retry < 1) {
					length = -ETIMEDOUT;
					goto out_clean_pending;
				}
			}

label_recv_ack:
			rdlen = data->recv(client, package, sizeof(struct cavan_sync_package));
			if (rdlen != sizeof(struct cavan_sync_package)) {
				length = -EFAULT;
				goto out_clean_pending;
			}

			if (package->type == CAVAN_SYNC_TYPE_ACK) {
				if (package->index == data->send_index) {
					data->send_index++;
					break;
				}

				if (poll(&pfd, 1, CAVAN_NET_UDP_TIMEOUT) > 0) {
#if CAVAN_NETWORK_DEBUG
					pr_red_info("retry receive ack");
#endif
					goto label_recv_ack;
				}
			} else if (package->type == CAVAN_SYNC_TYPE_DATA) {
				u8 send_index = data->send_index;

				pthread_mutex_unlock(&data->lock);
				usleep(1);
				pthread_mutex_lock(&data->lock);

				if (send_index != data->send_index) {
#if CAVAN_NETWORK_DEBUG
					pr_green_info("send_index = %d, index = %d", data->send_index, send_index); msleep(500);
#endif
					break;
				}
			}

#if CAVAN_NETWORK_DEBUG
			pd_red_info("index not match, send_index = %d, index = %d, type = %d, length = %d",
				data->send_index, package->index, package->type, package->length);
#endif
		}

		buff = ADDR_ADD(buff, datalen);
	}

	length = size;

out_clean_pending:
	data->send_pending--;
	pthread_mutex_unlock(&data->lock);
	return length;
}

static ssize_t network_client_recv_sync(struct network_client *client, void *buff, size_t size)
{
	ssize_t length;
	struct cavan_sync_package *package;
	struct network_client_sync_data *data = network_client_get_data(client);
	struct pollfd pfd = {
		.fd = client->sockfd,
		.events = POLLIN,
	};

	package = alloca(size + sizeof(struct cavan_sync_package));
	if (package == NULL) {
		pr_error_info("alloca");
		return -ENOMEM;
	}

	pthread_mutex_lock(&data->lock);

	data->recv_pending++;

	while (1) {
		int ret;
		ssize_t rdlen, wrlen;

		pthread_mutex_unlock(&data->lock);
		ret = poll(&pfd, 1, CAVAN_NET_UDP_ACTIVE_TIME);
		pthread_mutex_lock(&data->lock);
		if (ret < 1) {
			pr_red_info("file_poll_input");
			length = -ETIMEDOUT;
			goto out_clean_pending;
		}

		if (poll(&pfd, 1, 0) < 1) {
			continue;
		}

		rdlen = data->recv(client, package, size + sizeof(struct cavan_sync_package));
		if (rdlen < (ssize_t) sizeof(struct cavan_sync_package)) {
			length = -EFAULT;
			goto out_clean_pending;
		}

		length = package->length;

		if (package->type == CAVAN_SYNC_TYPE_DATA) {
			if ((size_t) rdlen == length + sizeof(struct cavan_sync_package)) {
				package->length = 0;
				package->type = CAVAN_SYNC_TYPE_ACK;

				wrlen = data->send(client, package, sizeof(struct cavan_sync_package));
				if (wrlen != sizeof(struct cavan_sync_package)) {
					pr_red_info("data->send");
					length = -EFAULT;
					goto out_clean_pending;
				}

				if (package->index == data->recv_index) {
					break;
				}
			}
		} else if (package->type == CAVAN_SYNC_TYPE_ACK) {
			if (package->index == data->send_index) {
				data->send_index++;
			}
		}

#if CAVAN_NETWORK_DEBUG
		pd_red_info("index not match, recv_index = %d, index = %d, type = %d, length = %d",
			data->recv_index, package->index, package->type, package->length);
#endif
	}

	data->recv_index++;
	mem_copy(buff, package->data, length);

out_clean_pending:
	data->recv_pending--;
	pthread_mutex_unlock(&data->lock);
	return length;
}

static void network_client_close_sync(struct network_client *client)
{
	int i;
	struct network_client_sync_data *data = network_client_get_data(client);

	if (data == NULL) {
		return;
	}

	client->send = data->send;
	client->recv = data->recv;
	client->close = data->close;

	for (i = 0; i < 10; i++) {
		client->send(client, "E", 1);
		fsync(client->sockfd);
	}

	client->close(client);

	for (i = 0; i < 100; i++) {
		bool ready;

		usleep(100);

		if (pthread_mutex_trylock(&data->lock) < 0) {
			continue;
		}

		ready = data->send_pending == 0 && data->recv_pending == 0;

		pthread_mutex_unlock(&data->lock);

		if (ready) {
			break;
		}

#if CAVAN_NETWORK_DEBUG
		pr_red_info("%d. data->send_pending = %d, data->recv_pending = %d", i, data->send_pending, data->recv_pending);
#endif
	}

	pthread_mutex_destroy(&data->lock);

	free(data);
	network_client_set_data(client, NULL);
}

static int network_client_set_sync(struct network_client *client)
{
	int ret;
	struct network_client_sync_data *data;

	data = malloc(sizeof(*data));
	if (data == NULL) {
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = pthread_mutex_init(&data->lock, NULL);
	if (ret < 0) {
		pr_error_info("pthread_mutex_init");
		free(data);
		return ret;
	}

	data->send_index = 0;
	data->recv_index = 0;
	data->send_pending = 0;
	data->recv_pending = 0;

	data->send = client->send;
	data->recv = client->recv;
	data->close = client->close;

	client->send = network_client_send_sync;
	client->recv = network_client_recv_sync;
	client->close = network_client_close_sync;

	network_client_set_data(client, data);

	return 0;
}

#if 0
static void network_client_udp_close(struct network_client *client)
{
	close(client->sockfd);
}

static ssize_t network_client_udp_send(struct network_client *client, const void *buff, size_t size)
{
	struct network_client_inet *inet = (struct network_client_inet *) client;

	return sendto(client->sockfd, buff, size, 0, (struct sockaddr *) &inet->addr, client->addrlen);
}

static ssize_t network_client_udp_recv(struct network_client *client, void *buff, size_t size)
{
	struct network_client_inet *inet = (struct network_client_inet *) client;

	return recvfrom(client->sockfd, buff, size, 0, (struct sockaddr *) &inet->addr, &client->addrlen);
}
#endif

static int network_client_udp_talk(struct network_client *client, struct sockaddr *addr)
{
	int ret;
	u32 magic;

	magic = CAVAN_NETWORK_MAGIC;
	ret = sendto(client->sockfd, &magic, sizeof(magic), 0, addr, client->addrlen);
	if (ret < (ssize_t) sizeof(magic)) {
		pr_error_info("sendto");
		return -EFAULT;
	}

	if (file_poll_input(client->sockfd, CAVAN_NET_UDP_TIMEOUT) == false) {
		pr_red_info("file_poll_input");
		return -EFAULT;
	}

	ret = recvfrom(client->sockfd, &magic, sizeof(magic), 0, addr, &client->addrlen);
	if (ret < (ssize_t) sizeof(magic)) {
		pr_red_info("recvfrom");
		return -EFAULT;
	}

	if (magic != CAVAN_NETWORK_MAGIC) {
		pr_red_info("invalid magic = 0x%08x", magic);
		return -EINVAL;
	}

	return 0;
}

static int network_client_udp_common_open(struct network_client *client, struct sockaddr *addr, int flags)
{
	int ret;

	if (addr && (flags & CAVAN_NET_FLAG_TALK)) {
		ret = network_client_udp_talk(client, addr);
		if (ret < 0) {
			pr_red_info("network_client_udp_talk");
			return ret;
		}
	}

	ret = connect(client->sockfd, addr, client->addrlen);
	if (ret < 0) {
		pr_error_info("connect");
		return ret;
	}

	client->close = network_client_tcp_close;

	if (flags & CAVAN_NET_FLAG_SYNC) {
		ret = network_client_set_sync(client);
		if (ret < 0) {
			pr_red_info("network_client_set_sync");
			return ret;
		}
	}

	return 0;
}

static int network_client_udp_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int ret;
	int sockfd;
	struct sockaddr_in addr;

	ret = inet_hostname2sockaddr(url->hostname, &addr);
	if (ret < 0) {
		pr_red_info("inet_hostname2sockaddr");
		return ret;
	}

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_error_info("inet_socket");
		return sockfd;
	}

	if (inet_addr_is_broadcast(&addr)) {
		int broadcast = 1;

		ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
		if (ret < 0) {
			pr_red_info("setsockopt SO_BROADCAST");
			goto out_close_sockfd;
		}

		pr_bold_info("set socket type to broadcast successfully");
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);

	addr.sin_port = htons(port);

	ret = network_client_udp_common_open(client, (struct sockaddr *) &addr, flags);
	if (ret < 0) {
		pr_red_info("network_client_udp_common_open");
		goto out_close_sockfd;
	}

	return 0;

out_close_sockfd:
	close(sockfd);
	return ret;
}

static int network_client_tcp_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = network_create_link(url->hostname, port, SOCK_STREAM, 0);
	if (sockfd < 0) {
#if CAVAN_NETWORK_DEBUG
		pr_err_info("inet_socket");
#endif
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);
	client->close = network_client_tcp_close;

	return 0;
}

static int network_create_unix_udp_client(struct network_client *client)
{
	int sockfd;

	sockfd = unix_create_service(SOCK_DGRAM, NULL);
	if (sockfd < 0) {
		pr_red_info("unix_create_service");
		return sockfd;
	}

	client->sockfd = sockfd;

	return 0;
}

static int network_client_unix_tcp_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = unix_create_tcp_link(url->pathname, 0);
	if (sockfd < 0) {
		pr_error_info("unix_socket %s", url->pathname);
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_un);
	client->close = network_client_tcp_close;

	return 0;
}

static int network_client_unix_udp_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int ret;
	struct sockaddr_un addr;

	ret = network_create_unix_udp_client(client);
	if (ret < 0) {
		pr_error_info("inet_socket");
		return ret;
	}

	unix_sockaddr_init(&addr, url->pathname);
	client->addrlen = sizeof(struct sockaddr_un);

	ret = network_client_udp_common_open(client, (struct sockaddr *) &addr, flags);
	if (ret < 0) {
		pr_red_info("network_client_udp_common_open");
		goto out_close_sockfd;
	}

	return 0;

out_close_sockfd:
	close(client->sockfd);
	return ret;
}

static int network_client_adb_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = adb_create_tcp_link(url->hostname, 0, port, (flags & CAVAN_NET_FLAG_WAIT) != 0);
	if (sockfd < 0) {
#if CAVAN_NETWORK_DEBUG
		pr_red_info("adb_create_tcp_link");
#endif
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);
	client->close = network_client_tcp_close;

	return 0;
}

static int network_client_icmp_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int ret;
	int sockfd;

	sockfd = network_create_link(url->hostname, port, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		pr_error_info("inet_socket");
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);

	ret = network_client_udp_common_open(client, NULL, 0);
	if (ret < 0) {
		pr_red_info("network_client_udp_common_open");
		goto out_close_sockfd;
	}

	return 0;

out_close_sockfd:
	close(sockfd);
	return ret;
}

static int network_client_ip_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int ret;
	int sockfd;

	sockfd = network_create_link(url->hostname, port, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		pr_error_info("inet_socket");
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);

	ret = network_client_udp_common_open(client, NULL, 0);
	if (ret < 0) {
		pr_red_info("network_client_udp_common_open");
		goto out_close_sockfd;
	}

	return 0;

out_close_sockfd:
	close(sockfd);
	return ret;
}

static int network_client_mac_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = network_create_socket_mac(url->hostname, 0);
	if (sockfd < 0) {
		pr_red_info("inet_socket");
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_in);

	return 0;
}

static int network_client_uevent_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = network_create_socket_uevent();
	if (sockfd < 0) {
		pr_red_info("network_create_socket_uevent");
		return sockfd;
	}

	client->sockfd = sockfd;
	client->addrlen = sizeof(struct sockaddr_nl);

	return 0;
}

static int network_file_get_open_flags(network_protocol_t type)
{
	switch (type) {
	case NETWORK_PROTOCOL_FILE_RO:
		return O_RDONLY;

	case NETWORK_PROTOCOL_FILE_WO:
		return O_WRONLY;

	default:
		return O_RDWR;
	}
}

static void network_client_file_close(struct network_client *client)
{
	if (isatty(client->sockfd)) {
		cavan_tty_attr_restore(client->sockfd, NULL);
	}

	if (client->service) {
		sem_post(&client->service->sem);
	} else {
		fsync(client->sockfd);
		close(client->sockfd);
	}
}

static ssize_t network_client_file_send(struct network_client *client, const void *buff, size_t size)
{
	return ffile_write(client->sockfd, buff, size);
}

static ssize_t network_client_file_recv(struct network_client *client, void *buff, size_t size)
{
	return ffile_read(client->sockfd, buff, size);
}

static int network_file_open(const char *pathname, int type)
{
	int fd;
	int flags = network_file_get_open_flags(type);

	fd = open(pathname, flags | O_CLOEXEC | O_NOCTTY);
	if (fd < 0) {
		pr_red_info("open file %s", pathname);
		return fd;
	}

	if (isatty(fd)) {
		cavan_tty_set_mode(fd, CAVAN_TTY_MODE_DATA, NULL);
	}

	return fd;
}

static int network_client_file_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int sockfd;

	sockfd = network_file_open(url->pathname, client->type);
	if (sockfd < 0) {
		return sockfd;
	}

	client->service = NULL;
	client->sockfd = sockfd;
	client->addrlen = 0;
	client->close = network_client_file_close;
	client->send = network_client_file_send;
	client->recv = network_client_file_recv;

	return 0;
}

static int network_service_file_accept(struct network_service *service, struct network_client *client)
{
	sem_wait(&service->sem);

	client->service = service;
	client->sockfd = service->sockfd;
	client->close = network_client_file_close;
	client->send = network_client_file_send;
	client->recv = network_client_file_recv;

	return 0;
}

static void network_service_file_close(struct network_service *service)
{
	fsync(service->sockfd);
	close(service->sockfd);
	sem_destroy(&service->sem);
}

static int network_service_file_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	int ret;
	int sockfd;

	ret = sem_init(&service->sem, 0, 1);
	if (ret < 0) {
		pr_err_info("sem_init");
		return ret;
	}

	sockfd = network_file_open(url->pathname, service->type);
	if (sockfd < 0) {
		ret = sockfd;
		goto out_sem_destroy;
	}

	service->sockfd = sockfd;
	service->addrlen = 0;
	service->accept = network_service_file_accept;
	service->close = network_service_file_close;

	return 0;

out_sem_destroy:
	sem_destroy(&service->sem);
	return ret;
}

// ================================================================================

ssize_t network_client_fill_buff(struct network_client *client, char *buff, size_t size)
{
	ssize_t rdlen;
	const char *buff_end = buff + size;

	for (buff_end = buff + size; buff < buff_end; buff += rdlen) {
		rdlen = client->recv(client, buff, buff_end - buff);
		if (rdlen <= 0) {
			return -EFAULT;
		}
	}

	return size;
}

int network_client_recv_file(struct network_client *client, int fd, size64_t skip, size64_t size)
{
	off_t pos;
	ssize_t rdlen;
	char buff[2048];
	struct progress_bar bar;

	if (skip && (pos = lseek(fd, skip, SEEK_CUR)) < 0) {
		pr_err_info("lseek");
		return pos;
	}

	progress_bar_init(&bar, size, skip, PROGRESS_BAR_TYPE_DATA);

	if (size == 0) {
		while (1) {
			rdlen = client->recv(client, buff, sizeof(buff));
			if (rdlen <= 0) {
				if (rdlen < 0) {
					return rdlen;
				}

				break;
			}

			if (ffile_write(fd, buff, rdlen) < rdlen) {
				pr_err_info("ffile_write");
				return -EIO;
			}

			progress_bar_add(&bar, rdlen);
		}
	} else {
		while (size) {
#if 1
			rdlen = client->recv(client, buff, size < sizeof(buff) ? size : sizeof(buff));
#else
			rdlen = client->recv(client, buff, sizeof(buff));
#endif
			if (rdlen <= 0) {
				pr_err_info("client->recv");
				return rdlen < 0 ? rdlen : -EFAULT;
			}

			if (ffile_write(fd, buff, rdlen) < rdlen) {
				pr_err_info("ffile_write");
				return -EIO;
			}

			size -= rdlen;
			progress_bar_add(&bar, rdlen);
		}
	}

	progress_bar_finish(&bar);

	return 0;
}

int network_client_recv_file2(struct network_client *client, const char *pathname, size64_t size, int flags)
{
	int fd;
	int ret;

	fd = open(pathname, flags | O_WRONLY | O_CREAT, 0777);
	if (fd < 0) {
		pr_error_info("open `%s'", pathname);
		return fd;
	}

	ret = network_client_recv_file(client, fd, 0, size);
	if (ret < 0) {
		pr_red_info("network_client_recv_file");
	}

	close(fd);

	return ret;
}

int network_client_send_file(struct network_client *client, int fd, size64_t skip, size64_t size)
{
	off_t pos;
	ssize_t rdlen;
	char buff[2048];
	struct progress_bar bar;

	if (skip && (pos = lseek(fd, skip, SEEK_CUR)) < 0) {
		pr_err_info("lseek");
		return pos;
	}

	progress_bar_init(&bar, size, skip, PROGRESS_BAR_TYPE_DATA);

	if (size == 0) {
		bool no_data = true;

		while (1) {
			rdlen = ffile_read(fd, buff, sizeof(buff));
			if (rdlen <= 0) {
				if (rdlen < 0) {
					return rdlen;
				}

				break;
			}

			no_data = false;

			if (client->send(client, buff, rdlen) < rdlen) {
				return -EIO;
			}

			progress_bar_add(&bar, rdlen);
		}

		if (no_data) {
			pr_warn_info("No data send!");
		}
	} else {
		while (size) {
#if 1
			rdlen = ffile_read(fd, buff, size < sizeof(buff) ? size : sizeof(buff));
#else
			rdlen = ffile_read(fd, buff, sizeof(buff));
#endif
			if (rdlen <= 0 || client->send(client, buff, rdlen) < rdlen) {
				return -EFAULT;
			}

			size -= rdlen;
			progress_bar_add(&bar, rdlen);
		}
	}

	client->flush(client);

	progress_bar_finish(&bar);

	return 0;
}

int network_client_send_file2(struct network_client *client, const char *pathname, size64_t size)
{
	int fd;
	int ret;

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pr_error_info("open `%s'", pathname);
		return fd;
	}

	ret = network_client_send_file(client, fd, 0, size);
	if (ret < 0) {
		pr_red_info("network_client_send_file");
	}

	close(fd);

	return ret;
}

ssize_t network_client_timed_recv(struct network_client *client, void *buff, size_t size, int timeout)
{
	if (file_poll_input(client->sockfd, timeout)) {
		return client->recv(client, buff, size);
	}

	return -ETIMEDOUT;
}

char *network_client_recv_line(struct network_client *client, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size; buff < buff_end; buff++) {
		ssize_t rdlen;

		rdlen = client->recv(client, buff, 1);
		if (rdlen <= 0) {
#if 0
			if (rdlen < 0) {
				return NULL;
			}

			break;
#else
			return NULL;
#endif
		}

		if (*buff == '\n') {
			break;
		}
	}

	return buff;
}

bool network_client_discard_all(struct network_client *client)
{
	int ret;
	ssize_t rdlen;
	char buff[1024];
	struct pollfd pfd = {
		.fd = client->sockfd,
		.events = POLLIN,
	};

	while (1) {
		ret = poll(&pfd, 1, 0);
		if (ret < 0) {
			pr_error_info("poll");
			return false;
		}

		if (ret < 1) {
			break;
		}

		rdlen = client->recv(client, buff, sizeof(buff));
		if (rdlen < 0) {
			pr_error_info("recv");
			return false;
		}

		if (rdlen < (ssize_t) sizeof(buff)) {
			break;
		}
	}

	return true;
}

int network_client_vprintf(struct network_client *client, const char *format, va_list ap)
{
	int length;
	char buff[2048];

	length = vsnprintf(buff, sizeof(buff), format, ap);

#if CAVAN_NETWORK_DEBUG
	print_ntext(buff, length);
#endif

	return client->send(client, buff, length);
}

int network_client_printf(struct network_client *client, const char *format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	ret = network_client_vprintf(client, format, ap);
	va_end(ap);

	return ret;
}

#if CAVAN_NETWORK_TRANSMIT_THREAD
static void *network_client_transmit_handler(void *_data)
{
	struct network_transmit_data *data = _data;
	struct network_client *client = data->receiver;
	int fd = data->sender_fd;

	while (1) {
		ssize_t rdlen;
		char buff[2014];

		rdlen = client->recv(client, buff, sizeof(buff));
		if (rdlen <= 0 || write(fd, buff, rdlen) < rdlen) {
			break;
		}

		fsync(fd);
	}

	return NULL;
}
#endif

int network_client_exec_redirect(struct network_client *client, int ttyin, int ttyout)
{
#if CAVAN_NETWORK_TRANSMIT_THREAD
	pthread_t thread;
	struct network_transmit_data data;

	data.receiver = client;
	data.sender_fd = ttyout;

	cavan_pthread_create(&thread, network_client_transmit_handler, &data, false);

	while (1) {
		ssize_t rdlen;
		char buff[1024];

		rdlen = ffile_read(ttyin, buff, sizeof(buff));
		if (rdlen <= 0 || client->send(client, buff, rdlen) < rdlen) {
			break;
		}

		fsync(client->sockfd);
	}

	cavan_pthread_kill(thread);
#else
	struct pollfd pfds[2];

	pfds[0].events = POLLIN;
	pfds[0].fd = client->sockfd;

	pfds[1].events = POLLIN;
	pfds[1].fd = ttyin;

	while (1) {
		int ret;
		ssize_t rdlen;
		char buff[1024];

		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0) {
			return -ETIMEDOUT;
		}

		if (pfds[0].revents) {
			rdlen = client->recv(client, buff, sizeof(buff));
			if (rdlen <= 0 || write(ttyout, buff, rdlen) < rdlen) {
				break;
			}

			fsync(ttyout);
		}

		if (pfds[1].revents) {
			rdlen = ffile_read(ttyin, buff, sizeof(buff));
			if (rdlen <= 0 || client->send(client, buff, rdlen) < rdlen) {
				break;
			}

			fsync(client->sockfd);
		}
	}
#endif

	return 0;
}

int network_client_exec_main(struct network_client *client, const char *command, int lines, int columns)
{
	int ret;
	int ttyfd;
	pid_t pid;

	ttyfd = cavan_exec_redirect_stdio_popen(command, lines, columns, &pid, 0x07);
	if (ttyfd < 0) {
		pr_red_info("cavan_exec_redirect_stdio_popen");
		return ttyfd;
	}

	ret = network_client_exec_redirect(client, ttyfd, ttyfd);
	if (ret < 0) {
		pr_red_info("tcp_dd_exec_redirect_loop");
		goto out_close_ttyfd;
	}

	ret = cavan_exec_waitpid(pid);

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

// ============================================================

static int network_service_udp_talk(struct network_service *service, struct network_client *client)
{
	int ret;
	u32 magic;
	struct sockaddr *addr;

	addr = alloca(service->addrlen);
	if (addr == NULL) {
		pr_error_info("alloca");
		return -ENOMEM;
	}

	client->addrlen = service->addrlen;
	ret = recvfrom(service->sockfd, &magic, sizeof(magic), 0, addr, &client->addrlen);
	if (ret < (ssize_t) sizeof(magic)) {
		pr_error_info("recvfrom");
		return -EFAULT;
	}

	pd_info("%s", network_sockaddr_tostring(addr, NULL, 0));
	pd_bold_info("magic = 0x%08x, type = %d", magic, service->type);

	if (magic != CAVAN_NETWORK_MAGIC) {
		pr_red_info("invalid magic = 0x%08x", magic);
		return -EINVAL;
	}

	if (service->type == NETWORK_PROTOCOL_UNIX_UDP) {
		int ret;

		ret = network_create_unix_udp_client(client);
		if (ret < 0) {
			pr_red_info("network_create_unix_udp_client");
			return ret;
		}
	} else {
		client->sockfd = inet_socket(SOCK_DGRAM);
		if (client->sockfd < 0) {
			pr_error_info("inet_socket");
			return client->sockfd;
		}
	}

	ret = sendto(client->sockfd, &magic, sizeof(magic), 0, addr, client->addrlen);
	if (ret < (ssize_t) sizeof(magic)) {
		pr_error_info("network_client_send_message");

		if (ret >= 0) {
			ret = -EFAULT;
		}

		goto out_client_close;
	}

	ret = connect(client->sockfd, addr, client->addrlen);
	if (ret < 0) {
		pr_error_info("connect");
		goto out_client_close;
	}

	client->close = network_client_tcp_close;

	return 0;

out_client_close:
	client->close(client);
	return ret;
}

static int network_service_udp_accept(struct network_service *service, struct network_client *client)
{
	int ret;

	ret = network_service_udp_talk(service, client);
	if (ret < 0) {
		pr_red_info("network_service_udp_talk");
		return ret;
	}

#if 0
	ret = network_client_set_sync(client);
	if (ret < 0) {
		pr_red_info("network_client_set_sync");
		return ret;
	}
#endif

	return 0;
}

static int network_service_udp_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	service->sockfd = inet_create_udp_service(port);
	if (service->sockfd < 0) {
		pr_red_info("inet_create_udp_service");
		return service->sockfd;
	}

	service->addrlen = sizeof(struct sockaddr_in);
	service->accept = network_service_udp_accept;

	return 0;
}

static int network_service_tcp_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	service->sockfd = inet_create_tcp_service(port);
	if (service->sockfd < 0) {
		pr_red_info("inet_create_tcp_service");
		return service->sockfd;
	}

	service->addrlen = sizeof(struct sockaddr_in);
	service->close = network_service_tcp_close;

	return 0;
}

static int network_service_unix_tcp_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	service->sockfd = unix_create_tcp_service(url->pathname);
	if (service->sockfd < 0) {
		pr_red_info("inet_create_tcp_service");
		return service->sockfd;
	}

	service->addrlen = sizeof(struct sockaddr_un);
	service->close = network_service_tcp_close;

	return 0;
}

static int network_service_unix_udp_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	service->sockfd = unix_create_udp_service(url->pathname);
	if (service->sockfd < 0) {
		pr_red_info("inet_create_tcp_service");
		return service->sockfd;
	}

	service->addrlen = sizeof(struct sockaddr_un);
	service->accept = network_service_udp_accept;

	return 0;
}

#if CONFIG_CAVAN_SSL
static char *network_ssl_err_str(const SSL *ssl, int code, char *buff)
{
	return ERR_error_string(SSL_get_error(ssl, code), buff);
}

static SSL_CTX *network_ssl_context_new(const SSL_METHOD *method, const char *cert_file, const char *key_file, const char *password)
{
	SSL_CTX *ctx;

	println("cert_file = %s", cert_file);
	println("key_file = %s", key_file);
	println("password = %s", password);

	ctx = SSL_CTX_new(method);
	if (ctx == NULL) {
		return NULL;
	}

	if (cert_file) {
		if (1 != SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM)) {
			pr_red_info("SSL_CTX_use_certificate_file");
			goto out_SSL_CTX_free;
		}
	}

	if (password) {
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) password);
	}

	if (key_file) {
		if (1 != SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM)) {
			pr_red_info("SSL_CTX_use_PrivateKey_file");
			goto out_SSL_CTX_free;
		}

		if (1 != SSL_CTX_check_private_key(ctx)) {
			pr_red_info("SSL_CTX_check_private_key");
			goto out_SSL_CTX_free;
		}
	}

	return ctx;

out_SSL_CTX_free:
	SSL_CTX_free(ctx);
	return NULL;
}

static SSL_CTX *network_ssl_context_get(boolean server)
{
	static boolean initialized;
	static SSL_CTX *context_client;
	static SSL_CTX *context_server;

	if (!initialized) {
		SSL_load_error_strings();
		SSL_library_init();
		initialized = true;
	}

	if (server) {
		if (context_server == NULL) {
			const char *cert, *key, *password;

			cert = getenv("CAVAN_SSL_CERT");
			key = getenv("CAVAN_SSL_KEY");
			password = getenv("CAVAN_SSL_PASSWORD");

#ifdef CONFIG_CAVAN_SSL_CERT
			if (cert == NULL) {
				cert = CONFIG_CAVAN_SSL_CERT;
			}
#endif

#ifdef CONFIG_CAVAN_SSL_KEY
			if (key == NULL) {
				key = CONFIG_CAVAN_SSL_KEY;
			}
#endif

#ifdef CONFIG_CAVAN_SSL_PASSWORD
			if (password == NULL) {
				password = CONFIG_CAVAN_SSL_PASSWORD;
			}
#endif

			context_server = network_ssl_context_new(SSLv23_server_method(), cert, key, password);
		}

		return context_server;
	}

	if (context_client == NULL) {
		context_client = network_ssl_context_new(SSLv23_client_method(), NULL, NULL, NULL);
	}

	return context_client;
}

static SSL *network_ssl_new(boolean server)
{
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	SSL_CTX *ctx;
	SSL *ssl;

	pthread_mutex_lock(&lock);

	ctx = network_ssl_context_get(server);
	if (ctx == NULL) {
		pr_red_info("network_ssl_context_get");
		ssl = NULL;
	} else {
		ssl = SSL_new(ctx);
	}

	pthread_mutex_unlock(&lock);

	return ssl;
}

static SSL *network_ssl_open(int fd, boolean server)
{
	int ret;
	SSL *ssl;

	ssl = network_ssl_new(server);
	if (ssl == NULL) {
		pr_red_info("SSL_new");
		return NULL;
	}

	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	ret = SSL_set_fd(ssl, fd);
	if (ret != 1) {
		pr_red_info("SSL_set_fd: %s", network_ssl_err_str(ssl, ret, NULL));
		goto out_SSL_free;
	}

	if (server) {
		ret = SSL_accept(ssl);
		if (ret != 1) {
			pr_red_info("SSL_accept: %s", network_ssl_err_str(ssl, ret, NULL));
			goto out_SSL_free;
		}
	} else {
		ret = SSL_connect(ssl);
		if (ret != 1) {
			pr_red_info("SSL_connect: %s", network_ssl_err_str(ssl, ret, NULL));
			goto out_SSL_free;
		}
	}

	return ssl;

out_SSL_free:
	SSL_free(ssl);
	return NULL;
}

static void network_client_ssl_close(struct network_client *client)
{
	if (client->context) {
		SSL_free(client->context); // or SSL_shutdown(ssl);
		client->context = NULL;
	}

	network_client_tcp_close(client);
}

static ssize_t network_client_ssl_send(struct network_client *client, const void *buff, size_t size)
{
	if (unlikely(client->context == NULL)) {
		return -EINVAL;
	}

	return SSL_write(client->context, buff, size);
}

static ssize_t network_client_ssl_recv(struct network_client *client, void *buff, size_t size)
{
	if (unlikely(client->context == NULL)) {
		return -EINVAL;
	}

	return SSL_read(client->context, buff, size);
}

int network_client_ssl_attach(struct network_client *client, boolean server)
{
	SSL *ssl;

	ssl = network_ssl_open(client->sockfd, server);
	if (ssl == NULL) {
		pr_red_info("network_ssl_open");
		return -EFAULT;
	}

	client->context = ssl;
	client->close = network_client_ssl_close;
	client->send = network_client_ssl_send;
	client->recv = network_client_ssl_recv;

	return 0;
}

static int network_client_ssl_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	int ret;

	ret = network_client_tcp_open(client, url, port, flags);
	if (ret < 0) {
		pr_red_info("network_client_tcp_open: %d", ret);
		return ret;
	}

	ret = network_client_ssl_attach(client, false);
	if (ret < 0) {
		pr_red_info("network_client_ssl_attach: %d", ret);
		goto out_client_close;
	}

	return 0;

out_client_close:
	client->close(client);
	return -EFAULT;
}

static int network_service_ssl_accept(struct network_service *service, struct network_client *conn)
{
	int ret;

	ret = network_service_accept_dummy(service, conn);
	if (ret < 0) {
		pr_red_info("network_service_accept_dummy");
		return ret;
	}

	ret = network_client_ssl_attach(conn, true);
	if (ret < 0) {
		pr_red_info("network_client_ssl_attach: %d", ret);
		goto out_conn_close;
	}

	return 0;

out_conn_close:
	conn->close(conn);
	return -EFAULT;
}

static int network_service_ssl_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	int ret;

	ret = network_service_tcp_open(service, url, port, flags);
	if (ret < 0) {
		pr_red_info("network_service_tcp_open: %d", ret);
		return ret;
	}

	service->accept = network_service_ssl_accept;

	return 0;
}
#else
static int network_client_ssl_open(struct network_client *client, const struct network_url *url, u16 port, int flags)
{
	pr_red_info("Need enable use CONFIG_CAVAN_SSL");
	return -EINVAL;
}

static int network_service_ssl_open(struct network_service *service, const struct network_url *url, u16 port, int flags)
{
	pr_red_info("Need enable use CONFIG_CAVAN_SSL");
	return -EINVAL;
}
#endif

// ============================================================

static const struct network_protocol_desc protocol_descs[] = {
	[NETWORK_PROTOCOL_FTP] = {
		.name = "ftp",
		.port = NETWORK_PORT_FTP,
		.type = NETWORK_PROTOCOL_FTP,
		.open_service = network_service_tcp_open,
		.open_client = network_client_tcp_open,
	},
	[NETWORK_PROTOCOL_HTTP] = {
		.name = "http",
		.port = NETWORK_PORT_HTTP,
		.type = NETWORK_PROTOCOL_HTTP,
		.open_service = network_service_tcp_open,
		.open_client = network_client_tcp_open,
	},
	[NETWORK_PROTOCOL_HTTPS] = {
		.name = "https",
		.port = NETWORK_PORT_HTTPS,
		.type = NETWORK_PROTOCOL_HTTPS,
		.open_service = network_service_ssl_open,
		.open_client = network_client_ssl_open,
	},
	[NETWORK_PROTOCOL_SSL] = {
		.name = "ssl",
		.type = NETWORK_PROTOCOL_SSL,
		.open_service = network_service_ssl_open,
		.open_client = network_client_ssl_open,
	},
	[NETWORK_PROTOCOL_TCP] = {
		.name = "tcp",
		.type = NETWORK_PROTOCOL_TCP,
		.open_service = network_service_tcp_open,
		.open_client = network_client_tcp_open,
	},
	[NETWORK_PROTOCOL_UDP] = {
		.name = "udp",
		.type = NETWORK_PROTOCOL_UDP,
		.open_service = network_service_udp_open,
		.open_client = network_client_udp_open,
	},
	[NETWORK_PROTOCOL_ADB] = {
		.name = "adb",
		.type = NETWORK_PROTOCOL_ADB,
		.open_service = network_service_tcp_open,
		.open_client = network_client_adb_open,
	},
	[NETWORK_PROTOCOL_ICMP] = {
		.name = "icmp",
		.type = NETWORK_PROTOCOL_ICMP,
		.open_service = network_service_open_dummy,
		.open_client = network_client_icmp_open,
	},
	[NETWORK_PROTOCOL_IP] = {
		.name = "ip",
		.type = NETWORK_PROTOCOL_IP,
		.open_service = network_service_open_dummy,
		.open_client = network_client_ip_open,
	},
	[NETWORK_PROTOCOL_MAC] = {
		.name = "mac",
		.type = NETWORK_PROTOCOL_MAC,
		.open_service = network_service_open_dummy,
		.open_client = network_client_mac_open,
	},
	[NETWORK_PROTOCOL_UNIX_TCP] = {
		.name = "unix-tcp",
		.type = NETWORK_PROTOCOL_UNIX_TCP,
		.open_service = network_service_unix_tcp_open,
		.open_client = network_client_unix_tcp_open,
	},
	[NETWORK_PROTOCOL_UNIX_UDP] = {
		.name = "unix-udp",
		.type = NETWORK_PROTOCOL_UNIX_UDP,
		.open_service = network_service_unix_udp_open,
		.open_client = network_client_unix_udp_open,
	},
	[NETWORK_PROTOCOL_LOCAL_TCP] = {
		.name = "local-tcp",
		.type = NETWORK_PROTOCOL_LOCAL_TCP,
		.open_service = network_service_unix_tcp_open,
		.open_client = network_client_unix_tcp_open,
	},
	[NETWORK_PROTOCOL_LOCAL_UDP] = {
		.name = "local-udp",
		.type = NETWORK_PROTOCOL_LOCAL_UDP,
		.open_service = network_service_unix_udp_open,
		.open_client = network_client_unix_udp_open,
	},
	[NETWORK_PROTOCOL_UEVENT] = {
		.name = "uevent",
		.type = NETWORK_PROTOCOL_UEVENT,
		.open_service = network_service_open_dummy,
		.open_client = network_client_uevent_open,
	},
	[NETWORK_PROTOCOL_FILE_RW] = {
		.name = "file-rw",
		.type = NETWORK_PROTOCOL_FILE_RW,
		.open_service = network_service_file_open,
		.open_client = network_client_file_open,
	},
	[NETWORK_PROTOCOL_FILE_RO] = {
		.name = "file-ro",
		.type = NETWORK_PROTOCOL_FILE_RO,
		.open_service = network_service_file_open,
		.open_client = network_client_file_open,
	},
	[NETWORK_PROTOCOL_FILE_WO] = {
		.name = "file-wo",
		.type = NETWORK_PROTOCOL_FILE_WO,
		.open_service = network_service_file_open,
		.open_client = network_client_file_open,
	},
};

network_protocol_t network_protocol_parse(const char *name)
{
	const struct network_protocol_desc *p, *p_end;

	switch (name[0]) {
	case 't':
		if (text_cmp(name + 1, "cp") == 0) {
			return NETWORK_PROTOCOL_TCP;
		}
		break;

	case 'u':
		if (text_cmp(name + 1, "dp") == 0) {
			return NETWORK_PROTOCOL_UDP;
		} else if (text_cmp(name + 1, "event") == 0) {
			return NETWORK_PROTOCOL_UEVENT;
		} else if (text_lhcmp("nix", name + 1) == 0) {
			if (name[4] != '-') {
				if (name[4] == 0) {
					return NETWORK_PROTOCOL_UNIX_TCP;
				}
				break;
			}

			if (text_cmp(name + 5, "tcp") == 0) {
				return NETWORK_PROTOCOL_UNIX_TCP;
			} else if (text_cmp(name + 5, "udp") == 0) {
				return NETWORK_PROTOCOL_UNIX_UDP;
			}
		}
		break;

	case 'a':
		if (text_cmp(name + 1, "db") == 0) {
			return NETWORK_PROTOCOL_ADB;
		}
		break;

	case 'i':
		if (text_cmp(name + 1, "p") == 0) {
			return NETWORK_PROTOCOL_IP;
		} else if (text_cmp(name + 1, "cmp") == 0) {
			return NETWORK_PROTOCOL_ICMP;
		}
		break;

	case 'l':
		if (text_lhcmp("ocal", name + 1) == 0) {
			if (name[5] != '-') {
				if (name[5] == 0) {
					return NETWORK_PROTOCOL_LOCAL_TCP;
				}
				break;
			}

			if (text_cmp(name + 6, "tcp") == 0) {
				return NETWORK_PROTOCOL_LOCAL_TCP;
			} else if (text_cmp(name + 6, "udp") == 0) {
				return NETWORK_PROTOCOL_LOCAL_UDP;
			}
		}
		break;

	case 'm':
		if (text_cmp(name + 1, "ac") == 0) {
			return NETWORK_PROTOCOL_MAC;
		}
		break;

	case 'f':
		if (text_cmp(name + 1, "tp") == 0) {
			return NETWORK_PROTOCOL_FTP;
		} else if (text_lhcmp("ile", name + 1) == 0) {
			if (name[4] != '-') {
				if (name[4] == 0) {
					return NETWORK_PROTOCOL_FILE_RW;
				}
				break;
			}

			if (text_cmp(name + 5, "rw") == 0) {
				return NETWORK_PROTOCOL_FILE_RW;
			} else if (text_cmp(name + 5, "ro") == 0) {
				return NETWORK_PROTOCOL_FILE_RO;
			} else if (text_cmp(name + 5, "wo") == 0) {
				return NETWORK_PROTOCOL_FILE_WO;
			}
		}
		break;

	case 'h':
		if (text_lhcmp("ttp", name + 1) == 0) {
			if (name[4] == 0) {
				return NETWORK_PROTOCOL_HTTP;
			} else if (name[4] == 's' && name[5] == 0) {
				return NETWORK_PROTOCOL_HTTPS;
			}
		}
		break;

	case 's':
		if (text_cmp(name + 1, "sl") == 0) {
			return NETWORK_PROTOCOL_SSL;
		}
		break;
	}

	for (p = protocol_descs, p_end = p + NELEM(protocol_descs); p < p_end; p++) {
		if (strcmp(name, p->name) == 0) {
			return p->type;
		}
	}

	return NETWORK_PROTOCOL_INVALID;
}

const char *network_protocol_tostring(network_protocol_t type)
{
	const struct network_protocol_desc *desc;

	desc = network_get_protocol_by_type(type);
	if (desc == NULL) {
		return "unknown";
	}

	return desc->name;
}

const struct network_protocol_desc *network_get_protocol_by_name(const char *name)
{
	network_protocol_t type;

	type = network_protocol_parse(name);
	if (type == NETWORK_PROTOCOL_INVALID) {
		return NULL;
	}

	return protocol_descs + type;
}

const struct network_protocol_desc *network_get_protocol_by_type(network_protocol_t type)
{
	if (type < 0 || type >= NELEM(protocol_descs)) {
		return NULL;
	}

	return protocol_descs + type;
}

const struct network_protocol_desc *network_get_protocol_by_port(u16 port)
{
	const struct network_protocol_desc *p, *p_end;

	for (p = protocol_descs, p_end = p + NELEM(protocol_descs); p < p_end; p++) {
		if (p->port == port) {
			return p;
		}
	}

	return NULL;
}

int network_get_port_by_url(const struct network_url *url, const struct network_protocol_desc *protocol)
{
	if (url->port != NETWORK_PORT_INVALID) {
		return url->port;
	} else if (protocol) {
		return protocol->port;
	} else if (url->protocol[0]) {
		protocol = network_get_protocol_by_name(url->protocol);
		if (protocol) {
			return protocol->port;
		}

		pr_red_info("unknown protocol %s", url->protocol);
	}

	return -EINVAL;
}

int network_client_open(struct network_client *client, const struct network_url *url, int flags)
{
	const struct network_protocol_desc *desc;

#if CAVAN_NETWORK_DEBUG
	pr_bold_info("URL = %s", network_url_tostring(url, NULL, 0, NULL));
#endif

	desc = network_get_protocol_by_name(url->protocol);
	if (desc == NULL) {
		pr_red_info("network_get_protocol_by_name");
		return -EINVAL;
	}

	return network_protocol_open_client(desc, client, url, flags);
}

int network_client_open2(struct network_client *client, const char *url_text, int flags)
{
	struct network_url url;

	if (url_text == NULL || network_url_parse(&url, url_text) == NULL) {
		pr_red_info("network_parse_url");
		return -EINVAL;
	}

	return network_client_open(client, &url, flags);
}

void network_client_close(struct network_client *client)
{
	if (client->close) {
		client->close(client);
	} else {
		close(client->sockfd);
	}

	client->sockfd = -1;
}

int network_client_get_local_port(struct network_client *client)
{
	int ret;
	struct sockaddr_in addr;

	ret = network_client_get_local_addr(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	return addr.sin_port;
}

int network_client_get_remote_port(struct network_client *client)
{
	int ret;
	struct sockaddr_in addr;

	ret = network_client_get_remote_addr(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	return addr.sin_port;
}

int network_client_get_local_ip(struct network_client *client, struct in_addr *sin_addr)
{
	int ret;
	struct sockaddr_in addr;

	ret = network_client_get_local_addr(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	*sin_addr = addr.sin_addr;

	return 0;
}

int network_client_get_remote_ip(struct network_client *client, struct in_addr *sin_addr)
{
	int ret;
	struct sockaddr_in addr;

	ret = network_client_get_remote_addr(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	*sin_addr = addr.sin_addr;

	return 0;
}

int network_service_accept_timed(struct network_service *service, struct network_client *client, u32 msec)
{
	if (!file_poll_input(service->sockfd, msec)) {
		return -ETIMEDOUT;
	}

	return network_service_accept(service, client);
}

int network_service_open(struct network_service *service, const struct network_url *url, int flags)
{
	int ret;
	const struct network_protocol_desc *desc;

	signal(SIGPIPE, SIG_IGN); // prevent exit when disconnect

	pd_bold_info("URL = %s", network_url_tostring(url, NULL, 0, NULL));

	desc = network_get_protocol_by_name(url->protocol);
	if (desc == NULL) {
		pr_red_info("network_get_protocol_by_name");
		return -EINVAL;
	}

	ret = network_protocol_open_service(desc, service, url, flags);
	if (ret < 0) {
		return ret;
	}

	cavan_lock_init(&service->lock, false);

	return 0;
}

int network_service_open2(struct network_service *service, const char *url_text, int flags)
{
	struct network_url url;

	if (url_text == NULL || network_url_parse(&url, url_text) == NULL) {
		pr_red_info("network_parse_url");
		return -EFAULT;
	}

	return network_service_open(service, &url, flags);
}

void network_service_close(struct network_service *service)
{
	if (service->close) {
		service->close(service);
	} else {
		close(service->sockfd);
	}

	service->sockfd = -1;

	if (service->type == NETWORK_PROTOCOL_UNIX_TCP || service->type == NETWORK_PROTOCOL_UNIX_UDP) {
		remove_directory(network_get_socket_pathname());
	}

	cavan_lock_deinit(&service->lock);
}

int network_service_get_local_port(struct network_service *service)
{
	int ret;
	struct sockaddr_in addr;

	ret = network_service_get_local_addr(service, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	return addr.sin_port;
}

int network_get_devlist(char buff[][NETWORK_IFNAME_SIZE], int max_count)
{
	int count;
	char *mem;
	size_t file_size;
	char *text, *text_end;

	mem = file_read_all_text("/proc/net/dev", &file_size);
	if (mem == NULL) {
		return -EFAULT;
	}

	text = mem;
	text_end = text + file_size;

	text = text_find_next_line(text);
	text = text_find_next_line(text);

	for (count = 0; text < text_end && count < max_count; count++) {
		char *p, *p_end;

		text = text_skip_space_and_lf(text, text_end);

		for (p = buff[count], p_end = p + 8; text < text_end && *text != ':' && p < p_end; p++, text++) {
			*p = *text;
		}

		*p = 0;

		text = text_find_next_line(text);
	}

	free(mem);

	return count;
}

void cavan_inet_route_dump(const struct cavan_inet_route *route)
{
	println("devname = %s", route->devname);
	println("dstaddr = %s", inet_ntoa(route->dstaddr.sin_addr));
	println("gateway = %s", inet_ntoa(route->gateway.sin_addr));
	println("netmask = %s", inet_ntoa(route->netmask.sin_addr));
	println("flags = 0x%04x", route->flags);
	println("ref_count = %d", route->ref_count);
	println("use = %d", route->use);
	println("metric = %d", route->metric);
	println("mtu = %d", route->mtu);
	println("window = %d", route->window);
	println("irtt = %d", route->irtt);
}

int cavan_inet_get_route_table(struct cavan_inet_route *routes, int max_count)
{
	int ret;
	int count;
	const char *pathname = "/proc/net/route";

	FILE *fp = fopen(pathname, "r");
	if (fp == NULL) {
		pr_err_info("fopen %s", pathname);
		return -ENOENT;
	}

	ret = fscanf(fp, "%*[^\n]\n");
	if (ret < 0) {
		pr_err_info("fscanf");
		goto out_fclose;
	}

	for (count = 0; count < max_count; count++) {
		struct cavan_inet_route *route = routes + count;

		memset(route, 0x00, sizeof(struct cavan_inet_route));

		ret = fscanf(fp, "%15s%lx%lx%X%d%d%d%lx%d%d%d\n",
			route->devname,
			(unsigned long *) &route->dstaddr.sin_addr,
			(unsigned long *) &route->gateway.sin_addr,
			&route->flags, &route->ref_count, &route->use, &route->metric,
			(unsigned long *) &route->netmask.sin_addr,
			&route->mtu, &route->window, &route->irtt);

		if (ret != 11) {
			if (feof(fp)) {
				break;
			}

			if (ret >= 0) {
				ret = -EFAULT;
			}

			goto out_fclose;
		}

		route->dstaddr.sin_family = AF_INET;
		route->gateway.sin_family = AF_INET;
		route->netmask.sin_family = AF_INET;
	}

	ret = count;

out_fclose:
	fclose(fp);
	return ret;
}

int cavan_inet_find_default_route(struct cavan_inet_route routes[], int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (routes[i].dstaddr.sin_addr.s_addr == INADDR_ANY
			&& routes[i].gateway.sin_addr.s_addr != INADDR_ANY) {
			return i;
		}
	}

	for (i = 0; i < count; i++) {
		if (routes[i].dstaddr.sin_addr.s_addr != INADDR_ANY
			&& routes[i].gateway.sin_addr.s_addr == INADDR_ANY) {
			return i;
		}
	}

	for (i = 0; i < count; i++) {
		if (routes[i].gateway.sin_addr.s_addr != INADDR_ANY) {
			return i;
		}
	}

	for (i = 0; i < count; i++) {
		if (routes[i].dstaddr.sin_addr.s_addr != INADDR_ANY) {
			return i;
		}
	}

	return -EFAULT;
}

int cavan_inet_get_default_route(struct cavan_inet_route *route)
{
	int index;
	int count;
	struct cavan_inet_route routes[16];

	count = cavan_inet_get_route_table(routes, NELEM(routes));
	if (count < 1) {
		if (count < 0) {
			pr_red_info("cavan_inet_get_route_table");
			return count;
		}

		return -ENOENT;
	}

	if (count > 1) {
		index = cavan_inet_find_default_route(routes, count);
		if (index < 0) {
#if 0
			pr_red_info("cavan_inet_find_default_route: %d", index);
			return index;
#else
			index = 0;
#endif
		}
	} else {
		index = 0;
	}

	memcpy(route, routes + index, sizeof(struct cavan_inet_route));

	if (route->gateway.sin_addr.s_addr == INADDR_ANY) {
		route->gateway.sin_addr.s_addr = route->dstaddr.sin_addr.s_addr;
		((u8 *) &route->gateway.sin_addr.s_addr)[3] = 1;
	}

	return 0;
}

void cavan_inet_ifconfig_dump(const struct cavan_inet_ifconfig *config)
{
	println("if_name = %s", config->if_name);
	println("if_hwaddr = %s", mac_address_tostring(config->if_hwaddr, sizeof(config->if_hwaddr)));

	if (config->has_ip) {
		println("if_addr = %s", inet_ntoa(config->if_addr.sin_addr));
		println("if_dstaddr = %s", inet_ntoa(config->if_dstaddr.sin_addr));
		println("if_broadaddr = %s", inet_ntoa(config->if_broadaddr.sin_addr));
		println("if_netmask = %s", inet_ntoa(config->if_netmask.sin_addr));
	}
}

int cavan_inet_get_address(int sockfd, const char *ifname, int command, struct ifreq *ifr, struct sockaddr_in *addr)
{
	int ret;

	strncpy(ifr->ifr_ifrn.ifrn_name, ifname, sizeof(ifr->ifr_ifrn.ifrn_name));

	ifr->ifr_addr.sa_family = AF_INET;

	ret = ioctl(sockfd, command, ifr);
	if (ret < 0) {
		memset(addr, 0x00, sizeof(struct sockaddr_in));
	} else {
		memcpy(addr, &ifr->ifr_addr, sizeof(struct sockaddr_in));
	}

	return ret;
}

int cavan_inet_get_ifconfig(int sockfd, struct ifreq *ifr, struct cavan_inet_ifconfig *config)
{
	int ret;

	strncpy(config->if_name, ifr->ifr_ifrn.ifrn_name, sizeof(config->if_name));

	ret = ioctl(sockfd, SIOCGIFHWADDR, ifr);
	if (ret < 0) {
		pr_err_info("ioctl SIOCGIFHWADDR: %d", ret);
		return ret;
	}

	memcpy(config->if_hwaddr, ifr->ifr_hwaddr.sa_data, sizeof(config->if_hwaddr));

#if CAVAN_IFCONFIG_DEBUG
	pr_func_info("ifname = %s", config->if_name);
#endif

	ret = cavan_inet_get_address(sockfd, config->if_name, SIOCGIFADDR, ifr, &config->if_addr);
	if (ret < 0) {
		config->has_ip = false;
	} else {
		config->has_ip = true;

		cavan_inet_get_address(sockfd, config->if_name, SIOCGIFDSTADDR, ifr, &config->if_dstaddr);
		cavan_inet_get_address(sockfd, config->if_name, SIOCGIFBRDADDR, ifr, &config->if_broadaddr);
		cavan_inet_get_address(sockfd, config->if_name, SIOCGIFNETMASK, ifr, &config->if_netmask);
	}

	return 0;
}

int cavan_inet_get_ifconfig2(int sockfd, const char *ifname, struct cavan_inet_ifconfig *config)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_ifrn.ifrn_name, ifname);

	return cavan_inet_get_ifconfig(sockfd, &ifr, config);
}

int cavan_inet_get_ifconfig_list(int sockfd, struct cavan_inet_ifconfig *configs, int max_count)
{
	int i;
	int ret;
	int count;
	struct ifconf ifc;
	struct ifreq ifr[NETWORK_DEV_MAX_COUNT];

	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_ifcu.ifcu_req = ifr;

	ret = ioctl(sockfd, SIOCGIFCONF, &ifc);
	if (ret < 0) {
		pr_err_info("ioctl SIOCGIFCONF: %d", ret);
		return ret;
	}

	count = ifc.ifc_len / sizeof(struct ifreq);

#if CAVAN_IFCONFIG_DEBUG
	pr_func_info("count = %d", count);
#endif

	for (i = 0; i < count; i++) {
		ret = cavan_inet_get_ifconfig(sockfd, ifr + i, configs + i);
		if (ret < 0) {
			pr_red_info("inet_get_ifconfig: %d", ret);
			return ret;
		}
	}

	return count;
}

int cavan_inet_get_ifconfig_list2(struct cavan_inet_ifconfig *configs, int max_count)
{
	int ret;
	int sockfd;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket: %d", sockfd);
		return sockfd;
	}

	ret = cavan_inet_get_ifconfig_list(sockfd, configs, max_count);
	if (ret < 0) {
		pr_red_info("inet_get_ifconfig_list: %d", ret);
	}

	close(sockfd);

	return ret;
}

static int udp_discovery_service_handler(struct cavan_thread *thread, void *data)
{
	ssize_t rdlen;
	ssize_t wrlen;
	char command[64];
	struct sockaddr_in addr;
	struct udp_discovery_service *service = data;
	struct network_service *udp_service = &service->service;

	rdlen = udp_service->recvfrom(udp_service, command, sizeof(command) - 1, (struct sockaddr *) &addr);
	if (rdlen < 0) {
		pr_err_info("udp_service->recvfrom");
		return rdlen;
	}

	inet_show_sockaddr(&addr);

	command[rdlen] = 0;
	pd_info("command = %s", command);

	if (strcmp(command, CAVAN_DISCOVERY_COMMAND) != 0) {
		return 0;
	}

	wrlen = udp_service->sendto(udp_service, service->command, service->command_len, (struct sockaddr *) &addr);
	if (wrlen < 0) {
		pr_err_info("udp_service->sendto");
		return wrlen;
	}

	return 0;
}

int udp_discovery_service_start(struct udp_discovery_service *service, const char *command, ...)
{
	int ret;
	va_list ap;
	struct network_url url;
	struct cavan_thread *thread = &service->thread;
	struct network_service *udp_service = &service->service;

	network_url_init(&url, "udp", NULL, service->port, NULL);

	ret = network_service_open(udp_service, &url, 0);
	if (ret < 0) {
		pr_red_info("network_service_open: %d", ret);
		return ret;
	}

	va_start(ap, command);
	service->command_len = vsnprintf(service->command, sizeof(service->command), command, ap);
	va_end(ap);

	thread->name = "UDP_DISCOVERY";
	thread->wake_handker = NULL;
	thread->handler = udp_discovery_service_handler;

	ret = cavan_thread_run(thread, service, 0);
	if (ret < 0) {
		pr_red_info("cavan_thread_run: %d", ret);
		goto out_network_service_close;
	}

	return 0;

out_network_service_close:
	network_service_close(udp_service);
	return ret;
}

void udp_discovery_service_stop(struct udp_discovery_service *service)
{
	network_service_close(&service->service);
	cavan_thread_stop(&service->thread);
}

static void udp_discovery_client_handler_dummy(int index, const char *command, struct sockaddr_in *addr, void *data)
{
	pr_green_info("%d. %-15s %s", index, inet_ntoa(addr->sin_addr), command);
}

int udp_discovery_client_run(u16 port, void *data, void (*handler)(int index, const char *command, struct sockaddr_in *addr, void *data))
{
	int ret;
	int count;
	ssize_t wrlen;
	struct network_url url;
	struct sockaddr_in addr;
	struct network_client client;

	network_url_init(&url, "udp", "all-hosts", port, NULL);

	ret = network_client_open(&client, &url, 0);
	if (ret < 0) {
		pr_red_info("network_client_open: %d", ret);
		return ret;
	}

	ret = inet_hostname2sockaddr(url.hostname, &addr);
	if (ret < 0) {
		pr_red_info("inet_hostname2sockaddr");
		return ret;
	}

	addr.sin_port = htons(port);

	wrlen = client.sendto(&client, CAVAN_DISCOVERY_COMMAND, strlen(CAVAN_DISCOVERY_COMMAND), (struct sockaddr *) &addr);
	if (wrlen < 0) {
		pr_err_info("client->sendto");
		goto out_network_client_close;
	}

	if (handler == NULL) {
		handler = udp_discovery_client_handler_dummy;
	}

	for (count = 0; file_poll_input(client.sockfd, 2000); count++) {
		ssize_t rdlen;
		char command[1024];

		rdlen = client.recvfrom(&client, command, sizeof(command) - 1, (struct sockaddr *) &addr);
		if (rdlen < 0) {
			pr_err_info("network_client_recv");
			goto out_network_client_close;
		}

		command[rdlen] = 0;
		handler(count, command, &addr, data);
	}

	ret = count;

out_network_client_close:
	network_client_close(&client);
	return ret;
}

static void *tcp_discovery_client_thread(void *_data)
{
	int ret;
	struct tcp_discovery_data *data = _data;
	struct tcp_discovery_client *discovery = data->discovery;

	ret = network_client_open(&data->client, &data->url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT);
	if (ret >= 0) {
		pthread_mutex_lock(&discovery->lock);

		data->index = discovery->count + 1;
		if (discovery->handler(discovery, data)) {
			discovery->count++;
		}

		pthread_mutex_unlock(&discovery->lock);

		network_client_close(&data->client);
	}

	pthread_mutex_lock(&discovery->lock);
	discovery->pendding--;
	data->pendding = false;
	pthread_mutex_unlock(&discovery->lock);

	return NULL;
}

static bool tcp_discovery_client_handler_dummy(struct tcp_discovery_client *discovery, struct tcp_discovery_data *data)
{
	int ret;
	struct in_addr addr;

	ret = network_client_get_remote_ip(&data->client, &addr);
	if (ret < 0) {
		return false;
	}

	pr_green_info("%03d. %s", data->index, inet_ntoa(addr));

	return true;
}

int tcp_discovery_client_run(struct tcp_discovery_client *client, void *data)
{
	int i;
	int ret;
	struct tcp_discovery_data datas[255];

	pthread_mutex_init(&client->lock, NULL);

	client->count = 0;
	client->private_data = data;
	client->pendding = NELEM(datas);

	if (client->handler == NULL) {
		client->handler = tcp_discovery_client_handler_dummy;
	}

	println("PORT = %d", client->port);

	for (i = 0; i < NELEM(datas); i++) {
		struct tcp_discovery_data *data = datas + i;

		data->pendding = true;
		data->discovery = client;
		value2text_unsigned_simple(i, data->url.memory, sizeof(data->url.memory), 10);
		network_url_init(&data->url, "tcp", data->url.memory, client->port, NULL);

		ret = cavan_pthread_create(&data->thread, tcp_discovery_client_thread, data, false);
		if (ret < 0) {
			data->pendding = false;
		}
	}

	pthread_mutex_lock(&client->lock);

	for (i = 0; i < 3 && client->pendding; i++) {
		pthread_mutex_unlock(&client->lock);
		msleep(1000);
		pthread_mutex_lock(&client->lock);
	}

	for (i = 0; i < NELEM(datas); i++) {
		if (datas[i].pendding) {
			cavan_pthread_kill(datas[i].thread);
		}
	}

	pthread_mutex_unlock(&client->lock);

	pthread_mutex_destroy(&client->lock);

	return client->count;
}
