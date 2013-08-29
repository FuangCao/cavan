#include <cavan.h>
#include <cavan/ftp.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

#define FTP_TIMEOUT_MS	0
#define FTP_DEBUG		0

char ftp_root_path[1024] = "/";

static inline int ftp_check_socket(int sockfd, const struct sockaddr_in *addr)
{
	return sockfd < 0 ? inet_create_tcp_link1(addr) : sockfd;
}

static int ftp_server_send_file1(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;
	int sockfd_bak = sockfd;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	ret = inet_tcp_send_file1(sockfd, fd);

	if (sockfd_bak != sockfd)
	{
		inet_close_tcp_socket(sockfd);
	}

	return ret;
}

#if 0
static int ftp_server_send_file2(int sockfd, const struct sockaddr_in *addr, const char *filename)
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
#endif

static int ftp_server_receive_file1(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;
	int sockfd_bak = sockfd;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("ftp_check_socket");
		return sockfd;
	}

	ret = inet_tcp_receive_file1(sockfd, fd);

	if (sockfd_bak != sockfd)
	{
		inet_close_tcp_socket(sockfd);
	}

	return ret;
}

#if 0
static int ftp_server_receive_file2(int sockfd, const struct sockaddr_in *addr, const char *fllename)
{
	return 0;
}
#endif

static ssize_t ftp_send_text_data(int sockfd, const char *text, size_t size)
{
	const char *text_end;
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

static ssize_t ftp_send_data(int sockfd, const struct sockaddr_in *addr, const void *buff, size_t size, char type)
{
	int ret;
	int sockfd_bak = sockfd;

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

	if (sockfd_bak != sockfd)
	{
		inet_close_tcp_socket(sockfd);
	}

	return ret;
}

#if 0
static int ftp_send_text_file1(int sockfd, int fd)
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

static int ftp_send_text_file2(int sockfd, const struct sockaddr_in *addr, int fd)
{
	int ret;

	sockfd = ftp_check_socket(sockfd, addr);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link1");
		return sockfd;
	}

	ret = ftp_send_text_file1(sockfd, fd);

	inet_close_tcp_socket(sockfd);

	return ret;
}

static int ftp_send_text_file3(int sockfd, struct sockaddr_in *addr, const char *filename)
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
#endif

static char *ftp_file_time_tostring(const time_t *time, char *text)
{
	struct tm ti;

	if (localtime_r(time, &ti) == NULL)
	{
		mem_set8((u8 *)&ti, 0, sizeof(ti));
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
#if __WORDSIZE == 64
	text += sprintf(text, " %-5ld %-5ld %-5ld %-10ld ", (u64)st.st_nlink, (u64)st.st_uid, (u64)st.st_gid, (u64)st.st_size);
#else
	text += sprintf(text, " %-5Ld %-5Ld %-5Ld %-10Ld ", (u64)st.st_nlink, (u64)st.st_uid, (u64)st.st_gid, (u64)st.st_size);
#endif
	text = ftp_file_time_tostring((time_t *)&st.st_mtime, text);

	return text;
}

static char *ftp_list_directory1(const char *dirpath, char *text)
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

#if 0
static ssize_t ftp_list_directory2(int sockfd, const struct sockaddr_in *addr, const char *dirpath)
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
	inet_close_tcp_socket(sockfd);

	return ret;
}
#endif

static int ftp_send_text(int sockfd, const char *format, ...)
{
	int ret;
	char buff[1024];
	va_list ap;

	va_start(ap, format);
	ret = vsprintf(buff, format, ap);
	va_end(ap);

#if FTP_DEBUG
	print_ntext(buff, ret);
#endif

	return inet_send(sockfd, buff, ret);
}

static ssize_t ftp_receive_timeout(int sockfd, void *buff, size_t size)
#if FTP_TIMEOUT_MS > 0
{
	while (1)
	{
		int i;
		ssize_t recvlen;
		time_t start_time, stop_time;

		start_time = time(NULL);

		for (i = 0; i < 2; i++)
		{
			recvlen = inet_recv_timeout(sockfd, buff, size, FTP_TIMEOUT_MS);
			if (recvlen)
			{
				return recvlen;
			}
		}

		stop_time = time(NULL);

		if (stop_time - start_time == 0)
		{
			break;
		}
	}

	return -ETIMEDOUT;
}
#else
{
	ssize_t recvlen = inet_recv(sockfd, buff, size);
#if FTP_DEBUG
	if (recvlen > 0)
	{
		print_ntext(buff, recvlen);
	}
#endif

	return recvlen;
}
#endif

static int ftp_service_login(int sockfd)
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
		if (recvlen <= 0)
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

static char *ftp_get_abs_path(const char *root_path, const char *curr_path, const char *path, char *abs_path)
{
	if (*path == '/')
	{
		text_path_cat(abs_path, root_path, path);
	}
	else
	{
		text_path_cat(abs_path, curr_path, path);
	}

#if FTP_DEBUG
	println("root_path = %s, curr_path = %s, abs_path = %s, path = %s", root_path, curr_path, abs_path, path);
#endif

	return abs_path;
}

static int ftp_service_cmdline(struct cavan_ftp_descriptor *desc, int sockfd, struct sockaddr_in *addr)
{
	ssize_t sendlen, recvlen;
	char cmd_buff[1024], rep_buff[1024], *cmd_arg;
	char list_buff[1024 * 1024], *list_p;
	char abs_path[1024], curr_path[1024];
	char rnfr_path[1024];
	char local_ip[32];
	struct sockaddr_in local_addr;
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

	reply = "230 User login successfull";
	text_copy(curr_path, ftp_root_path);

	ret = inet_getsockname(sockfd, &local_addr, &addrlen);
	if (ret < 0)
	{
		pr_red_info("inet_getsockname");
		return ret;
	}

#if FTP_DEBUG
	pr_bold_info("Local Address:");
	inet_show_sockaddr(&local_addr);
#endif

	text_replace_char2(inet_ntoa(local_addr.sin_addr), local_ip, '.', ',');

	while (1)
	{
		if (reply)
		{
			sendlen = ftp_send_text(sockfd, "%s\r\n", reply);
			if (sendlen < 0)
			{
				print_error("inet_send_text");
				goto out_close_data_sockfd;
			}
		}

		recvlen = ftp_receive_timeout(sockfd, cmd_buff, sizeof(cmd_buff));
		if (recvlen <= 0)
		{
			error_msg("ftp_receive_timeout");
			goto out_close_data_sockfd;
		}

		while (recvlen >= 0)
		{
			char c = cmd_buff[recvlen - 1];
			if (c != '\r' && c != '\n')
			{
				break;
			}

			recvlen--;
		}

		cmd_buff[recvlen] = 0;
		reply = rep_buff;

		for (cmd_arg = cmd_buff; *cmd_arg && !is_empty_character(*cmd_arg); cmd_arg++);
		while (is_empty_character(*cmd_arg))
		{
			cmd_arg++;
		}

#if FTP_DEBUG
		if (*cmd_arg)
		{
			println("cmd_arg = %s", cmd_arg);
		}
#endif

		switch (*(u32 *)cmd_buff)
		{
		/* quit */
		case 0x74697571:
		case 0x54495551:
			ftp_send_text(sockfd, "221 Goodbye\r\n");
			goto out_close_data_sockfd;

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
			reply = "200 TYPE commnd complete";
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
				if (cmd_arg[0] == '/')
				{
					text_copy(curr_path, cmd_arg);
				}
				else
				{
					ftp_get_abs_path(ftp_root_path, curr_path, cmd_arg, curr_path);
				}
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
				goto out_close_data_sockfd;
			}

			ret = ftp_send_data(data_sockfd, addr, list_buff, list_p - list_buff, file_type);
			inet_close_tcp_socket(data_sockfd);
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
#if __WORDSIZE == 64
				sprintf(rep_buff, "213 %ld", st.st_size);
#else
				sprintf(rep_buff, "213 %lld", st.st_size);
#endif
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
				goto out_close_data_sockfd;
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

			inet_close_tcp_socket(data_sockfd);
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
				goto out_close_data_sockfd;
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

			inet_close_tcp_socket(data_sockfd);
			data_sockfd = -1;
			break;

		/* pasv */
		case 0x76736170:
		case 0x56534150:
			if (data_sockfd >= 0)
			{
				inet_close_tcp_socket(data_sockfd);
			}

			data_sockfd = inet_socket(SOCK_STREAM);
			if (data_sockfd < 0)
			{
				sprintf(rep_buff, "425 Create socket failed: %s", strerror(errno));
				continue;
			}

			ret = inet_bind_rand(data_sockfd, 10);
			if (ret < 0)
			{
				inet_close_tcp_socket(data_sockfd);
				data_sockfd = -1;
				sprintf(rep_buff, "425 Bind socket failed: %s", strerror(errno));
				continue;
			}

			sendlen = ftp_send_text(sockfd, "227 Entering Passive Mode (%s,%d,%d)\r\n", local_ip, (ret >> 8) & 0xFF, ret & 0xFF);
			if (sendlen < 0)
			{
				print_error("ftp_send_text");
				goto out_close_data_sockfd;
			}

			ret = inet_listen(data_sockfd);
			if (ret >= 0)
			{
				ret = inet_accept(data_sockfd, addr, &addrlen);
			}

			inet_close_tcp_socket(data_sockfd);

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

				if (localtime_r((time_t *)&st.st_atime, &ti) == NULL)
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

out_close_data_sockfd:
	if (data_sockfd >= 0)
	{
		inet_close_tcp_socket(data_sockfd);
	}

	return 0;
}

static int ftp_service_handle(struct cavan_service_description *service, int index, cavan_shared_data_t data)
{
	struct sockaddr_in client_addr;
	socklen_t addrlen;
	int sockfd;
	struct cavan_ftp_descriptor *desc = data.type_void;

	sockfd = inet_accept(desc->ctrl_sockfd, &client_addr, &addrlen);
	if (sockfd < 0)
	{
		print_error("inet_accept");
		return sockfd;
	}

	inet_show_sockaddr(&client_addr);
	ftp_service_cmdline(desc, sockfd, &client_addr);

	inet_close_tcp_socket(sockfd);

	return 0;
}

int ftp_service_run(struct cavan_service_description *service_desc, u16 port)
{
	int ret;
	struct cavan_ftp_descriptor ftp_desc;

	ftp_desc.ctrl_sockfd = inet_create_tcp_service(port);
	if (ftp_desc.ctrl_sockfd < 0)
	{
		error_msg("inet_create_tcp_service");
		return ftp_desc.ctrl_sockfd;
	}

	pr_bold_info("FTP Root Path = %s", ftp_root_path);

	service_desc->data.type_void = (void *)&ftp_desc;
	service_desc->handler = ftp_service_handle;
	ret = cavan_service_run(service_desc);
	cavan_service_stop(service_desc);

	inet_close_tcp_socket(ftp_desc.ctrl_sockfd);

	return ret;
}

static int ftp_send_command_retry(int sockfd, const char *send_buff, size_t sendlen, char *recv_buff, size_t recvlen, int retry)
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

		ret = file_poll_input(sockfd, 5000);
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

static int ftp_client_receive_file(int ctrl_sockfd, const char *ip, u16 port)
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

#if FTP_DEBUG
		print_ntext(buff, recvlen);
#endif
	}

	ret = 0;

out_close_data_sockfd:
	shutdown(data_sockfd, SHUT_RDWR);
	inet_close_tcp_socket(data_sockfd);
out_close_sockfd:
	inet_close_tcp_socket(sockfd);

	return ret;
}

int ftp_client_read_response(int sockfd, char *response, size_t size)
{
	int state;
	ssize_t rwlen;
	char buff[2048], *p = buff, *p_end = p + sizeof(buff);

	while (p < p_end)
	{
		char *q;

		rwlen = inet_recv(sockfd, p, p_end - p);
		if (rwlen <= 0)
		{
			return -EFAULT;
		}

		for (q = p, p += rwlen; q < p; q++)
		{
			int step = 0;

			while (q < p && IS_NUMBER(*q))
			{
				q++;
			}

			if (q < p && *q != '-')
			{
				step++;
			}

			while (q < p)
			{
				if (*q == '\n')
				{
					step++;
					break;
				}

				q++;
			}

			if (step == 2)
			{
				goto label_read_complete;
			}
		}
	}

label_read_complete:
	*p = 0;
#if FTP_DEBUG
	print_ntext(buff, p - buff);
#endif
	state = text2value_unsigned(buff, (const char **)&p, 10);

	if (response && size > 0)
	{
		text_ncopy(response, p + 1, size);
	}

	return state;
}

int ftp_client_send_command(int sockfd, const char *command, size_t cmdsize, char *response, size_t repsize)
{
	size_t rwlen;

#if FTP_DEBUG
	println("%s", command);
#endif

	file_discard_all(sockfd);

	rwlen = inet_send(sockfd, command, cmdsize);
	if (rwlen < cmdsize)
	{
		return -EFAULT;
	}

	return ftp_client_read_response(sockfd, response, repsize);
}

int ftp_client_send_command2(int sockfd, char *response, size_t repsize, const char *command, ...)
{
	int ret;
	va_list ap;
	char buff[2048];

	va_start(ap, command);
	ret = vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

	return ftp_client_send_command(sockfd, buff, ret, response, repsize);
}

int ftp_client_send_pasv_command(int sockfd, struct sockaddr_in *addr)
{
	int ret;
	int data[6];
	char response[512], *p;

	ret = ftp_client_send_command2(sockfd, response, sizeof(response), "PASV\r\n");
	if (ret != 227)
	{
		pr_red_info("ftp_client_send_command2 PASV");
		return -EFAULT;
	}

	for (p = response; *p && *p != '('; p++);

	ret = sscanf(p, "(%d,%d,%d,%d,%d,%d)", data, data + 1, data + 2, data + 3, data + 4, data + 5);
	if (ret != 6)
	{
		pr_red_info("invalid response %s", response);
		return -EINVAL;
	}

	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);
	addr->sin_port = htons((u16)(data[4] << 8 | data[5]));

	inet_show_sockaddr(addr);

	return 0;
}

int ftp_client_login(int sockfd, const char *username, const char *password)
{
	int ret;

	if (username == NULL)
	{
		username = "anonymous";
	}

#if FTP_DEBUG
	println("username = %s, password = %s", username, password);
#endif

	ret = ftp_client_read_response(sockfd, NULL, 0);
	if (ret < 0)
	{
		pr_red_info("ftp_client_read_response");
		return ret;
	}

	if (ret != 220)
	{
		return -EFAULT;
	}

	ret = ftp_client_send_command2(sockfd, NULL, 0, "USER %s\r\n", username);
	if (ret < 0)
	{
		pr_red_info("ftp_client_send_command2");
		return ret;
	}

	if (ret == 230)
	{
		return 0;
	}

	if (password == NULL)
	{
		password = "cavan";
	}

	ret = ftp_client_send_command2(sockfd, NULL, 0, "PASS %s\r\n", password);
	if (ret < 0)
	{
		pr_red_info("ftp_client_send_command2");
		return ret;
	}

	return 0;
}

int ftp_client_run(const char *hostname, u16 port, const char *username, const char *password)
{
	int sockfd;
	char buff[1024], *p;
	ssize_t sendlen, recvlen;
	int ret;

	sockfd = inet_create_tcp_link2(hostname, port);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link2");
		return sockfd;
	}

	ret = ftp_client_login(sockfd, username, password);
	if (ret < 0)
	{
		pr_red_info("ftp_client_login");
		return ret;
	}

	while (1)
	{
		while (1)
		{
			for (p = buff; (*p = getchar()) != '\n'; p++);

			if (p > buff)
			{
				p = text_copy(p, "\r\n");
				break;
			}
		}

		if (text_lhcmp("ls", buff) == 0)
		{
			println("list command");
			ftp_client_receive_file(sockfd, hostname, 9999);
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
#if __WORDSIZE == 64 || CONFIG_BUILD_FOR_ANDROID
			println("send buff[%ld] = %s", sendlen, buff);
#else
			println("send buff[%d] = %s", sendlen, buff);
#endif
		}

		recvlen = inet_recv(sockfd, buff, sizeof(buff));
		if (recvlen < 0)
		{
			print_error("inet_recv");
			ret = recvlen;
			break;
		}

		buff[recvlen] = 0;
		println("%s", buff);
	}

	inet_close_tcp_socket(sockfd);

	return ret;
}
