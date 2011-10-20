// Fuang.Cao <cavan.cfa@gmail.com> Thu Mar 31 12:05:55 CST 2011

#include <cavan.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cavan/tftp.h>
#include <cavan/progress.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>
#include <semaphore.h>
#include <cavan/parser.h>

static int handle_read_request(struct tftp_request *req_p)
{
	int ret;

	ret = tftp_service_send_data(req_p->filename, req_p->offset, req_p->size, req_p->file_mode, &req_p->client_addr);
	if (ret < 0)
	{
		println("Send file \"%s\" failed", req_p->filename);
	}
	else
	{
		println("Send file \"%s\" success", req_p->filename);
	}

	return ret;
}

static int handle_write_request(struct tftp_request *req_p)
{
	int ret;

	ret = tftp_service_receive_data(req_p->filename, req_p->offset, req_p->file_mode, &req_p->client_addr);
	if (ret < 0)
	{
		println("Receive file \"%s\" failed", req_p->filename);
	}
	else
	{
		println("Receive file \"%s\" success", req_p->filename);
	}

	return ret;
}

static pthread_mutex_t tftp_mutex;
static int service_count = 0;
static void *service_handle(void *arg)
{
	int ret;
	int sockfd = (int)arg;
	union tftp_pkg pkg;
	struct tftp_request_pkg *req_pkg_p = (void *)&pkg;
	struct tftp_dd_request_pkg *dd_req_pkg_p = (void *)&pkg;
	struct tftp_request req;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int index;

	pthread_mutex_lock(&tftp_mutex);
	index = ++service_count;
	pthread_mutex_unlock(&tftp_mutex);

	while (1)
	{
		println("Service %d ready", index);

		ret = recvfrom(sockfd, req_pkg_p, sizeof(*req_pkg_p), 0, (struct sockaddr *)&req.client_addr, &addr_len);
		if (ret < 0)
		{
			print_error("Receive request failed");
			return NULL;
		}

		println("Received a request client IP = %s, client port = %d",
			inet_ntoa(req.client_addr.sin_addr), ntohs(req.client_addr.sin_port));
		println("Service index = %d", index);

		switch (ntohs(req_pkg_p->op_code))
		{
		case TFTP_RRQ:
			println("=> General read request");
			strcpy(req.filename, req_pkg_p->filename);
			strcpy(req.file_mode, req_pkg_p->filename + strlen(req_pkg_p->filename) + 1);
			req.offset = 0;
			req.size = 0;
			handle_read_request(&req);
			break;

		case TFTP_DD_RRQ:
			println("=> DD read request");
			strcpy(req.filename, dd_req_pkg_p->filename);
			strcpy(req.file_mode, dd_req_pkg_p->filename + strlen(dd_req_pkg_p->filename) + 1);
			req.offset = ntohl(dd_req_pkg_p->offset);
			req.size = ntohl(dd_req_pkg_p->size);
			handle_read_request(&req);
			break;

		case TFTP_WRQ:
			println("=> General write request");
			strcpy(req.filename, req_pkg_p->filename);
			strcpy(req.file_mode, req_pkg_p->filename + strlen(req_pkg_p->filename) + 1);
			req.offset = 0;
			req.size = 0;
			handle_write_request(&req);
			break;

		case TFTP_DD_WRQ:
			println("=> DD write request");
			strcpy(req.filename, dd_req_pkg_p->filename);
			strcpy(req.file_mode, dd_req_pkg_p->filename + strlen(dd_req_pkg_p->filename) + 1);
			req.offset = ntohl(dd_req_pkg_p->offset);
			req.size = ntohl(dd_req_pkg_p->size);
			handle_write_request(&req);
			break;

		case TFTP_MKDIR_REQ:
			println("=> mkdir request");
			tftp_mkdir(&pkg.mkdir, &req.client_addr);
			break;

		case TFTP_MKNODE_REQ:
			println("=> mknode request");
			tftp_mknode(&pkg.mknode, &req.client_addr);
			break;

		case TFTP_SYMLINK_REQ:
			println("=> symlink request");
			tftp_symlink(&pkg.symlink, &req.client_addr);
			break;

		case TFTP_COMMAND_REQ:
			println("=> command request");
			tftp_command(&pkg.command, &req.client_addr);
			break;

		default:
			warning_msg("unknown operation code");
		}
	}
}

int main(int argc, char *argv[])
{
	int ret;
	int i;
	int sockfd;
	struct sockaddr_in server_addr;
	pthread_t newthreads[TFTP_MAX_LINK_COUNT - 1];
	u16 server_port = TFTP_DEFAULT_PORT;
	struct option long_options[] =
	{
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
		},
	};
	int c;
	int option_index;
	int as_daemon, show_verbose;

	as_daemon = 0;
	show_verbose = 0;

	while ((c = getopt_long(argc, argv, "p:P:dDvV", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'p':
		case 'P':
			server_port = text2value(optarg, NULL, 10);
			break;

		case 'd':
		case 'D':
			as_daemon = 1;
			break;

		case 'v':
		case 'V':
			show_verbose = 1;
			break;

		default:
			error_msg("illegal option");
			return -EINVAL;
		}
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

	ret = bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		print_error("Bind socket failed");
		goto out_close_socket;
	}

	println("Bind socket to port \"%d\" success", server_port);

	if (as_daemon)
	{
		ret = daemon(0, show_verbose);
		if (ret < 0)
		{
			print_error("daemon");
			goto out_close_socket;
		}
	}

	pthread_mutex_init(&tftp_mutex, NULL);

	for (i = 0; i < ARRAY_SIZE(newthreads); i++)
	{
		ret = pthread_create(newthreads + i, NULL, service_handle, (void *)sockfd);
		if (ret < 0)
		{
			error_msg("create thread %d failed", i);
		}
	}

	service_handle((void *)sockfd);

	for (i = 0; i < ARRAY_SIZE(newthreads); i++)
	{
		pthread_join(newthreads[i], NULL);
	}

	ret = 0;
out_close_socket:
	close(sockfd);
	return ret;
}
