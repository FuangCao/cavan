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

		if (code < 0)
		{
			pr_error_info("%s", pkg.res_pkg.message);
		}
	}

	return inet_send(sockfd, (char *)&pkg, MOFS(struct tcp_dd_package, res_pkg.message) + ret);
}

static int tcp_dd_recv_response(int sockfd)
{
	ssize_t rdlen;
	struct tcp_dd_package pkg;

	rdlen = inet_recv(sockfd, &pkg, sizeof(pkg));
	if (rdlen < 0)
	{
		pr_red_info("inet_recv");
		return rdlen;
	}

	if (pkg.type != TCP_DD_RESPONSE)
	{
		pr_red_info("pkg.type = %d", pkg.type);
		return -EINVAL;
	}

	tcp_dd_show_response(&pkg.res_pkg);

	return pkg.res_pkg.code;
}

static int tcp_dd_send_read_request(int sockfd, const char *filename, off_t offset, off_t size, struct tcp_dd_package *pkg)
{
	int ret;

	pkg->type = TCP_DD_READ;
	pkg->file_req.offset = offset;
	pkg->file_req.size = size;
	ret = text_copy(pkg->file_req.filename, filename) - (char *)&pkg + 1;

	ret = inet_send(sockfd, (char *)pkg, ret);
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
	ret = inet_send(sockfd, (char *)&pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(sockfd);
}

static int tcp_dd_send_exec_request(int sockfd, int ttyfd, const char *command)
{
	int ret;
	struct tcp_dd_package pkg;
	char *p;

	if (isatty(ttyfd))
	{
		struct winsize wsize;

		ret = ioctl(ttyfd, TIOCGWINSZ, &wsize);
		if (ret < 0)
		{
			pr_error_info("ioctl TIOCGWINSZ");
			return ret;
		}

		pkg.exec_req.lines = wsize.ws_row;
		pkg.exec_req.columns = wsize.ws_col;
	}
	else
	{
		pr_red_info("%d is not a terminal", ttyfd);

		pkg.exec_req.lines = 0;
		pkg.exec_req.columns = 0;
	}

	pkg.type = TCP_DD_EXEC;
	p = text_copy(pkg.exec_req.command, command);

	ret = p - (char *)&pkg;
	ret = inet_send(sockfd, (char *)&pkg, ret + 1);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(sockfd);
}

static int tcp_dd_send_alarm_add_request(int sockfd, time_t time, time_t repeat, const char *command)
{
	int ret;
	struct tcp_dd_package pkg;
	char *p;

	pkg.type = TCP_ALARM_ADD;
	pkg.alarm_add.time = time;
	pkg.alarm_add.repeat = repeat;
	p = text_copy(pkg.alarm_add.command, command);

	ret = p - (char *)&pkg;
	ret = inet_send(sockfd, (char *)&pkg, ret + 1);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(sockfd);
}

static int tcp_dd_send_alarm_query_request(int sockfd, int type, int index)
{
	int ret;
	struct tcp_dd_package pkg;

	pkg.type = type;
	pkg.alarm_query.index = index;

	ret = sizeof(pkg.alarm_query) + MOFS(struct tcp_dd_package, alarm_query);
	ret = inet_send(sockfd, (char *)&pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(sockfd);
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

static int tcp_dd_handle_exec_request(int sockfd, struct tcp_dd_exec_request *req, struct sockaddr_in *addr)
{
	int ret;

	ret = tcp_dd_send_response(sockfd, 0, "[Server] start execute command");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	setenv(CAVAN_IP_ENV_NAME, inet_ntoa(addr->sin_addr), 1);

	return cavan_exec_redirect_stdio_main(req->command, req->lines, req->columns, sockfd, sockfd);
}

static void tcp_dd_alarm_handler(struct cavan_alarm_node *alarm, struct cavan_alarm_thread *thread, void *data)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		const char *shell_command = "sh";

		execlp(shell_command, shell_command, "-c", data, NULL);
	}
}

static void tcp_dd_alarm_destroy(struct cavan_alarm_node *node, void *data)
{
	free(node);
}

static int tcp_dd_handle_alarm_add_request(struct cavan_alarm_thread *alarm, int sockfd, struct tcp_alarm_add_request *req, struct sockaddr_in *addr)
{
	int ret;
	char *command;
	struct cavan_alarm_node *node;

	node = malloc(sizeof(*node) + text_len(req->command) + 1);
	if (node == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	command = (char *)(node + 1);
	text_copy(command, req->command);

	cavan_alarm_node_init(node, command, tcp_dd_alarm_handler);
	node->time = req->time;
	node->repeat = req->repeat;
	node->destroy = tcp_dd_alarm_destroy;

	ret = cavan_alarm_insert_node(alarm, node, NULL);
	if (ret < 0)
	{
		tcp_dd_send_response(sockfd, ret, "[Server] cavan_alarm_insert_node");
		goto out_free_node;
	}

	return 0;

out_free_node:
	free(node);
	return ret;
}

static int tcp_dd_handle_alarm_remove_request(struct cavan_alarm_thread *alarm, int sockfd, struct tcp_alarm_query_request *req, struct sockaddr_in *addr)
{
	struct double_link_node *node;

	node = double_link_get_node(&alarm->link, req->index);
	if (node == NULL)
	{
		tcp_dd_send_response(sockfd, -ENOENT, "[Server] alarm not found");
		return -ENOENT;
	}

	cavan_alarm_delete_node(alarm, double_link_get_container(&alarm->link, node));

	return 0;
}

static int tcp_dd_handle_alarm_list_request(struct cavan_alarm_thread *alarm, int sockfd, struct tcp_alarm_query_request *req, struct sockaddr_in *addr)
{
	int ret;
	struct cavan_alarm_node *node;
	struct tcp_alarm_add_request item;

	ret = tcp_dd_send_response(sockfd, 0, "[Server] start send alarm list");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	double_link_foreach(&alarm->link, node)
	{
		msleep(1);

		item.time = node->time;
		item.repeat = node->repeat;
		text_copy(item.command, node->private_data);

		ret = inet_send(sockfd, (char *)&item, MOFS(struct tcp_alarm_add_request, command) + text_len(item.command) + 1);
		if (ret < 0)
		{
			pr_red_info("inet_send");
			link_foreach_return(&alarm->link, ret);
		}
	}
	end_link_foreach(&alarm->link);

	return 0;
}

static int tcp_dd_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	socklen_t addrlen;
	struct inet_connect *client = conn;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	client->sockfd = inet_accept(dd_service->sockfd, &client->addr, &addrlen);
	if (client->sockfd < 0)
	{
		pr_error_info("inet_accept");
		return client->sockfd;
	}

	inet_show_sockaddr(&client->addr);

	return 0;
}

static void tcp_dd_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct inet_connect *client = conn;

	inet_close_tcp_socket(client->sockfd);
}

static int tcp_dd_service_start_handler(struct cavan_dynamic_service *service)
{
	int ret;
	int sockfd;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	sockfd = inet_create_tcp_service(dd_service->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	dd_service->sockfd = sockfd;

	ret = cavan_alarm_thread_init(&dd_service->alarm);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_init");
		goto out_inet_close_tcp_socket;
	}

	ret = cavan_alarm_thread_start(&dd_service->alarm);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_start");
		goto out_cavan_alarm_thread_deinit;
	}

#ifndef CAVAN_ARCH_ARM
	cavan_daemon_permission_clear(1 << CAP_SYS_BOOT);
#endif

	return 0;

out_cavan_alarm_thread_deinit:
	cavan_alarm_thread_deinit(&dd_service->alarm);
out_inet_close_tcp_socket:
	inet_close_tcp_socket(sockfd);
	return ret;
}

static void tcp_dd_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	cavan_alarm_thread_stop(&dd_service->alarm);
	cavan_alarm_thread_deinit(&dd_service->alarm);
	inet_close_tcp_socket(dd_service->sockfd);
}

static int tcp_dd_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	bool need_response;
	struct tcp_dd_package pkg;
	struct inet_connect *client = conn;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	ret = inet_recv(client->sockfd, &pkg, sizeof(pkg));
	if (ret < 0)
	{
		pr_red_info("inet_recv");
		return ret;
	}

	need_response = false;

	switch (pkg.type)
	{
	case TCP_DD_READ:
		pr_bold_info("TCP_DD_READ");
		ret = tcp_dd_handle_read_request(client->sockfd, &pkg.file_req);
		break;

	case TCP_DD_WRITE:
		pr_bold_info("TCP_DD_WRITE");
		ret = tcp_dd_handle_write_request(client->sockfd, &pkg.file_req);
		need_response = true;
		break;

	case TCP_DD_EXEC:
		pr_bold_info("TCP_DD_EXEC");
		ret = tcp_dd_handle_exec_request(client->sockfd, &pkg.exec_req, &client->addr);
		break;

	case TCP_ALARM_ADD:
		pr_bold_info("TCP_ALARM_ADD");
		ret = tcp_dd_handle_alarm_add_request(&dd_service->alarm, client->sockfd, &pkg.alarm_add, &client->addr);
		need_response = true;
		break;

	case TCP_ALARM_REMOVE:
		pr_bold_info("TCP_ALARM_REMOVE");
		ret = tcp_dd_handle_alarm_remove_request(&dd_service->alarm, client->sockfd, &pkg.alarm_query, &client->addr);
		need_response = true;
		break;

	case TCP_ALARM_LIST:
		pr_bold_info("TCP_ALARM_LIST");
		ret = tcp_dd_handle_alarm_list_request(&dd_service->alarm, client->sockfd, &pkg.alarm_query, &client->addr);
		break;

	default:
		pr_red_info("Unknown package type %d", pkg.type);
		return -EINVAL;
	}

	if (need_response)
	{
		tcp_dd_send_response(client->sockfd, ret, NULL);
	}

	msleep(100);

	return ret;
}

int tcp_dd_service_run(struct cavan_dynamic_service *service)
{
	service->name = "TCP_DD";
	service->conn_size = sizeof(struct inet_connect);
	service->start = tcp_dd_service_start_handler;
	service->stop = tcp_dd_service_stop_handler;
	service->run = tcp_dd_service_run_handler;
	service->open_connect = tcp_dd_service_open_connect;
	service->close_connect = tcp_dd_service_close_connect;

	return cavan_dynamic_service_run(service);
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
	if (ret < 0)
	{
		pr_red_info("ffile_ncopy");
		goto out_close_fd;
	}

	ret = tcp_dd_recv_response(sockfd);

out_close_sockfd:
	msleep(100);
	file_req->close_connect(sockfd);
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
		umount_partition(dest_file, MNT_DETACH);
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
		ret = fd;
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

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("file_req->open_connect");
		return sockfd;
	}

	ret = tcp_dd_send_exec_request(sockfd, fileno(stdout), file_req->command);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_exec_request");
		goto out_close_sockfd;
	}

	ret = cavan_tty_redirect_base(sockfd);
out_close_sockfd:
	file_req->close_connect(sockfd);
	return ret;
}

int tcp_alarm_add(struct inet_file_request *file_req, time_t time, time_t repeat)
{
	int ret;
	int sockfd;

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("file_req->open_connect");
		return sockfd;
	}

	cavan_show_date2(time, "date = ");
	pr_bold_info("repeat = %lds", repeat);
	pr_bold_info("command = %s", file_req->command);

	ret = tcp_dd_send_alarm_add_request(sockfd, time, repeat, file_req->command);
	file_req->close_connect(sockfd);

	return ret;
}

int tcp_alarm_remove(struct inet_file_request *file_req, int index)
{
	int ret;
	int sockfd;

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("file_req->open_connect");
		return sockfd;
	}

	ret = tcp_dd_send_alarm_query_request(sockfd, TCP_ALARM_REMOVE, index);
	file_req->close_connect(sockfd);

	return ret;
}

int tcp_alarm_list(struct inet_file_request *file_req, int index)
{
	int ret;
	int sockfd;
	struct tcp_alarm_add_request alarm;

	sockfd = file_req->open_connect(file_req->ip, file_req->port);
	if (sockfd < 0)
	{
		pr_red_info("file_req->open_connect");
		return sockfd;
	}

	ret = tcp_dd_send_alarm_query_request(sockfd, TCP_ALARM_LIST, index);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_alarm_query_request");
		goto out_close_sockfd;
	}

	index = 0;

	while (1)
	{
		char prompt[1024];

		ret = inet_recv(sockfd, &alarm, sizeof(alarm));
		if (ret <= 0)
		{
			break;
		}

		sprintf(prompt, "index = %d, command = %s, repeat = %ds, date = ", index++, alarm.command, alarm.repeat);
		cavan_show_date2(alarm.time, prompt);
	}

out_close_sockfd:
	file_req->close_connect(sockfd);
	return ret;
}
