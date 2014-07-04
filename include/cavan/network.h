#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 21 10:08:25 CST 2011

#include <cavan.h>
#include <netdb.h>
#include <sys/socket.h>
// #include <bits/sockaddr.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/un.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#define NETWORK_TIMEOUT_VALUE	5000
#define NETWORK_RETRY_COUNT		5
#define ROUTE_TABLE_SIZE		16
#define MAC_ADDRESS_LEN			6
#define CAVAN_LISTEN_BACKLOG	32
#define CAVAN_NETWORK_MAGIC		0x88888888

#define SYS_NET_DIRECTORY		"/sys/class/net"
#define LOCAL_HOST_IP			"127.0.0.1"
#define CAVAN_DEFAULT_IP		LOCAL_HOST_IP
#define CAVAN_IP_ENV_NAME		"CAVAN_SERVER_IP"
#define CAVAN_PORT_ENV_NAME		"CAVAN_SERVER_PORT"

#pragma pack(1)
struct mac_header
{
	u8 dest_mac[6];
	u8 src_mac[6];
	u16 protocol_type;

	u8 data[0];
};

struct ip_header
{
	u8 version			:4;
	u8 header_length	:4;
	u8 service_type;
	u16 total_length;

	u32 flags			:19;
	u32 piece_offset	:13;

	u8 ttl;
	u8 protocol_type;
	u16 header_checksum;

	u32 src_ip;
	u32 dest_ip;

	u8 data[0];
};

struct udp_header
{
	u16 src_port;
	u16 dest_port;

	u16 udp_length;
	u16 udp_checksum;

	u8 data[0];
};

struct tcp_header
{
	u16 src_port;
	u16 dest_port;

	u32 sequence;
	u32 sck_sequence;

	u16 header_length	:4;
	u16 reserve			:6;
	u16 TCP_URG			:1;
	u16 TCP_ACK			:1;
	u16 TCP_PSH			:1;
	u16 TCP_RST			:1;
	u16 TCP_SYN			:1;
	u16 TCP_FIN			:1;
	u16 window_size;

	u16 checksum;
	u16 urgent_pointer;

	u8 data[0];
};

struct arp_header
{
	u16 hardware_type;
	u16 protocol_type;
	u8 hardware_addrlen;
	u8 protocol_addrlen;
	u16 op_code;
	u8 src_mac[6];
	u32 src_ip;
	u8 dest_mac[6];
	u32 dest_ip;
};

struct icmp_header
{
	u8 type;
	u8 code;
	u16 checksum;

	u8 data[0];
};

struct dhcp_header
{
	u8 opcode;
	u8 htype;
	u8 hlen;
	u8 hops;

	u32 transction_id;

	u16 seconds;
	u16 flags;

	u32 ciaddr;
	u32 yiaddr;
	u32 siaddr;
	u32 giaddr;

	u8 chaddr[16];
	u8 sname[64];
	u8 file[128];

	u8 options[0];
};

struct udp_pseudo_header
{
	u32 src_ip;
	u32 dest_ip;

	u8 zero;
	u8 protocol;
	u16 udp_length;
};
#pragma pack()

struct cavan_route_node
{
	void *data;
	u8 mac_addr[MAC_ADDRESS_LEN];
	u32 ip_addr;
};

struct cavan_route_table
{
	struct cavan_route_node **route_table;
	int table_size;
};

struct inet_file_request
{
	const char *hostname;
	u16 port;

	union
	{
		struct
		{
			char src_file[1024];
			char dest_file[1024];
			off_t src_offset;
			off_t dest_offset;
			off_t size;
		};

		char command[2048];
	};

	int (*open_connect)(const char *hostname, u16 port);
	void (*close_connect)(int sockfd);
};

typedef enum
{
	NETWORK_PROTOCOL_INVALID = -1,
	NETWORK_PROTOCOL_HTTP,
	NETWORK_PROTOCOL_HTTPS,
	NETWORK_PROTOCOL_FTP,
} network_protocol_type_t;

struct network_protocol
{
	const char *name;
	u16 port;
	network_protocol_type_t type;
};

struct network_url
{
	char port[8];
	char protocol[8];
	char hostname[512];
};

struct inet_connect
{
	int sockfd;
	struct sockaddr_in addr;
};

typedef enum
{
	NETWORK_CONNECT_UNKNOWN,
	NETWORK_CONNECT_TCP,
	NETWORK_CONNECT_UDP,
	NETWORK_CONNECT_UNIX,
	NETWORK_CONNECT_ADB,
	NETWORK_CONNECT_ICMP,
	NETWORK_CONNECT_IP,
	NETWORK_CONNECT_MAC,
} network_connect_type_t;

struct network_client
{
	int sockfd;
	socklen_t addrlen;
	void *private_data;
	struct sockaddr addr;
	network_connect_type_t type;

	void (*close)(struct network_client *client);
	ssize_t (*send)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv)(struct network_client *client, void *buff, size_t size);
};

struct network_service
{
	int sockfd;
	void *private_data;
	pthread_mutex_t lock;
	network_connect_type_t type;

	int (*accept)(struct network_service *service, struct network_client *conn);
	void (*close)(struct network_service *service);
};

struct network_file_request
{
	char src_file[1024];
	char dest_file[1024];
	off_t src_offset;
	off_t dest_offset;
	off_t size;
};

extern int adb_create_tcp_link(const char *ip, u16 port, u16 tcp_port);

ssize_t sendto_select(int sockfd, int retry, const void *buff, size_t len, const struct sockaddr_in *remote_addr);
ssize_t sendto_receive(int sockfd, long timeout, int retry, const void *send_buff, ssize_t sendlen, void *recv_buff, ssize_t recvlen, struct sockaddr_in *remote_addr, socklen_t *addr_len);

const char *mac_protocol_type_tostring(int type);
const char *ip_protocol_type_tostring(int type);
void show_mac_header(struct mac_header *hdr);
void show_ip_header(struct ip_header *hdr, int simple);
void show_tcp_header(struct tcp_header *hdr);
void show_udp_header(struct udp_header *hdr);
void show_arp_header(struct arp_header *hdr, int simple);
void show_dhcp_header(struct dhcp_header *hdr);
void show_icmp_header(struct icmp_header *hdr);

int cavan_route_table_init(struct cavan_route_table *table, size_t table_size);
void cavan_route_table_deinit(struct cavan_route_table *table);
int cavan_route_table_insert_node(struct cavan_route_table *table, struct cavan_route_node *node);
struct cavan_route_node **cavan_find_route_by_mac(struct cavan_route_table *table, u8 *mac);
struct cavan_route_node **cavan_find_route_by_ip(struct cavan_route_table *table, u32 ip);
int cavan_route_table_delete_node(struct cavan_route_table *table, struct cavan_route_node *node);
int cavan_route_table_delete_by_mac(struct cavan_route_table *table, u8 *mac);
int cavan_route_table_delete_by_ip(struct cavan_route_table *table, u32 ip);

u16 udp_checksum(struct ip_header *ip_hdr);

void inet_sockaddr_init(struct sockaddr_in *addr, const char *ip, u16 port);
int inet_create_tcp_link1(const struct sockaddr_in *addr);
int inet_create_tcp_link2(const char *hostname, u16 port);
int inet_create_tcp_link_by_addrinfo(struct addrinfo *info, u16 port, struct sockaddr_in *addr);
int inet_create_service(int type, u16 port);
int inet_create_tcp_service(u16 port);

void unix_sockaddr_init(struct sockaddr_un *addr, const char *pathname);
int unix_create_service(int type, const char *pathname);
int unix_create_tcp_service(const char *pathname);
int unix_create_tcp_link(const char *hostname, u16 port);

ssize_t inet_tcp_sendto(struct sockaddr_in *addr, const void *buff, size_t size);

u32 get_rand_value(void);
int inet_bind_rand(int sockfd, int retry);

ssize_t inet_send(int sockfd, const char *buff, size_t size);
int inet_tcp_send_file1(int sockfd, int fd);
int inet_tcp_send_file2(int sockfd, const char *filename);
int inet_tcp_receive_file1(int sockfd, int fd);
int inet_tcp_receive_file2(int sockfd, const char *filename);
int inet_tcp_transfer(int src_sockfd, int dest_sockfd, size_t size);

int inet_get_sockaddr(int sockfd, const char *devname, struct sockaddr_in *sin_addr);
int inet_get_devname(int sockfd, int index, char *devname);

const char *cavan_get_server_hostname(void);
u16 cavan_get_server_port(u16 default_port);

int inet_tcp_transmit_loop(int src_sockfd, int dest_sockfd);
int inet_hostname2sockaddr(const char *host, struct sockaddr_in *addr);

char *network_url_tostring(const struct network_url *url, char *buff, size_t size, char **tail);
char *network_url_build(char *buff, size_t size, const char *protocol, const char *hostname, u16 port, const char *pathname);
char *network_parse_url(const char *text, struct network_url *url);
const struct network_protocol *network_get_protocol_by_name(const char *name);
const struct network_protocol *network_get_protocol_by_type(network_protocol_type_t type);
const struct network_protocol *network_get_protocol_by_port(u16 port);
int network_get_port_by_url(const struct network_url *url, const struct network_protocol *protocol);
bool network_url_equals(const struct network_url *url1, const struct network_url *url2);
int network_create_socket_mac(const char *if_name, int protocol);

int network_client_open(struct network_client *client, network_connect_type_t type, const char *hostname, u16 port, const char *pathname);
int network_client_open2(struct network_client *client, const char *url);
void network_client_close(struct network_client *client);
ssize_t network_client_fill_buff(struct network_client *client, char *buff, size_t size);
ssize_t network_client_send_buff(struct network_client *client, const char *buff, size_t size);
ssize_t network_client_recv_file(struct network_client *client, int fd, size_t size);
ssize_t network_client_send_file(struct network_client *client, int fd, size_t size);
int network_client_exec_redirect(struct network_client *client, int ttyin, int ttyout);
int network_client_exec_main(struct network_client *client, const char *command, int lines, int columns);

int network_service_open(struct network_service *service, network_connect_type_t type, u16 port, const char *pathname);
int network_service_open2(struct network_service *service, const char *url);
void network_service_close(struct network_service *service);

static inline int inet_socket(int type)
{
	return socket(PF_INET, type, 0);
}

static inline int unix_socket(int type)
{
	return socket(PF_UNIX, type, 0);
}

static inline int inet_bind(int sockfd, const struct sockaddr_in *addr)
{
	return bind(sockfd, (const struct sockaddr *) addr, sizeof(*addr));
}

static inline int unix_bind(int sockfd, const struct sockaddr_un *addr)
{
	return bind(sockfd, (const struct sockaddr *) addr, sizeof(*addr));
}

static inline int inet_connect(int sockfd, const struct sockaddr_in *addr)
{
	return connect(sockfd, (const struct sockaddr *) addr, sizeof(*addr));
}

static inline int unix_connect(int sockfd, const struct sockaddr_un *addr)
{
	return connect(sockfd, (const struct sockaddr *) addr, sizeof(*addr));
}

static inline int inet_accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen)
{
	*addrlen = sizeof(struct sockaddr_in);

	return accept(sockfd, (struct sockaddr *)addr, addrlen);
}

static inline ssize_t inet_sendto(int sockfd, const void *buff, size_t size, const struct sockaddr_in *addr)
{
	return sendto(sockfd, buff, size, 0, (const struct sockaddr *)addr, sizeof(*addr));
}

static inline ssize_t inet_recvfrom(int sockfd, void *buff, size_t size, struct sockaddr_in *addr, socklen_t *addrlen)
{
	*addrlen = sizeof(struct sockaddr_in);

	return recvfrom(sockfd, buff, size, 0, (struct sockaddr *)addr, addrlen);
}

static inline int inet_create_udp_service(u16 port)
{
	return inet_create_service(SOCK_DGRAM, port);
}

static inline int inet_listen(int sockfd)
{
	return listen(sockfd, CAVAN_LISTEN_BACKLOG);
}

static inline ssize_t inet_send_text(int sockfd, const char *text)
{
	return inet_send(sockfd, text, text_len(text));
}

static inline ssize_t inet_send_text_to(int sockfd, const char *text, const struct sockaddr_in *addr)
{
	return inet_sendto(sockfd, text, text_len(text), addr);
}

static inline ssize_t inet_recv(int sockfd, void *buff, size_t size)
{
	return recv(sockfd, buff, size, MSG_NOSIGNAL);
}

static inline ssize_t inet_recv2(int sockfd, void *buff, size_t size)
{
	return recv(sockfd, buff, size, 0);
}

static inline ssize_t inet_recv_timeout(int sockfd, void *buff, size_t size, int timeout_ms)
{
	if (file_poll_input(sockfd, timeout_ms))
	{
		return inet_recv(sockfd, buff, size);
	}

	return -ETIMEDOUT;
}

static inline ssize_t inet_recvfrom_timeout(int sockfd, void *buff, size_t size, struct sockaddr_in *addr, socklen_t *addrlen, int timeout_ms)
{
	if (file_poll_input(sockfd, timeout_ms))
	{
		return inet_recvfrom(sockfd, buff, size, addr, addrlen);
	}

	return -ETIMEDOUT;
}

static inline void inet_close_tcp_socket(int sockfd)
{
	fsync(sockfd);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

static inline int inet_getsockname(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen)
{
	*addrlen = sizeof(struct sockaddr_in);
	return getsockname(sockfd, (struct sockaddr *)addr, addrlen);
}

static inline void inet_show_sockaddr(const struct sockaddr_in *addr)
{
	println("IP = %s, PORT = %d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

static inline void unix_show_sockaddr(const struct sockaddr_un *addr)
{
	println("PATH = %s", addr->sun_path);
}

static inline bool inet_sockaddr_equals(const struct sockaddr_in *left, const struct sockaddr_in *right)
{
	return memcmp(&left->sin_addr, &right->sin_addr, sizeof(left->sin_addr)) == 0 && left->sin_port == right->sin_port;
}

static inline void network_client_set_data(struct network_client *client, void *data)
{
	client->private_data = data;
}

static inline void *network_client_get_data(struct network_client *client)
{
	return client->private_data;
}

static inline ssize_t network_client_send_message(struct network_client *client, u32 message)
{
	return network_client_send_buff(client, (char *) &message, sizeof(message));
}

static inline ssize_t network_client_recv_message(struct network_client *client, u32 *message)
{
	return network_client_fill_buff(client, (char *) message, sizeof(*message));
}

static inline void network_service_set_data(struct network_service *service, void *data)
{
	service->private_data = data;
}

static inline void *network_service_get_data(struct network_service *service)
{
	return service->private_data;
}

static inline void network_service_lock(struct network_service *service)
{
	pthread_mutex_lock(&service->lock);
}

static inline void network_service_unlock(struct network_service *service)
{
	pthread_mutex_unlock(&service->lock);
}
