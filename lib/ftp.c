#include <cavan.h>
#include <cavan/ftp.h>
#include <cavan/service.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

char ftp_root_path[1024] = "/";
char ftp_netdev_name[32] = "eth0";

static inline int ftp_check_socket(int sockfd, const struct sockaddr_in *addr)
{
	return sockfd < 0 ? inet_create_tcp_link1(addr) : sockfd;
}

int ftp_server_send_file1(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	ret = inet_tcp_send_file1(sockfd, fd);

	close(sockfd);

	return ret;
}

int ftp_server_send_file2(int sockfd, const struct sockaddr_in *addr, const char *filename)
{
	int fd;
	int ret;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s failed", filename);
		return fd;
	}

	ret = ftp_server_send_file1(sockfd, addr, fd);

	close(fd);

	return ret;
}

int ftp_server_receive_file1(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	ret = inet_tcp_receive_file1(sockfd, fd);

	close(sockfd);

	return ret;
}

int ftp_server_receive_file2(int sockfd, const struct sockaddr_in *addr, const char *fllename)
{
	return 0;
}

ssize_t ftp_send_text_data(int sockfd, const void *text, size_t size)
{
	const void *text_end;
	char buff[size << 1], *p;

	for (p = buff, text_end = text + size; text < text_end; p++, text++)
	{
		switch (*(const char *)text)
		{
		case '\r':
			break;

		case '\n':
			*p++ = '\r';
		default:
			*p = *(const char *)text;
		}
	}

	return inet_send(sockfd, buff, p - buff);
}

ssize_t ftp_send_data(int sockfd, const struct sockaddr_in *addr, const void *buff, size_t size, char type)
{
	int ret;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	if (type == 'I')
	{
		ret = inet_send(sockfd, buff, size);
	}
	else
	{
		ret = ftp_send_text_data(sockfd, buff, size);
	}

	close(sockfd);

	return ret;
}

int ftp_send_text_file1(int sockfd, int fd)
{
	int sendlen, readlen;
	char buff[1024];

	while (1)
	{
		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		sendlen = ftp_send_text_data(sockfd, buff, readlen);
		if (sendlen < 0)
		{
			print_error("ftp_send_text_data");
			return sendlen;
		}
	}

	return 0;
}

int ftp_send_text_file2(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link1");
		return sockfd;
	}

	ret = ftp_send_text_file1(sockfd, fd);

	close(sockfd);

	return ret;
}

int ftp_send_text_file3(int sockfd, struct sockaddr_in *addr, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open");
		return fd;
	}

	ret = ftp_send_text_file2(sockfd, addr, fd);

	close(fd);

	return ret;
}

static char *ftp_file_time_tostring(const time_t *time, char *text)
{
	struct tm ti;

	if (localtime_r(time, &ti) == NULL)
	{
		mem_set8(&ti, 0, sizeof(ti));
	}

	return text + sprintf(text, "%s %02d %02d:%02d", month_tostring(ti.tm_mon), ti.tm_mday, ti.tm_hour, ti.tm_min);
}

char *ftp_file_stat_tostring(const char *filepath, char *text)
{
	int ret;
	struct stat st;

	ret = stat(filepath, &st);
	if (ret < 0)
	{
		print_error("get file %s stat", filepath);
		return NULL;
	}

	*text++ = file_type_to_char(st.st_mode);
	text = file_permition_tostring(st.st_mode, text);
	text += sprintf(text, " %-5lld %-5lld %-5lld %-10lld ", (u64)st.st_nlink, (u64)st.st_uid, (u64)st.st_gid, (u64)st.st_size);
	text = ftp_file_time_tostring(&st.st_mtime, text);

	return text;
}

char *ftp_list_directory1(const char *dirpath, char *text)
{
	DIR *dp;
	struct dirent *ep;
	char tmp_path[1024], *name_p;
	char *text_p;

	dp = opendir(dirpath);
	if (dp == NULL)
	{
		print_error("opendir failed");
		return NULL;
	}

	name_p = text_path_cat(tmp_path, dirpath, NULL);

	while ((ep = readdir(dp)))
	{
		text_copy(name_p, ep->d_name);

		text_p = ftp_file_stat_tostring(tmp_path, text);
		if (text_p == NULL)
		{
			continue;
		}
		text = text_p;
		*text++ = ' ';
		text = text_copy(text, ep->d_name);
		*text++ = '\n';
	}

	*text = 0;

	return text;
}

ssize_t ftp_list_directory2(int sockfd, const struct sockaddr_in *addr, const char *dirpath)
{
	int ret;
	char buff[MB(1)], *p;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	p = ftp_list_directory1(dirpath, buff);
	if (p == NULL)
	{
		ret = -1;
		goto out_close_sockfd;
	}

	ret = inet_send(sockfd, buff, p - buff);

out_close_sockfd:
	close(sockfd);

	return ret;
}

int ftp_send_text(int sockfd, const char *format, ...)
{
	int ret;
	char buff[1024];
	va_list ap;

	va_start(ap, format);
	ret = vsprintf(buff, format, ap);
	va_end(ap);

	return inet_send(sockfd, buff, ret);
}

ssize_t ftp_receive_timeout(int sockfd, void *buff, size_t size)
{
	while (1)
	{
		int i;
		ssize_t recvlen;
		time_t start_time, stop_time;

		start_time = time(NULL);

		for (i = 0; i < 2; i++)
		{
			recvlen = inet_recv_timeout(sockfd, buff, size, 5000);
			if (recvlen)
			{
				return recvlen;
			}
		}

		stop_time = time(NULL);

		if (stop_time - start_time == 0)
		{
			return -ENOENT;
		}
	}

	return 0;
}

int ftp_service_login(int sockfd)
{
	char buff[1024];
	const char *reply;
	ssize_t sendlen, recvlen;
	enum cavan_ftp_state state;

	reply = "220 Cavan ftp server ready";
	state = FTP_STATE_READY;

	while (1)
	{
		sendlen = ftp_send_text(sockfd, "%s\r\n", reply);
		if (sendlen < 0)
		{
			print_error("inet_send_text");
			return sendlen;
		}

		recvlen = ftp_receive_timeout(sockfd, buff, sizeof(buff));
		if (recvlen < 0)
		{
			error_msg("inet_recv_timeout");
			return recvlen;
		}

		switch (*(u32 *)buff)
		{
		/* quit */
		case 0x74697571:
		case 0x54495551:
			ftp_send_text(sockfd, "221 Goodbye\r\n");
			return 0;

		/* user */
		case 0x72657375:
		case 0x52455355:
			if (state == FTP_STATE_READY)
			{
				state = FTP_STATE_USER_RECVED;
				reply = "331 Please input password";
			}
			else
			{
				reply = "331 Any password will do";
			}
			break;

		/* pass */
		case 0x73736170:
		case 0x53534150:
			if (state == FTP_STATE_USER_RECVED)
			{
				return 1;
			}
			else
			{
				if (state < FTP_STATE_USER_RECVED)
				{
					reply = "530 Please input username";
				}
			}
			break;

		default:
			reply = "530 Please login with USER and PASS";
		}
	}
}

char *ftp_get_abs_path(const char *root_path, const char *curr_path, const char *path, char *abs_path)
{
	if (*path == '/')
	{
		text_path_cat(abs_path, root_path, path);
	}
	else
	{
		text_path_cat(abs_path, curr_path, path);
	}

	return abs_path;
}

int ftp_service_cmdline(struct cavan_ftp_descriptor *desc, int sockfd, struct sockaddr_in *addr)
{
	ssize_t sendlen, recvlen;
	char cmd_buff[1024], rep_buff[1024], *cmd_arg;
	char list_buff[1024 * 1024], *list_p;
	char abs_path[1024], curr_path[1024];
	char rnfr_path[1024];
	const char *reply;
	int ret;
	char file_type;
	struct stat st;
	int pasv_port;
	int data_sockfd;
	socklen_t addrlen;
	int fd;

	ret = ftp_service_login(sockfd);
	if (ret <= 0)
	{
		return ret;
	}

	data_sockfd = -1;
	file_type = 0;
	pasv_port = 0;

	reply = "231 User login successfull";
	text_copy(curr_path, ftp_root_path);

	while (1)
	{
		if (reply)
		{
			sendlen = ftp_send_text(sockfd, "%s\r\n", reply);
			if (sendlen < 0)
			{
				print_error("inet_send_text");
				return sendlen;
			}
		}

		recvlen = ftp_receive_timeout(sockfd, cmd_buff, sizeof(cmd_buff));
		if (recvlen < 0)
		{
			error_msg("ftp_receive_timeout");
			return recvlen;
		}

		cmd_buff[recvlen - 2] = 0;
		reply = rep_buff;

		for (cmd_arg = cmd_buff; *cmd_arg && !is_empty_character(*cmd_arg); cmd_arg++);
		while (is_empty_character(*cmd_arg))
		{
			cmd_arg++;
		}

		println("command[%d] = %s", recvlen, cmd_buff);

		switch (*(u32 *)cmd_buff)
		{
		/* quit */
		case 0x74697571:
		case 0x54495551:
			ftp_send_text(sockfd, "221 Goodbye\r\n");
			return 0;

		/* user */
		case 0x72657375:
		case 0x52455355:
			reply = "331 Any password will do";
			break;

		/* pass */
		case 0x73736170:
		case 0x53534150:
			reply = "530 Please input username";
			break;

		/* port */
		case 0x74726f70:
		case 0x54524f50:
		{
			int temp[6];

			ret = sscanf(cmd_arg, "%d,%d,%d,%d,%d,%d", temp, temp + 1, temp + 2, temp + 3, temp + 4, temp + 5);
			if (ret == 6)
			{
				addr->sin_addr.s_addr = temp[3] << 24 | temp[2] << 16 | temp[1] << 8 | temp[0];
				addr->sin_port = temp[5] << 8 | temp[4];
				inet_show_sockaddr(addr);
				pasv_port = 1;
				reply = "200 PORT command complete";
			}
			else
			{
				reply = "501 Argument error";
			}
			break;
		}

		/* opts */
		case 0x7374706f:
		case 0x5354504f:
			reply = "200 OPTS command complete";
			break;

		/* pwd */
		case 0x00647770:
		case 0x00445750:
		case 0x64777078:
		case 0x44575058:
			sprintf(rep_buff, "257 \"%s\"", curr_path);
			break;

		/* type */
		case 0x65707974:
		case 0x45505954:
			reply = "220 TYPE commnd complete";
			file_type = cmd_buff[5];
			break;

		/* syst */
		case 0x74737973:
		case 0x54535953:
			reply = "215 UNIX Type L8";
			break;

		/* cwd */
		case 0x20647763:
		case 0x20445743:
			if (*cmd_arg)
			{
				ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, curr_path);
			}
			else
			{
				text_copy(curr_path, ftp_root_path);
			}

			reply = "250 CWD commnd complete";
			break;

		/* list */
		case 0x7473696c:
		case 0x5453494c:
			if (!pasv_port)
			{
				reply = "550 Please run PORT or PASV first";
				continue;
			}

#if 0
			if (*cmd_arg)
			{
				list_p = ftp_list_directory1(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), list_buff);
			}
			else
			{
				list_p = ftp_list_directory1(curr_path, list_buff);
			}
#else
			list_p = ftp_list_directory1(curr_path, list_buff);
#endif

			if (list_p == NULL)
			{
				sprintf(rep_buff, "550 List directory failed: %s", strerror(errno));
				continue;
			}

			sendlen = ftp_send_text(sockfd, "150 List directory complete\r\n");
			if (sendlen < 0)
			{
				print_error("ftp_send_text");
				return sendlen;
			}

			ret = ftp_send_data(data_sockfd, addr, list_buff, list_p - list_buff, file_type);
			data_sockfd = -1;
			if (ret < 0)
			{
				sprintf(rep_buff, "550 Send list failed: %s", strerror(errno));
			}
			else
			{
				reply = "226 List send complete";
			}
			break;

		/* size */
		case 0x657a6973:
		case 0x455a4953:
			if (*cmd_arg == 0 || stat(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), &st))
			{
				sprintf(rep_buff, "550 get file size failed: %s", strerror(errno));
			}
			else
			{
				sprintf(rep_buff, "213 %lld", st.st_size);
			}
			break;

		/* retr */
		case 0x72746572:
		case 0x52544552:
			fd = open(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), O_RDONLY);
			if (fd < 0)
			{
				reply = "550 Open file failed";
				continue;
			}

			sendlen = ftp_send_text(sockfd, "125 Starting transfer\r\n");
			if (sendlen < 0)
			{
				error_msg("ftp_send_text");
				return sendlen;
			}

			ret = ftp_server_send_file1(data_sockfd, addr, fd);
			if (ret < 0)
			{
				reply = "550 Send file failed";
			}
			else
			{
				reply = "226 Transfer complete";
			}

			data_sockfd = -1;
			break;

		/* stor */
		case 0x726f7473:
		case 0x524f5453:
			fd = open(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), O_WRONLY | O_CREAT, 0777);
			if (fd < 0)
			{
				reply = "550 Open file failed";
				continue;
			}

			sendlen = ftp_send_text(sockfd, "125 Starting transfer\r\n");
			if (sendlen < 0)
			{
				error_msg("ftp_send_text");
				return sendlen;
			}

			ret = ftp_server_receive_file1(data_sockfd, addr, fd);
			if (ret < 0)
			{
				reply = "550 Receive file failed";
			}
			else
			{
				reply = "226 Transfer complete";
			}

			data_sockfd = -1;
			break;

		/* pasv */
		case 0x76736170:
		case 0x56534150:
			data_sockfd = inet_socket(SOCK_STREAM);
			if (data_sockfd < 0)
			{
				sprintf(rep_buff, "425 Create socket failed: %s", strerror(errno));
				continue;
			}

			ret = inet_bind_rand(data_sockfd, 10);
			if (ret < 0)
			{
				close(data_sockfd);
				sprintf(rep_buff, "425 Bind socket failed: %s", strerror(errno));
				continue;
			}

			sendlen = ftp_send_text(sockfd, "227 Entering Passive Mode (%s,%d,%d)\r\n", desc->ip_addr, (ret >> 8) & 0xFF, ret & 0xFF);
			if (sendlen < 0)
			{
				print_error("ftp_send_text");
				return sendlen;
			}

			ret = inet_listen(data_sockfd);
			if (ret >= 0)
			{
				ret = inet_accept(data_sockfd, addr, &addrlen);
			}
			close(data_sockfd);
			if (ret < 0)
			{
				data_sockfd = -1;
			}
			else
			{
				data_sockfd = ret;
				pasv_port = 1;
			}
			reply = NULL;
			break;

		/* dele */
		case 0x656c6564:
		case 0x454c4544:
			ret = remove(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path));
			if (ret < 0)
			{
				sprintf(rep_buff, "550 remove %s failed: %s", cmd_arg, strerror(errno));
			}
			else
			{
				reply = "200 DELE command complete";
			}
			break;

		/* rmd */
		case 0x20646d72:
		case 0x20444d52:
			ret = rmdir(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path));
			if (ret < 0)
			{
				sprintf(rep_buff, "550 remove %s failed: %s", cmd_arg, strerror(errno));
			}
			else
			{
				reply = "200 DELE command complete";
			}
			break;

		/* mkd */
		case 0x20646b6d:
		case 0x20444b4d:
			ret = mkdir(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), 0777);
			if (ret < 0)
			{
				sprintf(rep_buff, "550 create directory %s failed: %s", cmd_arg, strerror(errno));
			}
			else
			{
				reply = "200 MKD command complete";
			}
			break;

		/* mdtm */
		case 0x6d74646d:
		case 0x4d54444d:
			ret = stat(ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path), &st);
			if (ret < 0)
			{
				sprintf(rep_buff, "550 get file stat failed: %s", strerror(errno));
			}
			else
			{
				struct tm ti;

				if (localtime_r(&st.st_atime, &ti) == NULL)
				{
					sprintf(rep_buff, "550 get localtime failed: %s", strerror(errno));
				}
				else
				{
					sprintf(rep_buff, "213 %04d%02d%02d%02d%02d%02d", \
						ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);
				}
			}
			break;

		/* noop */
		case 0x706f6f6e:
		case 0x504f4f4e:
			reply = "200 NOOP commnd complete";
			break;

		/* rnfr */
		case 0x76666E72:
		case 0x52464E52:
			ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, rnfr_path);
			reply = "350 RNFR command complete";
			break;

		/* rnto */
		case 0x6F746E72:
		case 0x4F544E52:
			ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, abs_path);
			ret = rename(rnfr_path, abs_path);
			if (ret < 0)
			{
				sprintf(rep_buff, "550 Rename %s to %s failed[%s]", rnfr_path, abs_path, strerror(errno));
			}
			else
			{
				sprintf(rep_buff, "250 Rename %s to %s successfully", rnfr_path, abs_path);
			}
			break;

		default:
			pr_red_info("unsupport command: %s", cmd_buff);
			reply = "500 Unknown command";
		}
	}

	return -1;
}

int ftp_service_handle(int index, void *data)
{
	struct sockaddr_in client_addr;
	socklen_t addrlen;
	int sockfd;
	struct cavan_ftp_descriptor *desc = data;

	sockfd = inet_accept(desc->ctrl_sockfd, &client_addr, &addrlen);
	if (sockfd < 0)
	{
		print_error("inet_accept");
		return sockfd;
	}

	inet_show_sockaddr(&client_addr);
	ftp_service_cmdline(desc, sockfd, &client_addr);

	close(sockfd);

	return 0;
}

int ftp_service_run(u16 port, int count)
{
	int ret;
	struct cavan_ftp_descriptor ftp_desc;
	struct sockaddr_in addr;
	struct cavan_service_description service_desc =
	{
		.name = "FTP",
		.daemon_count = count,
		.as_daemon = 0,
		.show_verbose = 0,
		.data = (void *)&ftp_desc,
		.handler = ftp_service_handle
	};

	ftp_desc.ctrl_sockfd = inet_create_tcp_service(port);
	if (ftp_desc.ctrl_sockfd < 0)
	{
		error_msg("inet_create_tcp_service");
		return ftp_desc.ctrl_sockfd;
	}

	ret = inet_get_sockaddr(ftp_desc.ctrl_sockfd, ftp_netdev_name, &addr);
	if (ret < 0)
	{
		error_msg("inet_get_sockaddr");
		goto out_close_ctrl_sockfd;
	}

	text_replace_char2(inet_ntoa(addr.sin_addr), ftp_desc.ip_addr, '.', ',');

	pr_bold_info("FTP Root Path = %s, Services = %d", ftp_root_path, count);
	pr_bold_info("Device = %s, IP = %s, Port = %d", ftp_netdev_name, ftp_desc.ip_addr, port);

	ret = cavan_service_run(&service_desc);
	cavan_service_stop(&service_desc);

out_close_ctrl_sockfd:
	close(ftp_desc.ctrl_sockfd);

	return ret;
}

int ftp_send_command_retry(int sockfd, const char *send_buff, size_t sendlen, char *recv_buff, size_t recvlen, int retry)
{
	int ret;

	if (sendlen == 0)
	{
		sendlen = text_len(send_buff);
	}

	println("send_buff = %s", send_buff);

	while (retry--)
	{
		ret = inet_send(sockfd, send_buff, sendlen);
		if (ret < 0)
		{
			print_error("inet_send");
			return ret;
		}

		ret = file_poll_read(sockfd, 5000);
		if (ret < 0)
		{
			error_msg("file_poll_read");
			return ret;
		}

		if (ret & POLLIN)
		{
			break;
		}
	}

	if (retry < 0)
	{
		return -ETIMEDOUT;
	}

	ret = inet_recv(sockfd, recv_buff, recvlen);
	if (ret < 0)
	{
		print_error("inet_recv");
		return ret;
	}

	recv_buff[ret] = 0;
	println("recv_buff = %s", recv_buff);

	return ret;
}

int ftp_client_receive_file(int ctrl_sockfd, const char *ip, u16 port)
{
	int ret;
	int sockfd, data_sockfd;
	ssize_t recvlen;
	char buff[1024], *p;
	struct sockaddr_in addr;
	socklen_t addrlen;

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_service");
		return sockfd;
	}

	p = text_copy(buff, "PORT ");
	p = text_replace_char2(ip, p, '.', ',');
	p += sprintf(p, ",%d,%d\r\n", port >> 8, port & 0xFF);

	recvlen = ftp_send_command_retry(ctrl_sockfd, buff, p - buff, buff, sizeof(buff), 5);
	if (recvlen < 0)
	{
		error_msg("ftp_send_command_retry");
		return recvlen;
	}

	recvlen = ftp_send_command_retry(ctrl_sockfd, "LIST\r\n", 0, buff, sizeof(buff), 5);
	if (recvlen < 0)
	{
		error_msg("ftp_send_command_retry");
		return recvlen;
	}

	data_sockfd = inet_accept(sockfd, &addr, &addrlen);
	if (data_sockfd < 0)
	{
		print_error("inet_accept");
		ret = data_sockfd;
		goto out_close_sockfd;
	}

	while (1)
	{
		recvlen = inet_recv(data_sockfd, buff, sizeof(buff));
		if (recvlen < 0)
		{
			print_error("inet_recv");
			ret = recvlen;
			goto out_close_data_sockfd;
		}

		if (recvlen == 0)
		{
			pr_green_info("data receive complete");
			break;
		}

		print_ntext(buff, recvlen);
	}

	ret = 0;

out_close_data_sockfd:
	shutdown(data_sockfd, SHUT_RDWR);
	close(data_sockfd);
out_close_sockfd:
	close(sockfd);

	return ret;
}

int ftp_client_run(const char *ip, u16 port)
{
	int sockfd;
	char buff[1024], *p;
	ssize_t sendlen, recvlen;
	int ret;

	pr_bold_info("IP = %s, Port = %d", ip, port);

	sockfd = inet_create_tcp_link2(ip, port);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link2");
		return sockfd;
	}

	while (1)
	{
		recvlen = inet_recv(sockfd, buff, sizeof(buff));
		if (recvlen < 0)
		{
			print_error("inet_recv");
			ret = recvlen;
			break;
		}

		buff[recvlen] = 0;
		println("receive buff[%d] = %s", recvlen, buff);

label_get_command:
		for (p = buff; (*p = getchar()) != '\n'; p++);

		if (p > buff)
		{
			p = text_copy(p, "\r\n");
		}
		else
		{
			goto label_get_command;
		}

		if (text_lhcmp("ls", buff) == 0)
		{
			println("list command");
			ftp_client_receive_file(sockfd, ip, 9999);
		}
		else
		{
			sendlen = inet_send(sockfd, buff, p - buff);
			if (sendlen < 0)
			{
				print_error("inet_send");
				ret = sendlen;
				break;
			}

			println("send buff[%d] = %s", sendlen, buff);
		}
	}

	close(sockfd);

	return ret;
}
