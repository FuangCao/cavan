// Fuang.Cao <cavan.cfa@gmail.com> Thu Mar 31 12:05:55 CST 2011

#include <cavan.h>
#include <cavan/tftp.h>
#include <cavan/progress.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>
#include <semaphore.h>
#include <cavan/parser.h>
#include <cavan/service.h>
#include <cavan/permission.h>

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

static int service_handle(int index, cavan_shared_data_t data)
{
	int ret;
	int sockfd = data.type_int;
	union tftp_pkg pkg;
	struct tftp_request_pkg *req_pkg_p = (void *)&pkg;
	struct tftp_dd_request_pkg *dd_req_pkg_p = (void *)&pkg;
	struct tftp_request req;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	ret = inet_recvfrom(sockfd, req_pkg_p, sizeof(*req_pkg_p), &req.client_addr, &addr_len);
	if (ret < 0)
	{
		print_error("Receive request failed");
		return ret;
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
		return -EINVAL;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int sockfd;
	u16 server_port = 0;
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
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 's',
		},
		{
		},
	};
	int c;
	int option_index;
	struct cavan_service_description desc =
	{
		.name = "TFTP_DD",
		.daemon_count = TFTP_MAX_LINK_COUNT,
		.as_daemon = 0,
		.show_verbose = 0,
		.super_permission = 1,
		.handler = service_handle
	};

	while ((c = getopt_long(argc, argv, "p:P:dDvVs:S:", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'p':
		case 'P':
			server_port = text2value(optarg, NULL, 10);
			break;

		case 'd':
		case 'D':
			desc.as_daemon = 1;
			break;

		case 'v':
		case 'V':
			desc.show_verbose = 1;
			break;

		case 's':
		case 'S':
			desc.super_permission = text_bool_value(optarg);
			break;

		default:
			error_msg("illegal option");
			return -EINVAL;
		}
	}

	if (server_port == 0)
	{
		server_port = cavan_get_server_port(TFTP_DD_DEFAULT_PORT);
	}

	sockfd = inet_create_udp_service(server_port);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	println("Bind socket to port \"%d\" success", server_port);

	desc.data.type_int = sockfd;
	ret = cavan_service_run(&desc);
	cavan_service_stop(&desc);
	close(sockfd);

	return ret;
}
