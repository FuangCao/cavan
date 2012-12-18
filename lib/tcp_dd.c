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
#include <cavan/command.h>

static void tcp_dd_show_response(struct tcp_dd_response_package *res)
{
	if (res->message[0] == 0)
	{
		return;
	}

	if ((int)res->code < 0)
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
	pkg.res_pkg.code = code;
	pkg.res_pkg.number = errno;

	if (fmt == NULL)
	{
		pkg.res_pkg.message[0] = 0;
		ret = 1;
	}
	else
	{
		va_list ap;

		va_start(ap, fmt);
		ret = vsprintf(pkg.res_pkg.message, fmt, ap) + 1;
		va_end(ap);
	}

	tcp_dd_show_response(&pkg.res_pkg);

	return inet_send(sockfd, &pkg, sizeof(pkg.type) + (sizeof(int) * 2) + ret);
}

static int tcp_dd_send_read_request(int sockfd, const char *filename, off_t offset, off_t size, struct tcp_dd_package *pkg)
{
	int ret;

	pkg->type = TCP_DD_READ;
	pkg->file_req.offset = offset;
	pkg->file_req.size = size;
	ret = text_copy(pkg->file_req.filename, filename) - (char *)&pkg + 1;

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
		tcp_dd_show_response(&pkg->res_pkg);
		return pkg->res_pkg.code;

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
	pkg.file_req.offset = offset;
	pkg.file_req.size = size;
	pkg.file_req.mode = mode;

	ret = text_copy(pkg.file_req.filename, filename) - (char *)&pkg + 1;
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
		tcp_dd_show_response(&pkg.res_pkg);
		return pkg.res_pkg.code;
	}
	else
	{
		return -EINVAL;
	}
}

static int tcp_dd_send_exec_request(int sockfd, const char *command)
{
	int ret;
	struct tcp_dd_package pkg;
	char *p;

	pkg.type = TCP_DD_EXEC;
	p = text_copy(pkg.exec_req.command, command);

	ret = p - (char *)&pkg;
	ret = inet_send(sockfd, &pkg, ret + 1);
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
		return pkg.res_pkg.code;
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

	if (size < (off_t)req->offset)
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

static int tcp_dd_handle_exec_request(int sockfd, struct tcp_dd_exec_request *req)
{
	int ret;

	ret = tcp_dd_send_response(sockfd, 0, "[Server] start execute command %s", req->command);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	return cavan_exec_redirect_stdio_main(req->command, sockfd, sockfd);
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
		ret = tcp_dd_handle_read_request(sockfd, &pkg.file_req);
		break;

	case TCP_DD_WRITE:
		pr_bold_info("TCP_DD_WRITE");
		ret = tcp_dd_handle_write_request(sockfd, &pkg.file_req);
		break;

	case TCP_DD_EXEC:
		pr_bold_info("TCP_DD_EXEC");
		ret = tcp_dd_handle_exec_request(sockfd, &pkg.exec_req);
		break;

	default:
		pr_red_info("Unknown Package type %d", pkg.type);
		return -EINVAL;
	}

	return ret;
}

static int tcp_dd_daemon_handle(int index, cavan_shared_data_t data)
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

static int tcp_dd_check_file_request(struct inet_file_request *file_req, const char **src_file, const char **dest_file)
{
	if (file_req->src_file[0] == 0 && file_req->dest_file[0] == 0)
	{
		pr_red_info("src_file == NULL && dest_file == NULL");
		ERROR_RETURN(EINVAL);
	}

	if (file_req->open_connect == NULL || file_req->close_connect == NULL)
	{
		pr_red_info("file_req->open_connect == NULL || file_req->close_connect == NULL");
		ERROR_RETURN(EINVAL);
	}

	if (file_req->src_file[0] == 0)
	{
		*src_file = *dest_file = file_req->dest_file;
	}
	else if (file_req->dest_file[0] == 0)
	{
		*src_file = *dest_file = file_req->src_file;
	}
	else
	{
		*src_file = file_req->src_file;
		*dest_file = file_req->dest_file;
	}

	return 0;
}

int tcp_dd_send_file(struct inet_file_request *file_req)
{
	int sockfd;
	int fd;
	int ret;
	struct stat st;
	const char *src_file, *dest_file;

	ret = tcp_dd_check_file_request(file_req, &src_file, &dest_file);
	if (ret < 0)
	{
		return ret;
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

	if (file_req->size == 0)
	{
		file_req->size = st.st_size;
	}

	if (file_req->size < file_req->src_offset)
	{
		pr_red_info("No data to sent");
		return -EINVAL;
	}

	ret = lseek(fd, file_req->src_offset, SEEK_SET);
	if (ret < 0)
	{
		pr_red_info("Seek file `%s' failed", src_file);
		goto out_close_fd;
	}

	file_req->size -= file_req->src_offset;

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_link2");
		goto out_close_fd;
	}

	ret = tcp_dd_send_write_request(sockfd, dest_file, file_req->dest_offset, file_req->size, st.st_mode);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_write_request2");
		goto out_close_sockfd;
	}

	println("filename = %s", src_file);
	println("offset = %s", size2text(file_req->src_offset));
	println("size = %s", size2text(file_req->size));
	ret = ffile_ncopy(fd, sockfd, file_req->size);

out_close_sockfd:
	msleep(100);
	inet_close_tcp_socket(sockfd);
out_close_fd:
	close(fd);
	return ret;
}

int tcp_dd_receive_file(struct inet_file_request *file_req)
{
	int fd;
	int sockfd;
	int ret;
	struct tcp_dd_package pkg;
	const char *src_file, *dest_file;

	ret = tcp_dd_check_file_request(file_req, &src_file, &dest_file);
	if (ret < 0)
	{
		return ret;
	}

	if (file_test(dest_file, "b") == 0)
	{
		umount_device(dest_file, MNT_DETACH);
	}

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_link2");
		return sockfd;
	}

	ret = tcp_dd_send_read_request(sockfd, src_file, file_req->src_offset, file_req->size, &pkg);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_read_request");
		goto out_close_sockfd;
	}

	fd = open(dest_file, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, pkg.file_req.mode);
	if (fd < 0)
	{
		tcp_dd_send_response(sockfd, fd, "[Client] Open file `%s' failed", dest_file);
		goto out_close_sockfd;
	}

	if (file_req->size == 0)
	{
		file_req->size = pkg.file_req.size;
	}

	ret = lseek(fd, file_req->dest_offset, SEEK_SET);
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
	println("offset = %s", size2text(file_req->dest_offset));
	println("size = %s", size2text(file_req->size));
	ret = ffile_ncopy(sockfd, fd, file_req->size);

out_close_fd:
	close(fd);
out_close_sockfd:
	msleep(100);
	file_req->close_connect(sockfd);
	return ret;
}

int tcp_dd_exec_command(struct inet_file_request *file_req)
{
	int ret;
	int sockfd;
	ssize_t rwlen;
	char buff[1024];
	struct pollfd pfds[2];
	int tty_in, tty_out;

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("file_req->open_connect");
		return sockfd;
	}

	ret = tcp_dd_send_exec_request(sockfd, file_req->command);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_exec_request");
		goto out_close_sockfd;
	}

	tty_in = fileno(stdin);
	tty_out = fileno(stdout);

	ret = set_tty_mode(tty_in, 5);
	if (ret < 0)
	{
		pr_red_info("set_tty_mode");
		goto out_close_sockfd;
	}

	pfds[0].events = POLLIN;
	pfds[1].fd = tty_in;

	pfds[1].events = POLL_IN;
	pfds[1].fd = sockfd;

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			goto out_restore_tty_attr;
		}

		if (pfds[0].revents)
		{
			rwlen = read(tty_in, buff, sizeof(buff));
			if (rwlen <= 0 || inet_send(sockfd, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = inet_recv(sockfd, buff, sizeof(buff));
			if (rwlen <= 0 || write(tty_out, buff, rwlen) < rwlen)
			{
				break;
			}

			fsync(tty_out);
		}
	}

	ret = 0;
out_restore_tty_attr:
	restore_tty_attr(tty_in);
out_close_sockfd:
	file_req->close_connect(sockfd);
	return ret;
}
