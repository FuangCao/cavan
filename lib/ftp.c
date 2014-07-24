#include <cavan.h>
#include <cavan/ftp.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

#define FTP_DEBUG		0

static inline int ftp_create_data_link(struct network_client *client, const struct network_url *url)
{
	if (client->sockfd < 0)
	{
		return network_client_open(client, url, 0);
	}

	return 0;
}

static int ftp_server_send_file1(struct network_client *client, const struct network_url *url, int fd)
{
	int ret;

	ret = ftp_create_data_link(client, url);
	if (ret < 0)
	{
		pr_red_info("ftp_create_data_link");
		return ret;
	}

	ret = network_client_send_file(client, fd, 0);

	network_client_close(client);

	return ret;
}

static int ftp_server_receive_file1(struct network_client *client, const struct network_url *url, int fd)
{
	int ret;

	ret = ftp_create_data_link(client, url);
	if (ret < 0)
	{
		pr_red_info("ftp_create_data_link");
		return ret;
	}

	ret = network_client_recv_file(client, fd, 0);

	network_client_close(client);

	return ret;
}

static char *ftp_file_time_tostring(const time_t *time, char *buff, char *buff_end)
{
	struct tm ti;

	if (localtime_r(time, &ti) == NULL)
	{
		mem_set8((u8 *) &ti, 0, sizeof(ti));
	}

	return buff + snprintf(buff, buff_end - buff, "%s %02d %02d:%02d", month_tostring(ti.tm_mon), ti.tm_mday, ti.tm_hour, ti.tm_min);
}

char *ftp_file_stat_tostring(const char *filepath, char *buff, char *buff_end)
{
	int ret;
	struct stat st;

	ret = stat(filepath, &st);
	if (ret < 0)
	{
		return NULL;
	}

	if (buff < buff_end)
	{
		*buff++ = file_type_to_char(st.st_mode);
	}

	buff = file_permition_tostring(st.st_mode, buff, buff_end);
	// buff += snprintf(buff, buff_end - buff, " %-5ld %-5d %-5d " PRINT_FORMAT_OFF("-10"), st.st_nlink, st.st_uid, st.st_gid, st.st_size);
	buff += snprintf(buff, buff_end - buff, " %-5" PRINT_FORMAT_SIZE " %-5" PRINT_FORMAT_UID " %-5" PRINT_FORMAT_UID " %-10" PRINT_FORMAT_OFF, st.st_nlink, st.st_uid, st.st_gid, st.st_size);
	buff = ftp_file_time_tostring((time_t *) &st.st_mtime, buff, buff_end);

	return buff;
}

static int ftp_list_directory1(const char *dirpath, const char *newline)
{
	int fd;
	DIR *dp;
	int ret;
	struct dirent *ep;
	char *filename;
	char pathname[1024];
	char buff[1024];
	char *buff_end = buff + sizeof(buff);

#if FTP_DEBUG
	pr_bold_info("dirpath = `%s'", dirpath);
#endif

	dp = opendir(dirpath);
	if (dp == NULL)
	{
		pr_red_info("opendir failed");
		return -EFAULT;
	}

	fd = cavan_temp_file_open(pathname, sizeof(pathname), "cavan-ftp-XXXXXX");
	if (fd < 0)
	{
		pr_red_info("cavan_temp_file_open");
		ret = fd;
		goto out_closedir;
	}

	filename = text_path_cat(pathname, sizeof(pathname), dirpath, NULL);

	while ((ep = readdir(dp)))
	{
		char *p;

		text_copy(filename, ep->d_name);

		p = ftp_file_stat_tostring(pathname, buff, buff_end);
		if (p == NULL)
		{
			continue;
		}

		p += snprintf(p, buff_end - p, " %s%s", ep->d_name, newline);

		ret = ffile_write(fd, buff, p - buff);
		if (ret < 0)
		{
			pr_red_info("ffile_write");
			goto out_close_fd;
		}
	}

	if (lseek(fd, 0, SEEK_SET) != 0)
	{
		pr_error_info("lseek");
		ret = -EFAULT;
		goto out_close_fd;
	}

	closedir(dp);
	return fd;

out_close_fd:
	close(fd);
out_closedir:
	closedir(dp);
	return ret;
}

static char *ftp_get_abs_path(const char *curr_path, const char *path, char *abs_path, size_t size)
{
	if (*path == '/')
	{
		text_ncopy(abs_path, path, size);
	}
	else
	{
		text_path_cat(abs_path, size, curr_path, path);
	}

#if FTP_DEBUG
	println("curr_path = %s, abs_path = %s, path = %s", curr_path, abs_path, path);
#endif

	return abs_path;
}

static int ftp_data_service_open(struct network_service *service, struct network_client *client, char *buff, size_t size)
{
	u8 *ip;
	int ret;
	u16 port;
	struct in_addr addr;
	struct network_url url;

	network_url_init(&url, "tcp", "any", 0, NULL);

	ret = network_service_open(service, &url, 0);
	if (ret < 0)
	{
		pr_red_info("network_service_open");
		return ret;
	}

	ret = network_service_get_local_port(service);
	if (ret < 0)
	{
		pr_error_info("network_service_get_local_port");
		goto out_network_service_close;
	}

	port = ret;

	ret = network_client_get_local_ip(client, &addr);
	if (ret < 0)
	{
		pr_error_info("network_client_getsockname");
		goto out_network_service_close;
	}

	ip = (u8 *) &addr;

	return snprintf(buff, size, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", ip[0], ip[1], ip[2], ip[3], port & 0xFF, (port >> 8) & 0xFF);

out_network_service_close:
	network_service_close(service);
	return ret;
}

static int ftp_service_cmdline(struct cavan_ftp_service *service, struct network_client *client)
{
	int fd;
	int ret;
	u32 command;
	char file_type;
	const char *reply;
	char rep_pasv[64];
	struct pollfd pfds[2];
	enum cavan_ftp_state state;
	struct network_url data_url;
	ssize_t wrlen, rdlen, replen;
	struct network_client data_link;
	struct network_service data_service;
	char abs_path[1024], curr_path[1024];
	char cmd_buff[1024], rep_buff[1024], *cmd_arg, *cmd_end;

	file_type = 0;
	data_link.sockfd = -1;
	data_service.sockfd = -1;
	network_url_init(&data_url, "tcp", NULL, 0, NULL);

	replen = 0;
	reply = "220 Cavan ftp server ready.\r\n";

	state = FTP_STATE_READY;
	text_copy(curr_path, service->home);

	pfds[0].fd = -1;
	pfds[1].fd = client->sockfd;
	pfds[0].events = pfds[1].events = POLLIN;

	while (1)
	{
		if (replen == 0)
		{
			replen = strlen(reply);
		}

#if FTP_DEBUG
		println("reply = %s", reply);
#endif

		wrlen = client->send(client, reply, replen);
		if (wrlen < 0)
		{
			pr_red_info("inet_send_text");
			goto out_close_data_link;
		}

		replen = 0;
		reply = rep_buff;

label_poll:
		ret = poll(pfds, NELEM(pfds), FTP_TIMEOUT_MS);
		if (ret < 1)
		{
			if (ret < 0)
			{
				pr_error_info("poll");
				goto out_close_data_link;
			}

			goto label_poll;
		}

		if (pfds[0].revents == POLLIN)
		{
			if (data_link.sockfd >= 0)
			{
				network_client_close(&data_link);
			}

			ret = network_service_accept(&data_service, &data_link);
			if (ret < 0)
			{
				pr_red_info("network_service_accept");
				goto out_network_service_close;
			}

			if (pfds[1].revents != POLLIN)
			{
				goto label_poll;
			}
		}

		rdlen = client->recv(client, cmd_buff, sizeof(cmd_buff) - 1);
		if (rdlen < 3)
		{
			goto out_close_data_link;
		}

		for (cmd_end = cmd_buff + rdlen - 1; cmd_end > cmd_buff && byte_is_lf(*cmd_end); cmd_end--);

		*++cmd_end = 0;

		for (cmd_arg = cmd_buff; cmd_arg < cmd_end && !byte_is_space(*cmd_arg); cmd_arg++);

		while ( cmd_arg < cmd_end && byte_is_space(*cmd_arg))
		{
			*cmd_arg++ = 0;
		}

#if FTP_DEBUG
		println("command = `%s', args = `%s'", cmd_buff, cmd_arg);
#endif

		command = *(u32 *) cmd_buff;

		switch (command)
		{
		case FTP_BUILD_CMD('q', 'u', 'i', 't'):
		case FTP_BUILD_CMD('Q', 'U', 'I', 'T'):
			network_client_send_text(client, "221 Goodbye.\r\n");
			return 0;

		case FTP_BUILD_CMD('u', 's', 'e', 'r'):
		case FTP_BUILD_CMD('U', 'S', 'E', 'R'):
			if (state < FTP_STATE_USER_RECVED)
			{
				state = FTP_STATE_USER_RECVED;
				reply = "331 Please input password.\r\n";
			}
			else
			{
				reply = "331 Any password will do.\r\n";
			}

			break;

		case FTP_BUILD_CMD('p', 'a', 's', 's'):
		case FTP_BUILD_CMD('P', 'A', 'S', 'S'):
			if (state < FTP_STATE_USER_RECVED)
			{
				reply = "530 Please input username.\r\n";
			}
			else if (state < FTP_STATE_LOGINED)
			{
				state = FTP_STATE_LOGINED;
				reply = "230 User login successfull.\r\n";
			}
			else
			{
				reply = "230 Alreay logged in.\r\n";
			}

			break;

		default:
			if (state < FTP_STATE_LOGINED)
			{
				if (state < FTP_STATE_USER_RECVED)
				{
					reply = "530 Please login with USER and PASS.\r\n";
				}
				else
				{
					reply = "530 Please login with PASS.\r\n";
				}

				break;
			}

			switch (command)
			{
			case FTP_BUILD_CMD('p', 'o', 'r', 't'):
			case FTP_BUILD_CMD('P', 'O', 'R', 'T'):
			{
				int temp[6];

				ret = sscanf(cmd_arg, "%d,%d,%d,%d,%d,%d", temp, temp + 1, temp + 2, temp + 3, temp + 4, temp + 5);
				if (ret == 6)
				{
					snprintf(data_url.memory, sizeof(data_url.memory), "%d.%d.%d.%d", temp[0], temp[1], temp[2], temp[3]);
					data_url.hostname = data_url.memory;
					data_url.port = temp[4] << 8 | temp[5];
					reply = "200 PORT command complete.\r\n";
				}
				else
				{
					reply = "501 Argument error.\r\n";
				}

				break;
			}

			case FTP_BUILD_CMD('o', 'p', 't', 's'):
			case FTP_BUILD_CMD('O', 'P', 'T', 'S'):
				reply = "200 OPTS command complete.\r\n";
				break;

			case FTP_BUILD_CMD('p', 'w', 'd', 0):
			case FTP_BUILD_CMD('P', 'W', 'D', 0):
			case FTP_BUILD_CMD('x', 'p', 'w', 'd'):
			case FTP_BUILD_CMD('X', 'P', 'W', 'D'):
				replen = snprintf(rep_buff, sizeof(rep_buff), "257 \"%s\"\r\n", curr_path);
				break;

			case FTP_BUILD_CMD('t', 'y', 'p', 'e'):
			case FTP_BUILD_CMD('T', 'Y', 'P', 'E'):
				reply = "200 TYPE commnd complete.\r\n";
				file_type = cmd_buff[5];
				break;

			case FTP_BUILD_CMD('s', 'y', 's', 't'):
			case FTP_BUILD_CMD('S', 'Y', 'S', 'T'):
				reply = "215 UNIX Type L8.\r\n";
				break;

			case FTP_BUILD_CMD('c', 'w', 'd', 0):
			case FTP_BUILD_CMD('C', 'W', 'D', 0):
			case FTP_BUILD_CMD('x', 'c', 'w', 'd'):
			case FTP_BUILD_CMD('X', 'C', 'W', 'D'):
				if (*cmd_arg)
				{
					ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path));
					if (access(abs_path, F_OK) < 0)
					{
						reply = "550 Failed to change directory.\r\n";
						break;
					}

					prettify_pathname_base(abs_path, curr_path, sizeof(curr_path));
				}
				else
				{
					text_copy(curr_path, service->home);
				}

				reply = "250 CWD commnd complete.\r\n";
				break;

			case FTP_BUILD_CMD('l', 'i', 's', 't'):
			case FTP_BUILD_CMD('L', 'I', 'S', 'T'):
				if (data_link.sockfd < 0 && data_url.port == 0)
				{
					reply = "550 Please run PORT or PASV first.\r\n";
					break;
				}

				fd = ftp_list_directory1(curr_path, file_type == 'I' ? "\n" : "\r\n");
				if (fd < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 List directory failed: %s.\r\n", strerror(errno));
					break;
				}

				wrlen = network_client_send_text(client, "150 List directory complete.\r\n");
				if (wrlen < 0)
				{
					pr_red_info("ftp_send_text");
					close(fd);
					goto out_close_data_link;
				}

				ret = ftp_server_send_file1(&data_link, &data_url, fd);
				close(fd);

				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 Send list failed: %s.\r\n", strerror(errno));
				}
				else
				{
					reply = "226 List send complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('s', 'i', 'z', 'e'):
			case FTP_BUILD_CMD('S', 'I', 'Z', 'E'):
			{
				struct stat st;

				if (*cmd_arg == 0 || stat(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)), &st))
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 get file size failed: %s.\r\n", strerror(errno));
				}
				else
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "213 %" PRINT_FORMAT_OFF "\r\n", st.st_size);
				}

				break;
			}

			case FTP_BUILD_CMD('r', 'e', 't', 'r'):
			case FTP_BUILD_CMD('R', 'E', 'T', 'R'):
				fd = open(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)), O_RDONLY);
				if (fd < 0)
				{
					reply = "550 Open file failed.\r\n";
					break;
				}

				wrlen = network_client_send_text(client, "125 Starting transfer\r\n");
				if (wrlen < 0)
				{
					pr_red_info("network_client_send_text");
					close(fd);
					goto out_close_data_link;
				}

				ret = ftp_server_send_file1(&data_link, &data_url, fd);
				close(fd);

				if (ret < 0)
				{
					reply = "550 Send file failed.\r\n";
				}
				else
				{
					reply = "226 Transfer complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('s', 't', 'o', 'r'):
			case FTP_BUILD_CMD('S', 'T', 'O', 'R'):
				fd = open(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)), O_WRONLY | O_CREAT, 0777);
				if (fd < 0)
				{
					reply = "550 Open file failed.\r\n";
					break;
				}

				wrlen = network_client_send_text(client, "125 Starting transfer\r\n");
				if (wrlen < 0)
				{
					pr_error_info("ftp_send_text");
					close(fd);
					goto out_close_data_link;
				}

				ret = ftp_server_receive_file1(&data_link, &data_url, fd);
				close(fd);

				if (ret < 0)
				{
					reply = "550 Receive file failed.\r\n";
				}
				else
				{
					reply = "226 Transfer complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('p', 'a', 's', 'v'):
			case FTP_BUILD_CMD('P', 'A', 'S', 'V'):
				if (data_service.sockfd < 0)
				{
					ret = ftp_data_service_open(&data_service, client, rep_pasv, sizeof(rep_pasv));
					if (ret < 0)
					{
						pr_red_info("ftp_data_service_open");
						return ret;
					}

					replen = ret;
					pfds[0].fd = data_service.sockfd;
				}

				reply = rep_pasv;
				break;

			case FTP_BUILD_CMD('d', 'e', 'l', 'e'):
			case FTP_BUILD_CMD('D', 'E', 'L', 'E'):
				ret = remove(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)));
				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 remove %s failed: %s.\r\n", cmd_arg, strerror(errno));
				}
				else
				{
					reply = "200 DELE command complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('r', 'm', 'd', 0):
			case FTP_BUILD_CMD('R', 'M', 'D', 0):
				ret = rmdir(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)));
				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 remove %s failed: %s.\r\n", cmd_arg, strerror(errno));
				}
				else
				{
					reply = "200 DELE command complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('m', 'k', 'd', 0):
			case FTP_BUILD_CMD('M', 'K', 'D', 0):
				ret = mkdir(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)), 0777);
				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 create directory %s failed: %s.\r\n", cmd_arg, strerror(errno));
				}
				else
				{
					reply = "200 MKD command complete.\r\n";
				}

				break;

			case FTP_BUILD_CMD('m', 'd', 't', 'm'):
			case FTP_BUILD_CMD('M', 'D', 'T', 'M'):
			{
				struct stat st;

				ret = stat(ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path)), &st);
				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 get file stat failed: %s.\r\n", strerror(errno));
				}
				else
				{
					struct tm ti;

					if (localtime_r((time_t *) &st.st_atime, &ti) == NULL)
					{
						replen = snprintf(rep_buff, sizeof(rep_buff), "550 get localtime failed: %s.\r\n", strerror(errno));
					}
					else
					{
						replen = snprintf(rep_buff, sizeof(rep_buff), "213 %04d%02d%02d%02d%02d%02d\r\n", \
							ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);
					}
				}

				break;
			}

			case FTP_BUILD_CMD('n', 'o', 'o', 'p'):
			case FTP_BUILD_CMD('N', 'O', 'O', 'P'):
				reply = "200 NOOP commnd complete.\r\n";
				break;

			case FTP_BUILD_CMD('r', 'n', 'f', 'r'):
			case FTP_BUILD_CMD('R', 'N', 'F', 'R'):
			{
				char rnfr_path[1024];

				ftp_get_abs_path(curr_path, cmd_arg, rnfr_path, sizeof(rnfr_path));
				reply = "350 RNFR command complete.\r\n";
				break;
			}

			case FTP_BUILD_CMD('r', 'n', 't', 'o'):
			case FTP_BUILD_CMD('R', 'N', 'T', 'O'):
			{
				char rnfr_path[1024];

				ftp_get_abs_path(curr_path, cmd_arg, abs_path, sizeof(abs_path));
				ret = rename(rnfr_path, abs_path);
				if (ret < 0)
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "550 Rename %s to %s failed[%s].\r\n", rnfr_path, abs_path, strerror(errno));
				}
				else
				{
					replen = snprintf(rep_buff, sizeof(rep_buff), "250 Rename %s to %s successfully.\r\n", rnfr_path, abs_path);
				}

				break;
			}

			case FTP_BUILD_CMD('s', 'i', 't', 'e'):
			case FTP_BUILD_CMD('S', 'I', 'T', 'E'):
				command = *(u32 *) cmd_arg;
				switch (command)
				{
				case FTP_BUILD_CMD('h', 'e', 'l', 'p'):
				case FTP_BUILD_CMD('H', 'E', 'L', 'P'):
					reply = "214 UNMASK HELP\r\n";
					break;

				default:
					goto label_unsupport_command;
				}

				break;

			case FTP_BUILD_CMD('f', 'e', 'a', 't'):
			case FTP_BUILD_CMD('F', 'E', 'A', 'T'):
				reply = "211 MDTM SIZE PASV PORT RNTO MKD RMD DELE RETR STOR\r\n";
				break;

			default:
label_unsupport_command:
				pr_red_info("unsupport command: `%s'", cmd_buff);
				reply = "500 Unknown command.\r\n";
			}
		}
	}

out_close_data_link:
	if (data_link.sockfd >= 0)
	{
		network_client_close(&data_link);
	}
out_network_service_close:
	if (data_service.sockfd >= 0)
	{
		network_service_close(&data_service);
	}
	return 0;
}

static int ftp_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_ftp_service *ftp = cavan_dynamic_service_get_data(service);

	return network_service_accept(&ftp->service, conn);
}

static void ftp_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
}

static int ftp_service_start_handler(struct cavan_dynamic_service *service)
{
	struct cavan_ftp_service *ftp = cavan_dynamic_service_get_data(service);

	return network_service_open(&ftp->service, &ftp->url, 0);
}

static void ftp_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_ftp_service *ftp = cavan_dynamic_service_get_data(service);

	network_service_close(&ftp->service);
}

static int ftp_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_ftp_service *ftp = cavan_dynamic_service_get_data(service);

	return ftp_service_cmdline(ftp, conn);
}

int ftp_service_run(struct cavan_dynamic_service *service)
{
	service->name = "FTP";
	service->conn_size = sizeof(struct network_client);

	service->start = ftp_service_start_handler;
	service->stop = ftp_service_stop_handler;
	service->run = ftp_service_run_handler;
	service->open_connect = ftp_service_open_connect;
	service->close_connect = ftp_service_close_connect;

	return cavan_dynamic_service_run(service);
}

static int ftp_client_receive_file(struct network_client *client, const char *ip, u16 port)
{
	int ret;
	int sockfd, data_sockfd;
	ssize_t rdlen;
	char buff[1024], *p;
	struct sockaddr_in addr;
	socklen_t addrlen;

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	p = text_copy(buff, "PORT ");
	p = text_replace_char2(ip, p, '.', ',');
	p += sprintf(p, ",%d,%d\r\n", port >> 8, port & 0xFF);

	rdlen = ftp_client_send_command(client, buff, p - buff, buff, sizeof(buff));
	if (rdlen < 0)
	{
		pr_red_info("ftp_client_send_command");
		return rdlen;
	}

	rdlen = ftp_client_send_command(client, "LIST\r\n", 0, buff, sizeof(buff));
	if (rdlen < 0)
	{
		pr_red_info("ftp_client_send_command");
		return rdlen;
	}

	data_sockfd = inet_accept(sockfd, &addr, &addrlen);
	if (data_sockfd < 0)
	{
		pr_red_info("inet_accept");
		ret = data_sockfd;
		goto out_close_sockfd;
	}

	while (1)
	{
		rdlen = inet_recv(data_sockfd, buff, sizeof(buff));
		if (rdlen < 0)
		{
			pr_red_info("inet_recv");
			ret = rdlen;
			goto out_close_data_link;
		}

		if (rdlen == 0)
		{
			pr_green_info("data receive complete");
			break;
		}

#if FTP_DEBUG
		print_ntext(buff, rdlen);
#endif
	}

	ret = 0;

out_close_data_link:
	shutdown(data_sockfd, SHUT_RDWR);
	inet_close_tcp_socket(data_sockfd);
out_close_sockfd:
	inet_close_tcp_socket(sockfd);

	return ret;
}

int ftp_client_read_response(struct network_client *client, char *response, size_t size)
{
	int state;
	ssize_t rwlen;
	char buff[2048], *p = buff, *p_end = p + sizeof(buff);

	while (p < p_end)
	{
		char *q;

		rwlen = network_client_timed_recv(client, p, p_end - p, 5000);
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
	state = text2value_unsigned(buff, (const char **) &p, 10);

	if (response && size > 0)
	{
		text_ncopy(response, p + 1, size);
	}

	return state;
}

int ftp_client_send_command(struct network_client *client, const char *command, size_t cmdsize, char *response, size_t repsize)
{
	size_t rwlen;

#if FTP_DEBUG
	println("%s", command);
#endif

	network_client_discard_all(client);

	rwlen = client->send(client, command, cmdsize);
	if (rwlen < cmdsize)
	{
		return -EFAULT;
	}

	return ftp_client_read_response(client, response, repsize);
}

int ftp_client_send_command2(struct network_client *client, char *response, size_t repsize, const char *command, ...)
{
	int ret;
	va_list ap;
	char buff[2048];

	va_start(ap, command);
	ret = vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

#if FTP_DEBUG
	println("%s", buff);
#endif

	return ftp_client_send_command(client, buff, ret, response, repsize);
}

int ftp_client_send_pasv_command(struct network_client *client, struct network_url *url)
{
	int ret;
	int data[6];
	char response[512], *p;

	ret = ftp_client_send_command2(client, response, sizeof(response), "PASV\r\n");
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

	url->protocol = "tcp";
	url->port = data[4] << 8 | data[5];
	url->hostname = url->memory;
	url->pathname = NULL;
	snprintf(url->memory, sizeof(url->memory), "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);

	return 0;
}

int ftp_client_create_pasv_link(struct network_client *client_ctrl, struct network_client *client_data)
{
	int ret;
	struct network_url url;;

	ret = ftp_client_send_pasv_command(client_ctrl, &url);
	if (ret < 0)
	{
		pr_red_info("ftp_client_send_pasv_command");
		return ret;
	}

	return network_client_open(client_data, &url, 0);
}

int ftp_client_login(struct network_client *client, const char *username, const char *password)
{
	int ret;

	if (username == NULL)
	{
		username = "anonymous";
	}

	println("username = %s", username);

	ret = ftp_client_read_response(client, NULL, 0);
	if (ret < 0)
	{
		pr_red_info("ftp_client_read_response");
		return ret;
	}

	if (ret != 220)
	{
		return -EFAULT;
	}

	ret = ftp_client_send_command2(client, NULL, 0, "USER %s\r\n", username);
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
		password = "8888";
	}

	println("password = %s", password);

	ret = ftp_client_send_command2(client, NULL, 0, "PASS %s\r\n", password);
	if (ret < 0)
	{
		pr_red_info("ftp_client_send_command2");
		return ret;
	}

	return 0;
}

int ftp_client_run(struct network_url *url, const char *username, const char *password)
{
	int ret;
	char buff[1024], *p;
	ssize_t wrlen, rdlen;
	struct network_client client;

	ret = network_client_open(&client, url, 0);
	if (ret < 0)
	{
		pr_red_info("network_client_open");
		return ret;
	}

	ret = ftp_client_login(&client, username, password);
	if (ret < 0)
	{
		pr_red_info("ftp_client_login");
		return ret;
	}

	while (1)
	{
		while (1)
		{
			print("cavan-ftp> ");

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
			ftp_client_receive_file(&client, url->hostname, 6789);
		}
		else
		{
			wrlen = client.send(&client, buff, p - buff);
			if (wrlen < 0)
			{
				pr_red_info("inet_send");
				ret = wrlen;
				break;
			}
		}

		rdlen = client.recv(&client, buff, sizeof(buff));
		if (rdlen < 0)
		{
			pr_red_info("client.recv");
			ret = rdlen;
			break;
		}

		buff[rdlen] = 0;
		println("%s", buff);
	}

	network_client_close(&client);

	return ret;
}
