/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/tcp_tftp.h>
#include <cavan/service.h>

static int tcp_tftp_send_ack(int sockfd, struct stat *st)
{
	struct tcp_tftp_ack ack;
	size_t sendlen;

	if (st)
	{
		ack.st = *st;
		sendlen = sizeof(ack);
	}
	else
	{
		sendlen = 2;
	}

	ack.pkg_type = TCP_TFTP_EVENT_ACK;

	return inet_send(sockfd, &ack, sendlen);
}

static int tcp_tftp_send_error_message(int sockfd, const char *fmt, ...)
{
	struct tcp_tftp_error_message msg;
	va_list ap;
	size_t ret;

	msg.pkg_type = TCP_TFTP_EVENT_ERROR;
	msg.err_code = errno;

	va_start(ap, fmt);
	ret = vsprintf(msg.message, fmt, ap);
	va_end(ap);

	return inet_send(sockfd, &msg, ret + 6);
}

static int tcp_tftp_symlink(int sockfd, const char *pathname)
{
	char buff[1024];
	ssize_t recvlen;
	int ret;

	tcp_tftp_send_ack(sockfd, NULL);

	recvlen = inet_recv(sockfd, buff, sizeof(buff));
	if (recvlen < 0)
	{
		print_error("inet_recv");
		return recvlen;
	}

	ret = symlink(buff, pathname);
	if (ret < 0)
	{
		tcp_tftp_send_error_message(sockfd, "Create symlink %s failed", pathname);
		return ret;
	}

	tcp_tftp_send_ack(sockfd, NULL);

	return 0;
}

static int tcp_tftp_mkdir(int sockfd, const char *pathname, mode_t mode)
{
	int ret;

	ret = mkdir_hierarchy(pathname, mode);
	if (ret < 0)
	{
		tcp_tftp_send_error_message(sockfd, "Create directory %s failed", pathname);
		return ret;
	}

	tcp_tftp_send_ack(sockfd, NULL);

	return 0;
}

static int tcp_tftp_recv_file(int sockfd, const char *pathname, size_t size, mode_t mode)
{
	int fd;

	fd = open(pathname, O_WRONLY | O_TRUNC | O_CREAT, mode);
	if (fd < 0)
	{
		tcp_tftp_send_error_message(sockfd, "server open file %s failed", pathname);
		return fd;
	}

	tcp_tftp_send_ack(sockfd, NULL);

	return ffile_ncopy(sockfd, fd, size);
}

static int tcp_tftp_handle_write_request(int sockfd, struct tcp_tftp_write_request *req)
{
	switch (req->st.st_mode & S_IFMT)
	{
	case S_IFREG:
		return tcp_tftp_recv_file(sockfd, req->pathname, req->st.st_size, req->st.st_mode);

	case S_IFLNK:
		return tcp_tftp_symlink(sockfd, req->pathname);

	case S_IFDIR:
		return tcp_tftp_mkdir(sockfd, req->pathname, req->st.st_mode);
	}

	return 0;
}

static int tcp_tftp_handle_read_request(int sockfd, struct tcp_tftp_read_request *req)
{
	return 0;
}

static int tcp_tftp_server_handle_request(int sockfd)
{
	ssize_t recvlen;
	union tcp_tftp_package pkg;

	recvlen = inet_recv(sockfd, &pkg, sizeof(pkg));
	if (recvlen < 0)
	{
		print_error("inet_recv");
		return recvlen;
	}

	switch (pkg.pkg_type)
	{
	case TCP_TFTP_REQ_WRITE:
		pr_bold_info("TCP_TFTP_REQ_WRITE");
		return tcp_tftp_handle_write_request(sockfd, &pkg.write_req);

	case TCP_TFTP_REQ_READ:
		pr_bold_info("TCP_TFTP_REQ_READ");
		return tcp_tftp_handle_read_request(sockfd, &pkg.read_req);

	default:
		pr_red_info("invalid request %d", pkg.pkg_type);
		return -EINVAL;
	}

	return 0;
}

static int tcp_tftp_daemon_handle(int index, void *data)
{
	int server_sockfd, client_sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;

	server_sockfd = (int)data;

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	pr_bold_info("IP = %s, port = %d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	tcp_tftp_server_handle_request(client_sockfd);
	close(client_sockfd);

	return 0;
}

int tcp_tftp_service_run(u16 port)
{
	int ret;
	int sockfd;
	struct cavan_service_description desc =
	{
		.name = "TCP_TFTP",
		.daemon_count = TCP_TFTP_DAEMON_COUNT,
		.as_daemon = 0,
		.show_verbose = 0,
		.handler = tcp_tftp_daemon_handle
	};

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		print_error("inet_create_tcp_service");
		return sockfd;
	}

	ret = cavan_service_run(&desc);
	cavan_service_stop(&desc);
	close(sockfd);

	return ret;
}

void show_tcp_tftp_error_message(struct tcp_tftp_error_message *msg)
{
	if (msg->err_code)
	{
		pr_red_info("%s: %s", msg->message, strerror(msg->err_code));
	}
	else
	{
		pr_red_info("%s", msg->message);
	}
}

int tcp_tftp_send_write_request(int sockfd, struct tcp_tftp_write_request *req, size_t size)
{
	ssize_t sendlen, recvlen;
	union tcp_tftp_package pkg;

	req->pkg_type = TCP_TFTP_REQ_WRITE;

	sendlen = inet_send(sockfd, req, size);
	if (sendlen < 0)
	{
		print_error("inet send");
		return sendlen;
	}

	recvlen = inet_recv_timeout(sockfd, &pkg, sizeof(pkg), TCP_TFTP_TIMEOUT);
	if (recvlen < 0)
	{
		print_error("inet_recv_timeout");
		return recvlen;
	}

	switch (pkg.pkg_type)
	{
	case TCP_TFTP_EVENT_ACK:
		return 0;

	case TCP_TFTP_EVENT_ERROR:
		show_tcp_tftp_error_message(&pkg.err_msg);
		return -1;

	default:
		pr_red_info("unknown package type %d", pkg.pkg_type);
		return -1;
	}

	return 0;
}

int tcp_tftp_send_file(const char *inpath, const char *outpath, const char *ip, u16 port)
{
	int ret;
	int sockfd;
	int fd;
	struct tcp_tftp_write_request req;

	ret = lstat(inpath, &req.st);
	if (ret < 0)
	{
		print_error("stat %s", inpath);
		return ret;
	}

	sockfd = inet_create_tcp_link2(ip, port);
	if (sockfd < 0)
	{
		print_error("inet_create_tcp_link2");
		return sockfd;
	}

	ret = text_copy(req.pathname, outpath) - (char *)&req;
	ret = tcp_tftp_send_write_request(sockfd, &req, ret + 1);
	if (ret < 0)
	{
		print_error("tcp_tftp_send_write_request");
		goto out_close_sockfd;
	}

	switch (req.st.st_mode & S_IFMT)
	{
	case S_IFREG:
		fd = open(inpath, O_RDONLY);
		if (fd < 0)
		{
			print_error("open %s failed", inpath);
			ret = fd;
			goto out_close_sockfd;
		}

		ret = ffile_ncopy(fd, sockfd, req.st.st_size);
		if (ret < 0)
		{
			print_error("ffile_ncopy");
		}
		break;

	case S_IFLNK:
		break;

	default:
		ret = 0;
	}

out_close_sockfd:
	close(sockfd);

	return ret;
}
