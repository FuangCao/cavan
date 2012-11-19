// Fuang.Cao <cavan.cfa@gmail.com> Wed Sep 14 18:08:27 CST 2011

#include <cavan.h>
#include <cavan/usb.h>
#include <pthread.h>
#include <cavan/network.h>
#include <cavan/net_bridge.h>

#define FILE_CREATE_DATE 		"Wed Sep 14 18:08:27 CST 2011"
#define DEVICE_ADB_ENABLE_PATH	"/dev/android_adb_enable"
#define DEVICE_ADB_PATH			"/dev/android_adb"
#define DEVICE_SWAN_VN_DATA		"/dev/swan_vn"
#define DEFAULT_NET_DEVICE		"eth0"

struct swan_vn_client_data
{
	int fd_data;
	int fd_adb;
	pthread_cond_t notify;
	pthread_mutex_t lock;
};

struct swan_vn_server_data
{
	struct cavan_net_bridge_descriptor *nb_desc;
	struct cavan_usb_descriptor *usb_desc;
	pthread_cond_t notify;
	pthread_mutex_t lock;
};

static u8 board_mac[MAC_ADDRESS_LEN];
static u32 board_ip;

static void show_usage(void)
{
	println("Usage:");
}

static void close_adb_device(struct swan_vn_client_data *client_data)
{
	pthread_mutex_lock(&client_data->lock);
	close(client_data->fd_adb);
	client_data->fd_adb = -1;
	pthread_cond_signal(&client_data->notify);
	pthread_mutex_unlock(&client_data->lock);
}

static void *client_send_handle(void *data)
{
	struct swan_vn_client_data *client_data;
	int fd_data, fd_adb;
	ssize_t readlen, writelen;
	char buff[4096];

	client_data = data;
	fd_data = client_data->fd_data;
	fd_adb = client_data->fd_adb;

	while (1)
	{
		readlen = read(fd_data, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read data");
			break;
		}

		writelen = cavan_adb_write_data(fd_adb, buff, readlen);
		if (writelen < 0)
		{
			print_error("write adb");
			close_adb_device(client_data);
			break;
		}
	}

	return NULL;
}

static void *client_recv_handle(void *data)
{
	struct swan_vn_client_data *client_data;
	int fd_data, fd_adb;
	ssize_t readlen, writelen;
	char buff[4096];

	client_data = data;
	fd_data = client_data->fd_data;
	fd_adb = client_data->fd_adb;

	while (1)
	{
		readlen = cavan_adb_read_data(fd_adb, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read adb");
			close_adb_device(client_data);
			break;
		}

		writelen = write(fd_data, buff, readlen);
		if (writelen < 0)
		{
			print_error("write data");
			break;
		}
	}

	return NULL;
}

static int run_client_thead(const char *data_path)
{
	int ret;
	int fd_data, fd_adb_en;
	struct swan_vn_client_data client_data;
	pthread_t thread_send, thread_recv;

	fd_adb_en = open(DEVICE_ADB_ENABLE_PATH, O_RDWR);
	if (fd_adb_en < 0)
	{
		print_error("open file \"%s\" failed", DEVICE_ADB_ENABLE_PATH);
		return fd_adb_en;
	}

	fcntl(fd_adb_en, F_SETFD, FD_CLOEXEC);

	fd_data = open(data_path, O_RDWR);
	if (fd_data < 0)
	{
		print_error("open file \"%s\" failed", data_path);
		goto out_close_adb_en;
	}

	client_data.fd_data = fd_data;
	pthread_cond_init(&client_data.notify, 0);
	pthread_mutex_init(&client_data.lock, 0);

	client_data.fd_adb = -1;
	thread_send = thread_recv = 0;

	while (1)
	{
		pthread_mutex_lock(&client_data.lock);
		while (client_data.fd_adb > 0)
		{
			pthread_cond_wait(&client_data.notify, &client_data.lock);
		}
		pthread_mutex_unlock(&client_data.lock);

#ifndef CONFIG_BUILD_FOR_ANDROID
		if (thread_send)
		{
			pthread_cancel(thread_send);
		}

		if (thread_recv)
		{
			pthread_cancel(thread_recv);
		}
#endif

		while (1)
		{
			client_data.fd_adb = open(DEVICE_ADB_PATH, O_RDWR);
			if ((client_data.fd_adb < 0))
			{
				pr_red_info("open device \"%s\" failed", DEVICE_ADB_PATH);
				msleep(100);
			}
			else
			{
				break;
			}
		}

		fcntl(client_data.fd_adb, F_SETFD, FD_CLOEXEC);

		ret = pthread_create(&thread_send, NULL, client_send_handle, &client_data);
		if (ret < 0)
		{
			print_error("pthread_create");
			goto out_close_data;
		}

		ret = pthread_create(&thread_recv, NULL, client_recv_handle, &client_data);
		if (ret < 0)
		{
			print_error("pthread_create");
			goto out_close_data;
		}
	}

out_close_data:
	close(fd_data);
out_close_adb_en:
	close(fd_adb_en);

	return -1;
}

static void server_exit(struct swan_vn_server_data *server_data)
{
	pthread_mutex_lock(&server_data->lock);
	pthread_cond_broadcast(&server_data->notify);
	pthread_mutex_unlock(&server_data->lock);
}

static void *server_send_handle(void *data)
{
	struct swan_vn_server_data *server_data = data;
	struct cavan_usb_descriptor *usb_desc = server_data->usb_desc;
	struct cavan_net_bridge_descriptor *nb_desc = server_data->nb_desc;
	char buff[4096];
	struct mac_header *mac_hdr = (void *)buff;
	ssize_t readlen, writelen;
	int sockfd = nb_desc->sockfd;
	u8 *host_mac = nb_desc->host_hwaddr;
	u8 *src_mac = mac_hdr->src_mac;
	struct arp_header *arp_hdr = (void *)(mac_hdr + 1);
	struct ip_header *ip_hdr = (void *)(mac_hdr + 1);
	struct udp_header *udp_hdr = (void *)(ip_hdr + 1);
	u32 host_ip;

	host_ip = *(u32 *)&nb_desc->host_ifaddr.sin_addr;

	while (1)
	{
		readlen = cavan_usb_read_data(usb_desc, buff, sizeof(buff));
		if (readlen < 0)
		{
#if __WORDSIZE == 64
			print_error("cavan_usb_read_data, readlen = %ld", readlen);
#else
			print_error("cavan_usb_read_data, readlen = %d", readlen);
#endif
			break;
		}

		switch (ntohs(mac_hdr->protocol_type))
		{
		case ETH_P_ARP:
		case ETH_P_RARP:
			board_ip = arp_hdr->src_ip;
			memcpy(board_mac, arp_hdr->src_mac, MAC_ADDRESS_LEN);

			if (host_ip == arp_hdr->dest_ip)
			{
				break;
			}

			memcpy(arp_hdr->src_mac, host_mac, MAC_ADDRESS_LEN);
			memcpy(src_mac, host_mac, MAC_ADDRESS_LEN);
			break;

		case ETH_P_IP:
			board_ip = ip_hdr->src_ip;

			if (ip_hdr->protocol_type == IPPROTO_UDP && udp_hdr->src_port == htons(68))
			{
				memcpy(board_mac, mac_hdr->src_mac, MAC_ADDRESS_LEN);
			}
			break;

		default:
			continue;
		}

		writelen = write(sockfd, buff, readlen);
		if (writelen < 0)
		{
			print_error("write");
			break;
		}
	}

	server_exit(server_data);

	return NULL;
}

static void *server_recv_handle(void *data)
{
	struct swan_vn_server_data *server_data = data;
	struct cavan_net_bridge_descriptor *nb_desc = server_data->nb_desc;
	struct cavan_usb_descriptor *usb_desc = server_data->usb_desc;
	int sockfd = nb_desc->sockfd;
	ssize_t readlen, writelen;
	char buff[4096];
	struct mac_header *mac_hdr = (void *)buff;
	struct ip_header *ip_hdr = (void *)(mac_hdr + 1);
	struct arp_header *arp_hdr = (void *)(mac_hdr + 1);
	u8 *dest_mac = mac_hdr->dest_mac;

	while (1)
	{
		readlen = read(sockfd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			break;
		}

		switch (ntohs(mac_hdr->protocol_type))
		{
		case ETH_P_IP:
			if (board_ip && ip_hdr->dest_ip != board_ip)
			{
				continue;
			}
			break;

		case ETH_P_ARP:
		case ETH_P_RARP:
			if (arp_hdr->dest_ip != board_ip)
			{
				continue;
			}

			memcpy(arp_hdr->dest_mac, board_mac, MAC_ADDRESS_LEN);
			break;

		default:
			continue;
		}

		memcpy(dest_mac, board_mac, MAC_ADDRESS_LEN);

		writelen = cavan_usb_write_data(usb_desc, buff, readlen);
		if (writelen < 0)
		{
			pr_red_info("cavan_usb_write_data");
			break;
		}
	}

	server_exit(server_data);

	return NULL;
}

static int run_server_thead(const char *net_dev, const char *usb_dev)
{
	int ret;
	struct cavan_net_bridge_descriptor nb_desc;
	struct cavan_usb_descriptor usb_desc;
	struct swan_vn_server_data server_data;
	pthread_t thread_send, thread_recv;

	ret = cavan_find_usb_device(usb_dev, &usb_desc);
	if (ret < 0)
	{
		pr_red_info("cavan_usb_init");
		return ret;
	}

	pr_bold_info("device path = %s", usb_desc.dev_path);
	pr_bold_info("serial = %s", usb_desc.serial);

	ret = cavan_net_bridge_init(&nb_desc, net_dev);
	if (ret < 0)
	{
		pr_red_info("cavan_net_bridge_init");
		goto out_usb_uninit;
	}

	pr_bold_info("host hwaddr = %s", mac_address_tostring((char *)nb_desc.host_hwaddr, MAC_ADDRESS_LEN));
	pr_bold_info("host ipaddr = %s", inet_ntoa(nb_desc.host_ifaddr.sin_addr));

	server_data.nb_desc = &nb_desc;
	server_data.usb_desc = &usb_desc;
	pthread_mutex_init(&server_data.lock, 0);
	pthread_cond_init(&server_data.notify, 0);

	ret = pthread_create(&thread_send, NULL, server_send_handle, &server_data);
	if (ret < 0)
	{
		print_error("pthread_create");
		goto out_net_bridge_uninit;
	}

	ret = pthread_create(&thread_recv, NULL, server_recv_handle, &server_data);
	if (ret < 0)
	{
		print_error("pthread_create");
		goto out_net_bridge_uninit;
	}

	pthread_mutex_lock(&server_data.lock);
	pthread_cond_wait(&server_data.notify, &server_data.lock);
	pthread_mutex_unlock(&server_data.lock);

out_net_bridge_uninit:
	cavan_net_bridge_uninit(&nb_desc);
out_usb_uninit:
	cavan_usb_uninit(&usb_desc);

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

	if (access(DEVICE_SWAN_VN_DATA, F_OK) < 0)
	{
		switch (argc - optind)
		{
		case 0:
			return run_server_thead(DEFAULT_NET_DEVICE, NULL);

		case 1:
			return run_server_thead(argv[optind], NULL);

		default:
			return run_server_thead(argv[optind], argv[optind + 1]);
		}
	}
	else
	{
		if (optind < argc)
		{
			return run_client_thead(argv[optind]);
		}
		else
		{
			return run_client_thead(DEVICE_SWAN_VN_DATA);
		}
	}

	return 0;
}
