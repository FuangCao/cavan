#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 21 10:08:25 CST 2011

#include <cavan.h>
#include <cavan/cache.h>
#include <cavan/thread.h>
#include <cavan/command.h>
#include <cavan/service.h>
#include <netdb.h>
#include <sys/socket.h>
// #include <bits/sockaddr.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/un.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <semaphore.h>

#define NETWORK_PORT_INVALID	0
#define NETWORK_PORT_FTP		21
#define NETWORK_PORT_HTTP		80
#define NETWORK_PORT_HTTPS		443
#define NETWORK_IFNAME_SIZE		16
#define NETWORK_DEV_MAX_COUNT	16

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
#define CAVAN_DISCOVERY_PORT	8888
#define CAVAN_DISCOVERY_COMMAND	"cavan-discovery"

#define CAVAN_NET_MASK				0xAA
#define CAVAN_NET_UDP_RETRY			10
#define CAVAN_NET_UDP_TIMEOUT		200
#define CAVAN_NET_UDP_ACTIVE_TIME	(1000 * 60 * 20)
#define CAVAN_NET_FLAG_TALK			(1 << 0)
#define CAVAN_NET_FLAG_SYNC			(1 << 1)
#define CAVAN_NET_FLAG_WAIT			(1 << 2)
#define CAVAN_NET_FLAG_NODELAY		(1 << 3)
#define CAVAN_NET_FLAG_MASK			(1 << 4)
#define CAVAN_NET_FLAG_PACK			(1 << 5)
#define CAVAN_NET_FLAG_PACK2		(1 << 6)
#define CAVAN_NET_FLAG_BOUND		(1 << 7)
#define CAVAN_NET_FLAG_LINKED		(1 << 8)

#ifndef SO_REUSEPORT
#define SO_REUSEPORT				15
#endif

#define CAVAN_BUILD_IP_ADDR(a, b, c, d) \
	((uint32_t) ((a) << 24 | (b) << 16 | (c) << 8 | (d)))

struct tcp_discovery_client;

#pragma pack(1)
struct mac_header {
	u8 dest_mac[6];
	u8 src_mac[6];
	u16 protocol_type;

	u8 data[0];
};

struct ip_header {
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

struct udp_header {
	u16 src_port;
	u16 dest_port;

	u16 udp_length;
	u16 udp_checksum;

	u8 data[0];
};

struct tcp_header {
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

struct arp_header {
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

struct icmp_header {
	u8 type;
	u8 code;
	u16 checksum;

	u8 data[0];
};

struct ping_header {
	u16 id;
	u16 seq;

	u8 data[0];
};

struct dhcp_header {
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

struct udp_pseudo_header {
	u32 src_ip;
	u32 dest_ip;

	u8 zero;
	u8 protocol;
	u16 udp_length;
};
#pragma pack()

struct cavan_route_node {
	void *data;
	u8 mac_addr[MAC_ADDRESS_LEN];
	u32 ip_addr;
};

struct cavan_route_table {
	struct cavan_route_node **route_table;
	int table_size;
};

struct inet_file_request {
	const char *hostname;
	u16 port;

	union {
		struct {
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

typedef enum {
	NETWORK_PROTOCOL_INVALID = -1,
	NETWORK_PROTOCOL_FTP,
	NETWORK_PROTOCOL_HTTP,
	NETWORK_PROTOCOL_HTTPS,
	NETWORK_PROTOCOL_SSL,
	NETWORK_PROTOCOL_SSL_MASK,
	NETWORK_PROTOCOL_SSL_PACK,
	NETWORK_PROTOCOL_SSL_PACK2,
	NETWORK_PROTOCOL_TCP,
	NETWORK_PROTOCOL_TCP_MASK,
	NETWORK_PROTOCOL_TCP_PACK,
	NETWORK_PROTOCOL_TCP_PACK2,
	NETWORK_PROTOCOL_UDP,
	NETWORK_PROTOCOL_ADB,
	NETWORK_PROTOCOL_ADB_MASK,
	NETWORK_PROTOCOL_ADB_PACK,
	NETWORK_PROTOCOL_ADB_PACK2,
	NETWORK_PROTOCOL_ICMP,
	NETWORK_PROTOCOL_IP,
	NETWORK_PROTOCOL_MAC,
	NETWORK_PROTOCOL_UNIX_TCP,
	NETWORK_PROTOCOL_UNIX_UDP,
	NETWORK_PROTOCOL_LOCAL_TCP,
	NETWORK_PROTOCOL_LOCAL_UDP,
	NETWORK_PROTOCOL_UEVENT,
	NETWORK_PROTOCOL_FILE_RW,
	NETWORK_PROTOCOL_FILE_RO,
	NETWORK_PROTOCOL_FILE_WO,
	NETWORK_PROTOCOL_COUNT,
} network_protocol_t;

struct network_url {
	u16 port;
	const char *protocol;
	const char *hostname;
	const char *pathname;
	char memory[128];
};

struct inet_connect {
	int sockfd;
	struct sockaddr_in addr;
};

typedef enum {
	CAVAN_SYNC_TYPE_ACK,
	CAVAN_SYNC_TYPE_DATA,
} cavan_sync_type_t;

struct cavan_sync_package {
	u8 type;
	u8 index;
	u16 length;
	char data[0];
};

struct network_service;

struct network_client {
	int sockfd;
	socklen_t addrlen;
	void *context;
	void *private_data;
	network_protocol_t type;
	struct network_service *service;

	void (*close)(struct network_client *client);
	void (*shutdown)(struct network_client *client);
	ssize_t (*send_raw)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv_raw)(struct network_client *client, void *buff, size_t size);
	ssize_t (*send)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv)(struct network_client *client, void *buff, size_t size);
	ssize_t (*sendto)(struct network_client *client, const void *buff, size_t size, const struct sockaddr *addr);
	ssize_t (*recvfrom)(struct network_client *client, void *buff, size_t size, struct sockaddr *addr);
};

struct network_client_lock {
	struct network_client *client;
	pthread_mutex_t wr_lock;
	pthread_mutex_t rd_lock;
};

struct network_transmit_data {
	int sender_fd;
	int receiver_fd;
	struct network_client *sender;
	struct network_client *receiver;
};

struct network_client_sync_data {
	pthread_mutex_t lock;
	u8 send_index, recv_index;
	u8 send_pending, recv_pending;

	void (*close)(struct network_client *client);
	ssize_t (*send)(struct network_client *client, const void *buff, size_t size);
	ssize_t (*recv)(struct network_client *client, void *buff, size_t size);
};

#if 0
struct network_client_inet {
	struct network_client client;
	struct sockaddr_in addr;
};

struct network_client_unix {
	struct network_client client;
	struct sockaddr_un addr;
};
#endif

struct network_service {
	int sockfd;
	socklen_t addrlen;
	void *private_data;
	network_protocol_t type;
	cavan_lock_t lock;
	sem_t sem;

	ssize_t (*sendto)(struct network_service *service, const void *buff, size_t size, const struct sockaddr *addr);
	ssize_t (*recvfrom)(struct network_service *service, void *buff, size_t size, struct sockaddr *addr);
	int (*accept_raw)(struct network_service *service, struct network_client *conn, int flags);
	int (*accept)(struct network_service *service, struct network_client *conn, int flags);
	void (*close)(struct network_service *service);
};

struct cavan_udp_client {
	int sockfd;
	struct sockaddr_in addr;
	u32 sequence;
};

struct udp_discovery_service {
	cavan_thread_t thread;
	struct network_service service;
	size_t command_len;
	char command[1024];
	u16 port;
};

struct tcp_discovery_data {
	pthread_t thread;
	struct network_url url;
	struct network_client client;
	struct tcp_discovery_client *discovery;

	int index;
	bool pendding;
};

struct tcp_discovery_client {
	u16 port;
	int count;
	int pendding;
	pthread_mutex_t lock;

	void *private_data;
	bool (*handler)(struct tcp_discovery_client *client, struct tcp_discovery_data *data);
};

struct network_file_request {
	char src_file[1024];
	char dest_file[1024];
	off_t src_offset;
	off_t dest_offset;
	off_t size;
};

struct network_protocol_desc {
	const char *name;
	int flags;
	u16 port;
	network_protocol_t type;

	int (*open_client)(struct network_client *client, const struct network_url *url, u16 port, int flags);
	int (*open_service)(struct network_service *service, const struct network_url *url, u16 port, int flags);
};

struct cavan_inet_ifconfig {
	char if_name[NETWORK_IFNAME_SIZE];
	char if_hwaddr[6];
	bool has_ip;
	struct sockaddr_in if_addr;
	struct sockaddr_in if_dstaddr;
	struct sockaddr_in if_broadaddr;
	struct sockaddr_in if_netmask;
};

struct cavan_inet_route {
	char devname[NETWORK_IFNAME_SIZE];
	struct sockaddr_in dstaddr;
	struct sockaddr_in gateway;
	struct sockaddr_in netmask;
	unsigned int flags;
	unsigned int ref_count;
	unsigned int use;
	unsigned int metric;
	unsigned int mtu;
	unsigned int window;
	unsigned int irtt;
};

__BEGIN_DECLS

const char *network_get_socket_pathname(void);
char *network_get_hostname(char *buff, size_t size);
int cavan_inet_aton(const char *text, struct in_addr *addr);
bool inet_addr_is_broadcast(struct sockaddr_in *addr);
const char *inet_get_special_address(const char *hostname);
const char *inet_check_hostname(const char *hostname, char *buff, size_t size);
ssize_t sendto_select(int sockfd, int retry, const void *buff, size_t len, const struct sockaddr_in *remote_addr);
ssize_t sendto_receive(int sockfd, long timeout, int retry, const void *send_buff, ssize_t sendlen, void *recv_buff, ssize_t recvlen, struct sockaddr_in *remote_addr);

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

void inet_sockaddr_init(struct sockaddr_in *addr, const char *host, u16 port);
int inet_sockaddr_init_url(struct sockaddr_in *addr, const char *url);
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
int inet_bind_dup(int sockfd, int type);

ssize_t inet_send(int sockfd, const char *buff, size_t size);
ssize_t inet_recv(int sockfd, char *buff, size_t size);
bool inet_fill(int sockfd, char *buff, size_t size);
int inet_tcp_send_file1(int sockfd, int fd);
int inet_tcp_send_file2(int sockfd, const char *filename);
int inet_tcp_receive_file1(int sockfd, int fd);
int inet_tcp_receive_file2(int sockfd, const char *filename);
int inet_tcp_transfer(int src_sockfd, int dest_sockfd, size_t size);

int inet_get_sockaddr(int sockfd, const char *devname, struct sockaddr_in *sin_addr);
int inet_get_devname(int sockfd, int index, char *devname);

int inet_tcp_transmit_loop(int src_sockfd, int dest_sockfd);
int inet_hostname2sockaddr(const char *host, struct sockaddr_in *addr);

int inet_create_link(int sockfd, const struct sockaddr_in *addr, int socktype, int protocol);
int network_create_link(int sockfd, const char *hostname, u16 port, int socktype, int protocol);

char *network_sockaddr_tostring(const struct sockaddr *addr, char *buff, size_t size);
void network_url_init(struct network_url *url, const char *protocol, const char *hostname, u16 port, const char *pathname);
char *network_url_get_pathname(const struct network_url *url, char *buff, size_t size);
char *network_url_tostring(const struct network_url *url, char *buff, size_t size, char **tail);
char *network_url_parse(struct network_url *url, const char *text);
int network_url_parse_cmdline(struct network_url *url, struct cavan_dynamic_service *service, int argc, char *argv[]);

const struct network_protocol_desc *network_get_protocol_by_name(const char *name);
const struct network_protocol_desc *network_get_protocol_by_type(network_protocol_t type);
const struct network_protocol_desc *network_get_protocol_by_port(u16 port);
int network_get_port_by_url(const struct network_url *url, const struct network_protocol_desc *protocol);
bool network_url_equals(const struct network_url *url1, const struct network_url *url2);
int network_create_socket_mac(const char *if_name, int protocol);
int network_create_socket_uevent(void);

void network_protocol_dump(void);
network_protocol_t network_protocol_parse(const char *name);
const char *network_protocol_tostring(network_protocol_t type);

int network_client_open(struct network_client *client, const struct network_url *url, int flags);
int network_client_open2(struct network_client *client, const char *url, int flags);
__printf_format_34__ int network_client_openf(struct network_client *client, int flags, const char *url, ...);
int network_client_ssl_attach(struct network_client *client, boolean server);
void network_client_close(struct network_client *client);
void network_client_close_socket(struct network_client *client);
int network_client_vprintf(struct network_client *client, const char *format, va_list ap);
int network_client_printf(struct network_client *client, const char *format, ...);
ssize_t network_client_fill_buff(struct network_client *client, char *buff, size_t size);
int network_client_recv_file(struct network_client *client, int fd, size64_t skip, size64_t size);
int network_client_recv_file2(struct network_client *client, const char *pathname, size64_t size, int flags);
int network_client_send_file(struct network_client *client, int fd, size64_t skip, size64_t size);
int network_client_send_file2(struct network_client *client, const char *pathname, size64_t size);
ssize_t network_client_timed_recv(struct network_client *client, void *buff, size_t size, int timeout);
char *network_client_recv_line(struct network_client *client, char *buff, size_t size);
bool network_client_discard_all(struct network_client *client);
int network_client_exec_redirect(struct network_client *client, int ttyin, int ttyout);
int network_client_exec_main(struct network_client *client, const char *command, int lines, int columns);
int network_client_get_local_port(struct network_client *client);
int network_client_get_remote_port(struct network_client *client);
int network_client_get_local_ip(struct network_client *client, struct in_addr *sin_addr);
int network_client_get_remote_ip(struct network_client *client, struct in_addr *sin_addr);
int network_client_fill(struct network_client *client, char *buff, size_t size);
ssize_t network_client_send_packet(struct network_client *client, const void *buff, size_t size);
ssize_t network_client_recv_packet(struct network_client *client, void *buff, size_t size);
int network_client_fifo_init(struct cavan_fifo *fifo, size_t size, struct network_client *client);
int network_client_fifo_read_cache(struct cavan_fifo *fifo, struct network_client *client);

int network_service_accept_timed(struct network_service *service, struct network_client *client, u32 msec, int flags);
int network_service_open(struct network_service *service, const struct network_url *url, int flags);
int network_service_open2(struct network_service *service, const char *url, int flags);
__printf_format_34__ int network_service_openf(struct network_service *service, int flags, const char *url, ...);
void network_service_close(struct network_service *service);
int network_service_get_local_port(struct network_service *service);

int network_get_devlist(char buff[][NETWORK_IFNAME_SIZE], int max_count);
void cavan_inet_route_dump(const struct cavan_inet_route *route);
int cavan_inet_get_route_table(struct cavan_inet_route *routes, int max_count);
int cavan_inet_find_default_route(struct cavan_inet_route routes[], int count);
int cavan_inet_get_default_route(struct cavan_inet_route *route);
void cavan_inet_ifconfig_dump(const struct cavan_inet_ifconfig *config);
int cavan_inet_get_address(int sockfd, const char *ifname, int command, struct ifreq *ifr, struct sockaddr_in *addr);
int cavan_inet_get_ifconfig(int sockfd, struct ifreq *ifr, struct cavan_inet_ifconfig *config);
int cavan_inet_get_ifconfig2(int sockfd, const char *ifname, struct cavan_inet_ifconfig *config);
int cavan_inet_get_ifconfig_list(int sockfd, struct cavan_inet_ifconfig *configs, int max_count);
int cavan_inet_get_ifconfig_list2(struct cavan_inet_ifconfig *configs, int max_count);

int udp_discovery_service_start(struct udp_discovery_service *service, const char *command, ...);
void udp_discovery_service_stop(struct udp_discovery_service *service);
int udp_discovery_client_run(u16 port, void *data, void (*handler)(int index, const char *command, struct sockaddr_in *addr, void *data));
int tcp_discovery_client_run(struct tcp_discovery_client *client, void *data);

int network_client_lock_init(struct network_client_lock *lock, struct network_client *client);
void network_client_lock_deinit(struct network_client_lock *lock);
struct network_client *network_client_lock_read_acquire(struct network_client_lock *lock);
void network_client_lock_read_release(struct network_client_lock *lock);
struct network_client *network_client_lock_write_acquire(struct network_client_lock *lock);
void network_client_lock_write_release(struct network_client_lock *lock);

static inline int setsockopt_uint(int sockfd, int level, int optname, uint value)
{
	return setsockopt(sockfd, level, optname, (void *) &value, sizeof(value));
}

static inline int setsockopt_timeval(int sockfd, int level, int optname, uint value)
{
	struct timeval time = { (long int) (value / 1000), (long int) (value % 1000 * 1000) };
	return setsockopt(sockfd, level, optname, (void *) &time, sizeof(time));
}

static inline int setsockopt_reuse_addr(int sockfd)
{
	return setsockopt_uint(sockfd, SOL_SOCKET, SO_REUSEADDR, 1);
}

static inline int setsockopt_reuse_port(int sockfd)
{
	return setsockopt_uint(sockfd, SOL_SOCKET, SO_REUSEPORT, 1);
}

static inline int setsockopt_keepalive(int sockfd)
{
	return setsockopt_uint(sockfd, SOL_SOCKET, SO_KEEPALIVE, 1);
}

static inline int setsockopt_tcp_nodelay(int sockfd)
{
	return setsockopt_uint(sockfd, IPPROTO_TCP, TCP_NODELAY, 1);
}

static inline int setsockopt_send_timeout(int sockfd, u32 mseconds)
{
	return setsockopt_timeval(sockfd, SOL_SOCKET, SO_SNDTIMEO, mseconds);
}

static inline int setsockopt_recv_timeout(int sockfd, u32 mseconds)
{
	return setsockopt_timeval(sockfd, SOL_SOCKET, SO_RCVTIMEO, mseconds);
}

static inline int inet_socket(int type)
{
	return socket(PF_INET, type, 0);
}

static inline int tcp_socket(void)
{
	return inet_socket(SOCK_STREAM);
}

static inline int udp_socket(void)
{
	return inet_socket(SOCK_DGRAM);
}

static inline int unix_socket(int type)
{
	return socket(PF_UNIX, type, 0);
}

static inline int unix_tcp_socket(void)
{
	return unix_socket(SOCK_STREAM);
}

static inline int unix_udp_socket(void)
{
	return unix_socket(SOCK_DGRAM);
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

static inline ssize_t inet_recvfrom(int sockfd, void *buff, size_t size, struct sockaddr_in *addr)
{
	socklen_t addrlen = sizeof(*addr);
	return recvfrom(sockfd, buff, size, 0, (struct sockaddr *) addr, &addrlen);
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

static inline ssize_t inet_recv2(int sockfd, void *buff, size_t size)
{
	return recv(sockfd, buff, size, 0);
}

static inline ssize_t inet_recv_timeout(int sockfd, void *buff, size_t size, int timeout_ms)
{
	if (file_poll_input(sockfd, timeout_ms)) {
		return inet_recv(sockfd, (char *) buff, size);
	}

	return -ETIMEDOUT;
}

static inline ssize_t inet_recvfrom_timeout(int sockfd, void *buff, size_t size, struct sockaddr_in *addr, int timeout_ms)
{
	if (file_poll_input(sockfd, timeout_ms)) {
		return inet_recvfrom(sockfd, (char *) buff, size, addr);
	}

	errno = ETIMEDOUT;

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
	return network_client_recv((struct network_client *) fifo->private_data, buff, size);
}

static inline ssize_t network_client_fifo_write(struct cavan_fifo *fifo, const void *buff, size_t size)
{
	return network_client_send((struct network_client *) fifo->private_data, buff, size);
}

static inline int network_client_get_send_timeout(struct network_client *client, struct timeval *tv)
{
	socklen_t len = sizeof(struct timeval);
	return getsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, tv, &len);
}

static inline int network_client_set_send_timeout(struct network_client *client, const struct timeval *tv)
{
	return setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, tv, sizeof(struct timeval));
}

static inline int network_client_get_recv_timeout(struct network_client *client, struct timeval *tv)
{
	socklen_t len = sizeof(struct timeval);
	return getsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, tv, &len);
}

static inline int network_client_set_recv_timeout(struct network_client *client, const struct timeval *tv)
{
	return setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof(struct timeval));
}

static inline void network_client_shutdown(struct network_client *client)
{
	client->shutdown(client);
}

static inline int network_client_set_keepalive(struct network_client *client)
{
	return setsockopt_keepalive(client->sockfd);
}

static inline int network_client_set_tcp_nodelay(struct network_client *client)
{
	return setsockopt_tcp_nodelay(client->sockfd);
}

static inline void network_service_set_data(struct network_service *service, void *data)
{
	service->private_data = data;
}

static inline void *network_service_get_data(struct network_service *service)
{
	return service->private_data;
}

static inline int network_service_accept(struct network_service *service, struct network_client *client, int flags)
{
	return service->accept(service, client, flags);
}

static inline int network_service_get_local_addr(struct network_service *service, struct sockaddr *addr, socklen_t addrlen)
{
	return getsockname(service->sockfd, addr, &addrlen);
}

static inline void network_service_lock(struct network_service *service)
{
	cavan_lock_acquire(&service->lock);
}

static inline void network_service_unlock(struct network_service *service)
{
	cavan_lock_release(&service->lock);
}

static inline const char *cavan_get_server_hostname(void)
{
	return cavan_getenv(CAVAN_IP_ENV_NAME, CAVAN_DEFAULT_IP);
}

static inline u16 cavan_get_server_port(u16 default_value)
{
	return cavan_getenv_u32(CAVAN_PORT_ENV_NAME, default_value);
}

static inline int network_client_send_file3(struct network_client *client, const char *pathname)
{
	size64_t size = file_get_size(pathname);
	return network_client_send_file2(client, pathname, size);
}

static inline bool network_client_readable(struct network_client *client)
{
	return file_poll_input(client->sockfd, 0);
}

static inline bool network_service_readable(struct network_service *service)
{
	return file_poll_input(service->sockfd, 0);
}

__END_DECLS
