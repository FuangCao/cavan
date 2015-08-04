#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 21 10:08:25 CST 2011

#include <cavan.h>
#include <cavan/cache.h>
#include <cavan/command.h>
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

#define NETWORK_PORT_INVALID	0
#define NETWORK_PORT_FTP		21
#define NETWORK_PORT_HTTP		80
#define NETWORK_PORT_HTTPS		445

#define NETWORK_TIMEOUT_VALUE	5000
#define NETWORK_RETRY_COUNT		5
#define ROUTE_TABLE_SIZE		16
#define MAC_ADDRESS_LEN			6
#define CAVAN_LISTEN_BACKLOG	32
#define CAVAN_NETWORK_MAGIC		0x88888888

#define SYS_NET_DIRECTORY		"/sys/class/net"
#define LOCAL_HOST_IP			"127.0.0.1"
#define CAVAN_DEFAULT_IP		LOCAL_HOST_IP
#define CAVAN_DEFAULT_PORT		8888
#define CAVAN_DEFAULT_PROTOCOL	"tcp"
#define CAVAN_IP_ENV_NAME		"CAVAN_SERVER_IP"
#define CAVAN_PORT_ENV_NAME		"CAVAN_SERVER_PORT"

#define CAVAN_NET_UDP_RETRY			10
#define CAVAN_NET_UDP_TIMEOUT		200
#define CAVAN_NET_UDP_ACTIVE_TIME	(1000 * 60 * 20)
#define CAVAN_NET_FLAG_TALK			(1 << 0)
#define CAVAN_NET_FLAG_SYNC			(1 << 1)
#define CAVAN_NET_FLAG_WAIT			(1 << 2)

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

struct ping_header
{
	u16 id;
	u16 seq;

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
	NETWORK_PROTOCOL_FTP,
	NETWORK_PROTOCOL_HTTP,
	NETWORK_PROTOCOL_HTTPS,
	NETWORK_PROTOCOL_TCP,
	NETWORK_PROTOCOL_UDP,
	NETWORK_PROTOCOL_ADB,
	NETWORK_PROTOCOL_ICMP,
	NETWORK_PROTOCOL_IP,
	NETWORK_PROTOCOL_MAC,
	NETWORK_PROTOCOL_UNIX_TCP,
	NETWORK_PROTOCOL_UNIX_UDP,
	NETWORK_PROTOCOL_UEVENT,
} network_protocol_t;

struct network_url
{
	u16 port;
	const char *protocol;
	const char *hostname;
	const char *pathname;
	char memory[128];
};

struct inet_connect
{
	int sockfd;
	struct sockaddr_in addr;
};

typedef enum
{
	CAVAN_SYNC_TYPE_ACK,
	CAVAN_SYNC_TYPE_DATA,
} cavan_sync_type_t;

struct cavan_sync_package
{
	u8 type;
	u8 index;
	u16 length;
	char data[0];
};

struct network_client
{
	int sockfd;
	socklen_t addrlen;
	void *private_data;
	network_protocol_t type;

	void (*close)(struct network_client *client);
	ssize_t (*send)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv)(struct network_client *client, void *buff, size_t size);
};

struct network_client_sync_data
{
	pthread_mutex_t lock;
	u8 send_index, recv_index;
	u8 send_pending, recv_pending;

	void (*close)(struct network_client *client);
	ssize_t (*send)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv)(struct network_client *client, void *buff, size_t size);
};

#if 0
struct network_client_inet
{
	struct network_client client;
	struct sockaddr_in addr;
};

struct network_client_unix
{
	struct network_client client;
	struct sockaddr_un addr;
};
#endif

struct network_service
{
	int sockfd;
	socklen_t addrlen;
	void *private_data;
	network_protocol_t type;

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

struct network_protocol_desc
{
	const char *name;
	u16 port;
	network_protocol_t type;

	int (*open_client)(struct network_client *client, const struct network_url *url, u16 port, int flags);
	int (*open_service)(struct network_service *service, const struct network_url *url, u16 port, int flags);
};

extern int adb_create_tcp_link(const char *ip, u16 port, u16 tcp_port);

const char *network_get_socket_pathname(void);
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
void show_ping_header(struct ping_header *hdr);
void inet_show_sockaddr(const struct sockaddr_in *addr);

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

int inet_tcp_transmit_loop(int src_sockfd, int dest_sockfd);
int inet_hostname2sockaddr(const char *host, struct sockaddr_in *addr);

int inet_create_link(const struct sockaddr_in *addr, int socktype, int protocol);
int network_create_link(const char *hostname, u16 port, int socktype, int protocol);

char *network_sockaddr_tostring(const struct sockaddr *addr, char *buff, size_t size);
void network_url_init(struct network_url *url, const char *protocol, const char *hostname, u16 port, const char *pathname);
char *network_url_get_pathname(const struct network_url *url, char *buff, size_t size);
char *network_url_tostring(const struct network_url *url, char *buff, size_t size, char **tail);
char *network_url_parse(struct network_url *url, const char *text);

const struct network_protocol_desc *network_get_protocol_by_name(const char *name);
const struct network_protocol_desc *network_get_protocol_by_type(network_protocol_t type);
const struct network_protocol_desc *network_get_protocol_by_port(u16 port);
int network_get_port_by_url(const struct network_url *url, const struct network_protocol_desc *protocol);
bool network_url_equals(const struct network_url *url1, const struct network_url *url2);
int network_create_socket_mac(const char *if_name, int protocol);
int network_create_socket_uevent(void);

network_protocol_t network_protocol_parse(const char *name);
const char *network_protocol_tostring(network_protocol_t type);

int network_client_open(struct network_client *client, const struct network_url *url, int flags);
int network_client_open2(struct network_client *client, const char *url, int flags);
void network_client_close(struct network_client *client);
int network_client_vprintf(struct network_client *client, const char *format, va_list ap);
int network_client_printf(struct network_client *client, const char *format, ...);
ssize_t network_client_fill_buff(struct network_client *client, char *buff, size_t size);
ssize_t network_client_recv_file(struct network_client *client, int fd, size_t size);
ssize_t network_client_recv_file2(struct network_client *client, const char *pathname, size_t size, int flags);
ssize_t network_client_send_file(struct network_client *client, int fd, size_t size);
ssize_t network_client_send_file2(struct network_client *client, const char *pathname, size_t size);
ssize_t network_client_timed_recv(struct network_client *client, void *buff, size_t size, int timeout);
bool network_client_discard_all(struct network_client *client);
int network_client_exec_redirect(struct network_client *client, int ttyin, int ttyout);
int network_client_exec_main(struct network_client *client, const char *command, int lines, int columns);
int network_client_get_local_port(struct network_client *client);
int network_client_get_remote_port(struct network_client *client);
int network_client_get_local_ip(struct network_client *client, struct in_addr *sin_addr);
int network_client_get_remote_ip(struct network_client *client, struct in_addr *sin_addr);

int network_service_open(struct network_service *service, const struct network_url *url, int flags);
int network_service_open2(struct network_service *service, const char *url, int flags);
void network_service_close(struct network_service *service);
int network_service_get_local_port(struct network_service *service);

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

	return accept(sockfd, (struct sockaddr *) addr, addrlen);
}

static inline ssize_t inet_sendto(int sockfd, const void *buff, size_t size, const struct sockaddr_in *addr)
{
	return sendto(sockfd, buff, size, 0, (const struct sockaddr *) addr, sizeof(*addr));
}

static inline ssize_t inet_recvfrom(int sockfd, void *buff, size_t size, struct sockaddr_in *addr, socklen_t *addrlen)
{
	*addrlen = sizeof(struct sockaddr_in);

	return recvfrom(sockfd, buff, size, 0, (struct sockaddr *) addr, addrlen);
}

static inline int inet_create_udp_service(u16 port)
{
	return inet_create_service(SOCK_DGRAM, port);
}

static inline int unix_create_udp_service(const char *pathname)
{
	return unix_create_service(SOCK_DGRAM, pathname);
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

static inline int inet_getsockname(int sockfd, struct sockaddr_in *addr)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);

	return getsockname(sockfd, (struct sockaddr *) addr, &addrlen);
}

static inline int inet_getpeername(int sockfd, struct sockaddr_in *addr)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);

	return getpeername(sockfd, (struct sockaddr *) addr, &addrlen);
}

static inline void unix_show_sockaddr(const struct sockaddr_un *addr)
{
	println("PATH = %s", addr->sun_path);
}

static inline bool inet_sockaddr_equals(const struct sockaddr_in *left, const struct sockaddr_in *right)
{
	return memcmp(&left->sin_addr, &right->sin_addr, sizeof(left->sin_addr)) == 0 && left->sin_port == right->sin_port;
}

static inline int network_protocol_open_client(const struct network_protocol_desc *desc, struct network_client *client, const struct network_url *url, int flags)
{
	client->type = desc->type;

	return desc->open_client(client, url, network_get_port_by_url(url, desc), flags);
}

static inline int network_protocol_open_service(const struct network_protocol_desc *desc, struct network_service *service, const struct network_url *url, int flags)
{
	service->type = desc->type;

	return desc->open_service(service, url, network_get_port_by_url(url, desc), flags);
}

static inline void network_client_set_data(struct network_client *client, void *data)
{
	client->private_data = data;
}

static inline void *network_client_get_data(struct network_client *client)
{
	return client->private_data;
}

static inline ssize_t network_client_send(struct network_client *client, const void *buff, size_t size)
{
	return client->send(client, buff, size);
}

static inline ssize_t network_client_recv(struct network_client *client, void *buff, size_t size)
{
	return client->recv(client, buff, size);
}

static inline ssize_t network_client_send_message(struct network_client *client, u32 message)
{
	return client->send(client, (char *) &message, sizeof(message));
}

static inline ssize_t network_client_recv_message(struct network_client *client, u32 *message)
{
	return network_client_fill_buff(client, (char *) message, sizeof(*message));
}

static inline ssize_t network_client_send_text(struct network_client *client, const char *text)
{
	return client->send(client, text, text_len(text));
}

static inline int network_client_get_local_addr(struct network_client *client, struct sockaddr *addr, socklen_t addrlen)
{
	return getsockname(client->sockfd, addr, &addrlen);
}

static inline int network_client_get_remote_addr(struct network_client *client, struct sockaddr *addr, socklen_t addrlen)
{
	return getpeername(client->sockfd, addr, &addrlen);
}

static inline ssize_t network_client_fifo_read(struct cavan_fifo *fifo, void *buff, size_t size)
{
	return network_client_recv(fifo->private_data, buff, size);
}

static inline ssize_t network_client_fifo_write(struct cavan_fifo *fifo, const void *buff, size_t size)
{
	return network_client_send(fifo->private_data, buff, size);
}

static inline void network_service_set_data(struct network_service *service, void *data)
{
	service->private_data = data;
}

static inline void *network_service_get_data(struct network_service *service)
{
	return service->private_data;
}

static inline int network_service_accept(struct network_service *service, struct network_client *client)
{
	return service->accept(service, client);
}

static inline int network_service_get_local_addr(struct network_service *service, struct sockaddr *addr, socklen_t addrlen)
{
	return getsockname(service->sockfd, addr, &addrlen);
}

static inline const char *cavan_get_server_hostname(void)
{
	return cavan_getenv(CAVAN_IP_ENV_NAME, CAVAN_DEFAULT_IP);
}

static inline u16 cavan_get_server_port(u16 default_value)
{
	return cavan_getenv_u32(CAVAN_PORT_ENV_NAME, default_value);
}
