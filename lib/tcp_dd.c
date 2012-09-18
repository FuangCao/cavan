/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/tcp_dd.h>
#include <cavan/service.h>
#include <cavan/device.h>

static void tcp_dd_show_response(struct tcp_dd_response_package *res)
{
	if (res->message[0] == 0)
	{
		return;
	}

	if (res->code < 0)
	{
		if (res->number)
		{
			pr_red_info("%s [%s]", res->message, strerror(res->number));
		}
		else
		{
			pr_red_info("%s", res->message);
		}
	}
	else
	{
		pr_green_info("%s", res->message);
	}
}

static int __printf_format_34__ tcp_dd_send_response(int sockfd, int code, const char *fmt, ...)
{
	struct tcp_dd_package pkg;
	int ret;

	pkg.type = TCP_DD_RESPONSE;
	pkg.body.res_pkg.code = code;
	pkg.body.res_pkg.number = errno;

	if (fmt == NULL)
	{
		pkg.body.res_pkg.message[0] = 0;
		ret = 1;
	}
	else
	{
		va_list ap;

		va_start(ap, fmt);
		ret = vsprintf(pkg.body.res_pkg.message, fmt, ap) + 1;
		va_end(ap);
	}

	tcp_dd_show_response(&pkg.body.res_pkg);

	return inet_send(sockfd, &pkg, sizeof(pkg.type) + (sizeof(int) * 2) + ret);
}

static int tcp_dd_send_read_request(int sockfd, const char *filename, off_t offset, off_t size, struct tcp_dd_package *pkg)
{
	int ret;

	pkg->type = TCP_DD_READ;
	pkg->body.file_req.offset = offset;
	pkg->body.file_req.size = size;
	ret = text_copy(pkg->body.file_req.filename, filename) - (char *)&pkg + 1;

	ret = inet_send(sockfd, pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	ret = inet_recv(sockfd, pkg, sizeof(*pkg));
	if (ret < 0)
	{
		pr_red_info("inet_recv");
		return ret;
	}

	switch (pkg->type)
	{
	case TCP_DD_RESPONSE:
		tcp_dd_show_response(&pkg->body.res_pkg);
		return pkg->body.res_pkg.code;

	case TCP_DD_WRITE:
		return 0;

	default:
		return -EINVAL;
	}
}

static int tcp_dd_send_write_request(int sockfd, const char *filename, off_t offset, off_t size, mode_t mode)
{
	int ret;
	struct tcp_dd_package pkg;

	pkg.type = TCP_DD_WRITE;
	pkg.body.file_req.offset = offset;
	pkg.body.file_req.size = size;
	pkg.body.file_req.mode = mode;

	ret = text_copy(pkg.body.file_req.filename, filename) - (char *)&pkg + 1;
	ret = inet_send(sockfd, &pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	ret = inet_recv(sockfd, &pkg, sizeof(pkg));
	if (ret < 0)
	{
		pr_red_info("inet_recv");
		return ret;
	}

	if (pkg.type == TCP_DD_RESPONSE)
	{
		tcp_dd_show_response(&pkg.body.res_pkg);
		return pkg.body.res_pkg.code;
	}
	else
	{
		return -EINVAL;
	}
}

static int tcp_dd_handle_read_request(int sockfd, struct tcp_dd_file_request *req)
{
	int fd;
	int ret;
	off_t size;
	mode_t mode;

	fd = open(req->filename, O_RDONLY);
	if (fd < 0)
	{
		tcp_dd_send_response(sockfd, fd, "[Server] Open file `%s' failed", req->filename);
		return fd;
	}

	if (req->size == 0)
	{
		size = ffile_get_size(fd);
	}
	else
	{
		size = req->size;
	}

	if (size < req->offset)
	{
		ret = -EINVAL;
		tcp_dd_send_response(sockfd, ret, "[Server] No data to be sent");
		goto out_close_fd;
	}

	ret = lseek(fd, req->offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(sockfd, ret, "[Server] Seek file `%s' failed", req->filename);
		goto out_close_fd;
	}

	size -= req->offset;

	mode = ffile_get_mode(fd);
	if (mode == 0)
	{
		ret = -EFAULT;
		tcp_dd_send_response(sockfd, ret, "[Server] Get file `%s' mode failed", req->filename);
		goto out_close_fd;
	}

	ret = tcp_dd_send_write_request(sockfd, req->filename, req->offset, size, mode);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_write_request");
		return ret;
	}

	println("filename = %s", req->filename);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(size));
	ret = ffile_ncopy(fd, sockfd, size);

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_write_request(int sockfd, struct tcp_dd_file_request *req)
{
	int fd;
	int ret;

	if (file_test(req->filename, "b") == 0)
	{
		umount_device(req->filename, MNT_DETACH);
	}

	fd = open(req->filename, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, req->mode);
	if (fd < 0)
	{
		tcp_dd_send_response(sockfd, fd, "[Server] Open file `%s' failed", req->filename);
		return fd;
	}

	ret = lseek(fd, req->offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(sockfd, ret, "[Server] Seek file failed");
		goto out_close_fd;
	}

	ret = tcp_dd_send_response(sockfd, 0, "[Server] Start receive file");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	println("filename = %s", req->filename);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(req->size));
	ret = ffile_ncopy(sockfd, fd, req->size);

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_request(int sockfd)
{
	int ret;
	struct tcp_dd_package pkg;

	ret = inet_recv(sockfd, &pkg, sizeof(pkg));
	if (ret < 0)
	{
		pr_red_info("inet_recv");
		return ret;
	}

	switch (pkg.type)
	{
	case TCP_DD_READ:
		pr_bold_info("TCP_DD_READ");
		ret = tcp_dd_handle_read_request(sockfd, &pkg.body.file_req);
		break;

	case TCP_DD_WRITE:
		pr_bold_info("TCP_DD_WRITE");
		ret = tcp_dd_handle_write_request(sockfd, &pkg.body.file_req);
		break;

	default:
		pr_red_info("Unknown Package type %d", pkg.type);
		return -EINVAL;
	}

	return ret;
}

static int tcp_dd_daemon_handle(int index, union cavan_service_data data)
{
	int ret;
	int server_sockfd, client_sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;

	server_sockfd = data.type_int;

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	pr_bold_info("IP = %s, port = %d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	ret = tcp_dd_handle_request(client_sockfd);
	msleep(100);
	inet_close_tcp_socket(client_sockfd);

	return ret;
}

int tcp_dd_service_run(struct cavan_service_description *desc, u16 port)
{
	int ret;
	int sockfd;

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		print_error("inet_create_tcp_service");
		return sockfd;
	}

	pr_bold_info("Port = %d", port);

	desc->data.type_int = sockfd;
	desc->handler = tcp_dd_daemon_handle;
	desc->threads = NULL;
	ret = cavan_service_run(desc);
	cavan_service_stop(desc);
	inet_close_tcp_socket(sockfd);

	return ret;
}

int tcp_dd_send_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size)
{
	int sockfd;
	int fd;
	int ret;
	struct stat st;

	if (src_file == NULL && dest_file == NULL)
	{
		pr_red_info("src_file == NULL && dest_file == NULL");
		return -EINVAL;
	}

	if (src_file == NULL)
	{
		src_file = dest_file;
	}

	if (dest_file == NULL)
	{
		dest_file = src_file;
	}

	fd = open(src_file, O_RDONLY);
	if (fd < 0)
	{
		pr_red_info("Open file `%s' failed", src_file);
		return fd;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		pr_red_info("Get file `%s' stat failed", src_file);
		goto out_close_fd;
	}

	if (size == 0)
	{
		size = st.st_size;
	}

	if (size < src_offset)
	{
		pr_red_info("No data to sent");
		return -EINVAL;
	}

	ret = lseek(fd, src_offset, SEEK_SET);
	if (ret < 0)
	{
		pr_red_info("Seek file `%s' failed", src_file);
		goto out_close_fd;
	}

	size -= src_offset;

	sockfd = inet_create_tcp_link2(ip, port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_link2");
		goto out_close_fd;
	}

	ret = tcp_dd_send_write_request(sockfd, dest_file, dest_offset, size, st.st_mode);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_write_request2");
		goto out_close_sockfd;
	}

	println("filename = %s", src_file);
	println("offset = %s", size2text(src_offset));
	println("size = %s", size2text(size));
	ret = ffile_ncopy(fd, sockfd, size);

out_close_sockfd:
	msleep(100);
	inet_close_tcp_socket(sockfd);
out_close_fd:
	close(fd);
	return ret;
}

int tcp_dd_receive_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size)
{
	int fd;
	int sockfd;
	int ret;
	struct tcp_dd_package pkg;

	if (src_file == NULL && dest_file == NULL)
	{
		pr_red_info("src_file == NULL && dest_file == NULL");
		return -EINVAL;
	}

	if (src_file == NULL)
	{
		src_file = dest_file;
	}

	if (dest_file == NULL)
	{
		dest_file = src_file;
	}

	if (file_test(dest_file, "b") == 0)
	{
		umount_device(dest_file, MNT_DETACH);
	}

	sockfd = inet_create_tcp_link2(ip, port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_link2");
		return sockfd;
	}

	ret = tcp_dd_send_read_request(sockfd, src_file, src_offset, size, &pkg);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_read_request");
		goto out_close_sockfd;
	}

	fd = open(dest_file, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, pkg.body.file_req.mode);
	if (fd < 0)
	{
		tcp_dd_send_response(sockfd, fd, "[Client] Open file `%s' failed", dest_file);
		goto out_close_sockfd;
	}

	if (size == 0)
	{
		size = pkg.body.file_req.size;
	}

	ret = lseek(fd, dest_offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(sockfd, ret, "[Client] Seek file `%s' failed", dest_file);
		goto out_close_fd;
	}

	ret = tcp_dd_send_response(sockfd, 0, "[Client] Start receive file");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		goto out_close_fd;
	}

	println("filename = %s", dest_file);
	println("offset = %s", size2text(dest_offset));
	println("size = %s", size2text(size));
	ret = ffile_ncopy(sockfd, fd, size);

out_close_fd:
	close(fd);
out_close_sockfd:
	msleep(100);
	inet_close_tcp_socket(sockfd);
	return ret;
}
