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

	if ((int) res->code < 0)
	{
		if (res->number)
		{
			pd_red_info("%s [%s]", res->message, strerror(res->number));
		}
		else
		{
			pd_red_info("%s", res->message);
		}
	}
	else
	{
		pd_green_info("%s", res->message);
	}
}

static int __printf_format_34__ tcp_dd_send_response(struct network_client *client, int code, const char *fmt, ...)
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

	return client->send(client, (char *)&pkg, MOFS(struct tcp_dd_package, res_pkg.message) + ret);
}

static int tcp_dd_recv_response(struct network_client *client)
{
	ssize_t rdlen;
	struct tcp_dd_package pkg;

	rdlen = client->recv(client, &pkg, sizeof(pkg));
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

static int tcp_dd_send_read_request(struct network_client *client, const char *filename, off_t offset, off_t size, struct tcp_dd_package *pkg)
{
	int ret;

	pkg->type = TCP_DD_READ;
	pkg->file_req.offset = offset;
	pkg->file_req.size = size;
	ret = text_copy(pkg->file_req.filename, filename) - (char *)&pkg + 1;

	ret = client->send(client, (char *) pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	ret = client->recv(client, pkg, sizeof(*pkg));
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

static int tcp_dd_send_write_request(struct network_client *client, const char *filename, off_t offset, off_t size, mode_t mode)
{
	int ret;
	struct tcp_dd_package pkg;

	pkg.type = TCP_DD_WRITE;
	pkg.file_req.offset = offset;
	pkg.file_req.size = size;
	pkg.file_req.mode = mode;

	ret = text_copy(pkg.file_req.filename, filename) - (char *)&pkg + 1;
	ret = client->send(client, (char *)&pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(client);
}

static int tcp_dd_send_exec_request(struct network_client *client, int ttyfd, const char *command)
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

		if (pkg.exec_req.lines == 0)
		{
			p = getenv("LINES");
			if (p)
			{
				pkg.exec_req.lines = text2value_unsigned(p, NULL, 10);
			}
		}

		if (pkg.exec_req.columns == 0)
		{
			p = getenv("COLUMNS");
			if (p)
			{
				pkg.exec_req.columns = text2value_unsigned(p, NULL, 10);
			}
		}
	}
	else
	{
		pd_info("output tty is not a terminal");

		pkg.exec_req.lines = 0xFFFF;
		pkg.exec_req.columns = 0xFFFF;
	}

	pd_info("terminal size = %d x %d", pkg.exec_req.lines, pkg.exec_req.columns);

	pkg.type = TCP_DD_EXEC;

	if (command)
	{
		p = text_copy(pkg.exec_req.command, command);
	}
	else
	{
		p = pkg.exec_req.command;
		*p = 0;
	}

	ret = p - (char *)&pkg;
	client->send(client, (char *)&pkg, ret + 1);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(client);
}

static int tcp_dd_send_alarm_add_request(struct network_client *client, time_t time, time_t repeat, const char *command)
{
	int ret;
	struct tcp_dd_package pkg;
	char *p;

	pkg.type = TCP_ALARM_ADD;
	pkg.alarm_add.time = time;
	pkg.alarm_add.repeat = repeat;
	p = text_copy(pkg.alarm_add.command, command);

	ret = p - (char *)&pkg;
	ret = client->send(client, (char *)&pkg, ret + 1);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(client);
}

static int tcp_dd_send_alarm_query_request(struct network_client *client, int type, int index)
{
	int ret;
	struct tcp_dd_package pkg;

	pkg.type = type;
	pkg.alarm_query.index = index;

	ret = sizeof(pkg.alarm_query) + MOFS(struct tcp_dd_package, alarm_query);
	ret = client->send(client, (char *)&pkg, ret);
	if (ret < 0)
	{
		pr_red_info("inet_send");
		return ret;
	}

	return tcp_dd_recv_response(client);
}

static int tcp_dd_handle_read_request(struct network_client *client, struct tcp_dd_file_request *req)
{
	int fd;
	int ret;
	off_t size;
	mode_t mode;

	fd = open(req->filename, O_RDONLY);
	if (fd < 0)
	{
		tcp_dd_send_response(client, fd, "[Server] Open file `%s' failed", req->filename);
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

	if (size < (off_t) req->offset)
	{
		ret = -EINVAL;
		tcp_dd_send_response(client, ret, "[Server] No data to be sent");
		goto out_close_fd;
	}

	ret = lseek(fd, req->offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(client, ret, "[Server] Seek file `%s' failed", req->filename);
		goto out_close_fd;
	}

	size -= req->offset;

	mode = ffile_get_mode(fd);
	if (mode == 0)
	{
		ret = -EFAULT;
		tcp_dd_send_response(client, ret, "[Server] Get file `%s' mode failed", req->filename);
		goto out_close_fd;
	}

	ret = tcp_dd_send_write_request(client, req->filename, req->offset, size, mode);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_write_request");
		return ret;
	}

	println("filename = %s", req->filename);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(size));

	ret = network_client_send_file(client, fd, size);

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_write_request(struct network_client *client, struct tcp_dd_file_request *req)
{
	int fd;
	int ret;
	mode_t mode;

	mode = file_get_mode(req->filename);
	switch (mode & S_IFMT)
	{
	case S_IFREG:
		pr_info("remove regular file %s", req->filename);
		unlink(req->filename);
		break;

	case S_IFBLK:
		pr_info("umount block device %s", req->filename);
		umount_device(req->filename, MNT_DETACH);
		break;
	}

	fd = open(req->filename, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, req->mode);
	if (fd < 0)
	{

		tcp_dd_send_response(client, fd, "[Server] Open file `%s' failed", req->filename);
		return fd;
	}

	ret = lseek(fd, req->offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(client, ret, "[Server] Seek file failed");
		goto out_close_fd;
	}

	ret = tcp_dd_send_response(client, 0, "[Server] Start receive file");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	println("filename = %s", req->filename);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(req->size));

	ret = network_client_recv_file(client, fd, req->size);

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_exec_request(struct network_client *client, struct tcp_dd_exec_request *req)
{
	int ret;

	pd_info("command = `%s'", req->command);

#ifndef CAVAN_ARCH_ARM
	if (text_lhcmp("reboot", req->command) == 0 || text_lhcmp("halt", req->command) == 0)
	{
		tcp_dd_send_response(client, -EPERM, "[Server] Don't allow to execute command %s", req->command);
		ERROR_RETURN(EPERM);
	}
#endif

	ret = tcp_dd_send_response(client, 0, "[Server] start execute command");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		return ret;
	}

	if (client->type == NETWORK_CONNECT_TCP || client->type == NETWORK_CONNECT_UDP)
	{
		struct sockaddr_in addr;

		if (inet_getpeername(client->sockfd, &addr) == 0)
		{
			setenv(CAVAN_IP_ENV_NAME, inet_ntoa(addr.sin_addr), 1);
		}
	}

	return network_client_exec_main(client, req->command, req->lines, req->columns);
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

static int tcp_dd_handle_alarm_add_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_add_request *req)
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

	command = (char *) (node + 1);
	text_copy(command, req->command);

	cavan_alarm_node_init(node, command, tcp_dd_alarm_handler);
	node->time = req->time;
	node->repeat = req->repeat;
	node->destroy = tcp_dd_alarm_destroy;

	ret = cavan_alarm_insert_node(alarm, node, NULL);
	if (ret < 0)
	{
		tcp_dd_send_response(client, ret, "[Server] cavan_alarm_insert_node");
		goto out_free_node;
	}

	return 0;

out_free_node:
	free(node);
	return ret;
}

static int tcp_dd_handle_alarm_remove_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_query_request *req)
{
	struct double_link_node *node;

	node = double_link_get_node(&alarm->link, req->index);
	if (node == NULL)
	{
		tcp_dd_send_response(client, -ENOENT, "[Server] alarm not found");
		return -ENOENT;
	}

	cavan_alarm_delete_node(alarm, double_link_get_container(&alarm->link, node));

	return 0;
}

static int tcp_dd_handle_alarm_list_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_query_request *req)
{
	int ret;
	struct cavan_alarm_node *node;
	struct tcp_alarm_add_request item;

	ret = tcp_dd_send_response(client, 0, "[Server] start send alarm list");
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

		ret = client->send(client, (char *)&item, MOFS(struct tcp_alarm_add_request, command) + text_len(item.command) + 1);
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
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	return dd_service->service.accept(&dd_service->service, conn);
}

static void tcp_dd_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct network_client *client = conn;

	client->close(client);
}

static int tcp_dd_service_start_handler(struct cavan_dynamic_service *service)
{
	int ret;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	ret = network_service_open(&dd_service->service, &dd_service->url);
	if (ret < 0)
	{
		pr_red_info("network_service_open2");
		return ret;
	}

	ret = cavan_alarm_thread_init(&dd_service->alarm);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_init");
		goto out_network_service_close;
	}

	ret = cavan_alarm_thread_start(&dd_service->alarm);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_start");
		goto out_cavan_alarm_thread_deinit;
	}

	return 0;

out_cavan_alarm_thread_deinit:
	cavan_alarm_thread_deinit(&dd_service->alarm);
out_network_service_close:
	network_service_close(&dd_service->service);
	return ret;
}

static void tcp_dd_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	cavan_alarm_thread_stop(&dd_service->alarm);
	cavan_alarm_thread_deinit(&dd_service->alarm);
	network_service_close(&dd_service->service);
}

static int tcp_dd_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	bool need_response;
	struct tcp_dd_package pkg;
	struct network_client *client = conn;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	ret = client->recv(client, &pkg, sizeof(pkg));
	if (ret < 1)
	{
		pr_error_info("client->recv");
		return ret < 0 ? ret : -EFAULT;
	}

	need_response = false;

	switch (pkg.type)
	{
	case TCP_DD_READ:
		pr_bold_info("TCP_DD_READ");
		ret = tcp_dd_handle_read_request(client, &pkg.file_req);
		break;

	case TCP_DD_WRITE:
		pr_bold_info("TCP_DD_WRITE");
		ret = tcp_dd_handle_write_request(client, &pkg.file_req);
		need_response = true;
		break;

	case TCP_DD_EXEC:
		pr_bold_info("TCP_DD_EXEC");
		ret = tcp_dd_handle_exec_request(client, &pkg.exec_req);
		break;

	case TCP_ALARM_ADD:
		pr_bold_info("TCP_ALARM_ADD");
		ret = tcp_dd_handle_alarm_add_request(client, &dd_service->alarm, &pkg.alarm_add);
		need_response = true;
		break;

	case TCP_ALARM_REMOVE:
		pr_bold_info("TCP_ALARM_REMOVE");
		ret = tcp_dd_handle_alarm_remove_request(client, &dd_service->alarm, &pkg.alarm_query);
		need_response = true;
		break;

	case TCP_ALARM_LIST:
		pr_bold_info("TCP_ALARM_LIST");
		ret = tcp_dd_handle_alarm_list_request(client, &dd_service->alarm, &pkg.alarm_query);
		break;

	default:
		pr_red_info("Unknown package type %d", pkg.type);
		return -EINVAL;
	}

	if (need_response && ret >= 0)
	{
		tcp_dd_send_response(client, ret, NULL);
	}

	msleep(100);

	return ret;
}

int tcp_dd_service_run(struct cavan_dynamic_service *service)
{
	service->name = "TCP_DD";
	service->conn_size = sizeof(struct network_client);

	service->start = tcp_dd_service_start_handler;
	service->stop = tcp_dd_service_stop_handler;
	service->run = tcp_dd_service_run_handler;
	service->open_connect = tcp_dd_service_open_connect;
	service->close_connect = tcp_dd_service_close_connect;

	return cavan_dynamic_service_run(service);
}

static int tcp_dd_check_file_request(struct network_file_request *file_req, const char **src_file, const char **dest_file)
{
	if (file_req->src_file[0] == 0 && file_req->dest_file[0] == 0)
	{
		pr_red_info("src_file == NULL && dest_file == NULL");
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

int tcp_dd_send_file(struct network_url *url, struct network_file_request *file_req)
{
	int fd;
	int ret;
	struct stat st;
	const char *src_file = NULL;
	const char *dest_file = NULL;
	struct network_client client;

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

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		goto out_close_fd;
	}

	ret = tcp_dd_send_write_request(&client, dest_file, file_req->dest_offset, file_req->size, st.st_mode);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_write_request2");
		goto out_client_close;
	}

	println("filename = %s", src_file);
	println("offset = %s", size2text(file_req->src_offset));
	println("size = %s", size2text(file_req->size));

	ret = network_client_send_file(&client, fd, file_req->size);
	if (ret < 0)
	{
		pr_red_info("network_client_send_file");
		goto out_close_fd;
	}

	ret = tcp_dd_recv_response(&client);

out_client_close:
	msleep(100);
	client.close(&client);
out_close_fd:
	close(fd);
	return ret;
}

int tcp_dd_receive_file(struct network_url *url, struct network_file_request *file_req)
{
	int fd;
	int ret;
	struct tcp_dd_package pkg;
	const char *src_file = NULL;
	const char *dest_file = NULL;
	struct network_client client;

	ret = tcp_dd_check_file_request(file_req, &src_file, &dest_file);
	if (ret < 0)
	{
		return ret;
	}

	if (file_test(dest_file, "b") == 0)
	{
		umount_partition(dest_file, MNT_DETACH);
	}

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("inet_create_tcp_link2");
		return ret;
	}

	ret = tcp_dd_send_read_request(&client, src_file, file_req->src_offset, file_req->size, &pkg);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_read_request");
		goto out_client_close;
	}

	fd = open(dest_file, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, pkg.file_req.mode);
	if (fd < 0)
	{
		ret = fd;
		tcp_dd_send_response(&client, fd, "[Client] Open file `%s' failed", dest_file);
		goto out_client_close;
	}

	if (file_req->size == 0)
	{
		file_req->size = pkg.file_req.size;
	}

	ret = lseek(fd, file_req->dest_offset, SEEK_SET);
	if (ret < 0)
	{
		tcp_dd_send_response(&client, ret, "[Client] Seek file `%s' failed", dest_file);
		goto out_close_fd;
	}

	ret = tcp_dd_send_response(&client, 0, "[Client] Start receive file");
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_response");
		goto out_close_fd;
	}

	println("filename = %s", dest_file);
	println("offset = %s", size2text(file_req->dest_offset));
	println("size = %s", size2text(file_req->size));

	ret = network_client_recv_file(&client, fd, file_req->size);

out_close_fd:
	close(fd);
out_client_close:
	msleep(100);
	client.close(&client);
	return ret;
}

int tcp_dd_exec_command(struct network_url *url, const char *command)
{
	int ret;
	struct termios tty_attr;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_exec_request(&client, fileno(stdout), command);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_exec_request");
		goto out_client_close;
	}

	ret = set_tty_mode(fileno(stdin), 5, &tty_attr);
	if (ret < 0)
	{
		pr_red_info("set_tty_mode");
		goto out_client_close;
	}

	ret = network_client_exec_redirect(&client, fileno(stdin), fileno(stdout));
	restore_tty_attr(fileno(stdin), &tty_attr);
out_client_close:
	client.close(&client);
	return ret;
}

int tcp_alarm_add(struct network_url *url, const char *command, time_t time, time_t repeat)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	cavan_show_date2(time, "date = ");
	pr_bold_info("repeat = %lds", repeat);
	pr_bold_info("command = %s", command);

	ret = tcp_dd_send_alarm_add_request(&client, time, repeat, command);
	client.close(&client);

	return ret;
}

int tcp_alarm_remove(struct network_url *url, int index)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_alarm_query_request(&client, TCP_ALARM_REMOVE, index);
	client.close(&client);

	return ret;
}

int tcp_alarm_list(struct network_url *url, int index)
{
	int ret;
	struct network_client client;
	struct tcp_alarm_add_request alarm;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_alarm_query_request(&client, TCP_ALARM_LIST, index);
	if (ret < 0)
	{
		pr_red_info("tcp_dd_send_alarm_query_request");
		goto out_client_close;
	}

	index = 0;

	while (1)
	{
		char prompt[1024];

		ret = client.recv(&client, &alarm, sizeof(alarm));
		if (ret <= 0)
		{
			break;
		}

		sprintf(prompt, "index = %d, command = %s, repeat = %ds, date = ", index++, alarm.command, alarm.repeat);
		cavan_show_date2(alarm.time, prompt);
	}

out_client_close:
	client.close(&client);
	return ret;
}
