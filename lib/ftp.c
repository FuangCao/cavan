#include <cavan.h>
#include <cavan/ftp.h>
#include <pthread.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

static int ftp_send_file(const struct sockaddr_in *addr, char file_type, const char *filename)
{
	return 0;
}

static int ftp_receive_file(const struct sockaddr_in *addr, char file_type, const char *fllename)
{
	return 0;
}

static int ftp_service_cmdline(struct cavan_ftp_descriptor *desc, int sockfd, struct sockaddr_in *addr)
{
	ssize_t sendlen, recvlen, replylen;
	char buff[1024], reply[1024];
	enum cavan_ftp_state state;
	FILE *fp;
	int ret;
	int data_sockfd;
	char file_type;

	file_type = 0;
	state = FTP_STATE_READY;
	replylen = sprintf(reply, "220 cavan ftp server ready\r\n");

	while (1)
	{
		sendlen = send(sockfd, reply, replylen, 0);
		if (sendlen < 0)
		{
			print_error("inet_send_text");
			return sendlen;
		}

		recvlen = inet_recv_timeout(sockfd, buff, sizeof(buff), 5000);
		if (recvlen < 0)
		{
			error_msg("inet_recv_timeout");
			return recvlen;
		}

		if (recvlen < 5)
		{
			pr_blue_info("comand too short");
			replylen = sprintf(reply, "500 command too short\r\n");
			continue;
		}

		buff[recvlen - 2] = 0;
		println("buff[%d] = %s", recvlen, buff);

		if (recvlen == 5)
		{
			buff[3] = ' ';
		}

		switch (*(u32 *)buff)
		{
		/* quit */
		case 0x74697571:
		case 0x54495551:
			replylen = sprintf(reply, "221 Goodbye\r\n");
			send(sockfd, reply, replylen, 0);
			return 0;

		/* user */
		case 0x72657375:
		case 0x52455355:
			if (state == FTP_STATE_READY)
			{
				state = FTP_STATE_USER_RECVED;
				replylen = sprintf(reply, "331 please input password\r\n");
			}
			else
			{
				pr_red_info("invalid command");
			}
			break;

		/* pass */
		case 0x73736170:
		case 0x53534150:
			if (state == FTP_STATE_USER_RECVED)
			{
				state = FTP_STATE_LOGINED;
				replylen = sprintf(reply, "231 user login successfull\r\n");
			}
			else
			{
				pr_red_info("invalid command");
				if (state < FTP_STATE_USER_RECVED)
				{
					replylen = sprintf(reply, "530 please input username\r\n");
				}
			}
			break;

		/* port */
		case 0x74726f70:
		case 0x54524f50:
			if (state == FTP_STATE_LOGINED)
			{
				int temp[6];

				ret = sscanf(buff + 5, "%d,%d,%d,%d,%d,%d", temp, temp + 1, temp + 2, temp + 3, temp + 4, temp + 5);
				if (ret == 6)
				{
					addr->sin_addr.s_addr = temp[3] << 24 | temp[2] << 16 | temp[1] << 8 | temp[0];
					addr->sin_port = temp[5] << 8 | temp[4];
					inet_show_sockaddr(addr);
					replylen = sprintf(reply, "200 port command successfull\r\n");
				}
				else
				{
					replylen = sprintf(reply, "501 argument error\r\n");
				}
			}
			else
			{
				pr_red_info("invalid command");
			}
			break;

		/* opts */
		case 0x7374706f:
		case 0x5354504f:
			if (state == FTP_STATE_LOGINED)
			{
				replylen = sprintf(reply, "200 opts command successfull\r\n");
			}
			else
			{
				pr_red_info("invalid command");
			}
			break;

		/* pwd */
		case 0x20647770:
		case 0x20445750:
			pr_bold_pos();
			if (state == FTP_STATE_LOGINED && getcwd(buff, sizeof(buff)))
			{
				replylen = sprintf(reply, "200 %s\r\n", buff);
			}
			else
			{
				pr_red_info("invalid command");
			}
			break;

		/* type */
		case 0x65707974:
		case 0x45505954:
			replylen = sprintf(reply, "220 commnd ok\r\n");
			file_type = buff[5];
			break;

		/* syst */
		case 0x74737973:
		case 0x54535953:
			replylen = sprintf(reply, "215 UNIX Type L8\r\n");
			break;

		/* cwd */
		case 0x20647763:
		case 0x20445743:
			ret = chdir(buff + 4);
			if (ret < 0)
			{
				replylen = sprintf(reply, "550 chdir failed\r\n");
			}
			else
			{
				replylen = sprintf(reply, "250 commnd ok\r\n");
			}
			break;

		/* list */
		case 0x7473696c:
		case 0x5453494c:
			if (recvlen == 6)
			{
				fp = pipe_command("ls");
			}
			else
			{
				fp = pipe_command("ls %s", buff + 5);
			}
			if (fp == NULL)
			{
				error_msg("pipe_command");
				replylen = sprintf(reply, "550 list directory failed\r\n");
				continue;
			}

			replylen = sprintf(reply, "150 list directory successfull\r\n");
			sendlen = send(sockfd, reply, recvlen, 0);
			if (sendlen < 0)
			{
				print_error("send");
				return sendlen;
			}

			data_sockfd = inet_create_tcp_link1(addr);
			if (data_sockfd < 0)
			{
				print_error("inet_create_tcp_link1");
				replylen = sprintf(reply, "425 can't open data link\r\n");
				continue;
			}

			ret = ffile_copy_simple(fileno(fp), data_sockfd);
			close(data_sockfd);
			fclose(fp);
			if (ret < 0)
			{
				error_msg("ffile_copy_simple");
				replylen = sprintf(reply, "425 can't open data link\r\n");
				continue;
			}

			replylen = sprintf(reply, "226 directory send OK\r\n");
			break;

		/* retr */
		case 0x72746572:
		case 0x52544552:
			ret = ftp_send_file(addr, file_type, NULL);
			break;

		/* stor */
		case 0x726f7473:
		case 0x524f5453:
			ret = ftp_receive_file(addr, file_type, NULL);
			break;

		/* noop */
		case 0x706f6f6e:
		case 0x504f4f4e:
		/* pasv */
		case 0x76736170:
		case 0x56534150:
		default:
			pr_red_info("unsupport command");
			replylen = sprintf(reply, "500 invalid command\r\n");
		}
	}

	return -1;
}

static void *ftp_service_handle(void *data)
{
	struct sockaddr_in client_addr;
	socklen_t addrlen;
	int sockfd;
	struct cavan_ftp_descriptor *desc = data;

	while (1)
	{
		sockfd = inet_accept(desc->ctrl_sockfd, &client_addr, &addrlen);
		if (sockfd < 0)
		{
			print_error("inet_accept");
			return NULL;
		}

		inet_show_sockaddr(&client_addr);
		ftp_service_cmdline(desc, sockfd, &client_addr);

		close(sockfd);
	}

	return NULL;
}

int ftp_service_run(u16 port, int count)
{
	int i;
	int ret;
	pthread_t services[count - 1];
	struct cavan_ftp_descriptor ftp_desc;

	ftp_desc.ctrl_sockfd = inet_create_tcp_service(port);
	if (ftp_desc.ctrl_sockfd < 0)
	{
		error_msg("inet_create_tcp_service");
		return ftp_desc.ctrl_sockfd;
	}

	ftp_desc.data_sockfd = inet_create_tcp_service(port - 1);
	if (ftp_desc.data_sockfd < 0)
	{
		error_msg("inet_create_tcp_service");
		ret = ftp_desc.data_sockfd;
		goto out_close_ctrl_sockfd;
	}

	for (i = count - 1; i >= 0; i--)
	{
		ret = pthread_create(services + i, NULL, ftp_service_handle, (void *)&ftp_desc);
		if (ret < 0)
		{
			print_error("pthread_create");
			goto out_close_data_sockfd;
		}
	}

	ftp_service_handle((void *)&ftp_desc);

	for (i = count - 1; i >= 0; i--)
	{
		pthread_join(services[i], NULL);
	}

	ret = 0;

out_close_data_sockfd:
	close(ftp_desc.data_sockfd);
out_close_ctrl_sockfd:
	close(ftp_desc.ctrl_sockfd);

	return ret;
}

int ftp_client_run(const char *ip_address, u16 port)
{
	int sockfd;
	char buff[1024], *p;
	ssize_t sendlen, recvlen;
	int ret;

	sockfd = inet_create_tcp_link2(ip_address, port);
	if (sockfd < 0)
	{
		error_msg("inet_create_tcp_link2");
		return sockfd;
	}

	while (1)
	{
		recvlen = recv(sockfd, buff, sizeof(buff), 0);
		if (recvlen < 0)
		{
			print_error("recv");
			ret = recvlen;
			break;
		}

		buff[recvlen] = 0;
		println("receive buff[%d] = %s", recvlen, buff);

		for (p = buff; (*p = getchar()) != '\n'; p++);

		if (p > buff)
		{
			p = text_copy(p, "\r\n");
		}
		else
		{
			*p = 0;
		}

		sendlen = send(sockfd, buff, p - buff, 0);
		if (sendlen < 0)
		{
			print_error("send");
			ret = sendlen;
			break;
		}

		println("send buff[%d] = %s", sendlen, buff);
	}

	close(sockfd);

	return ret;
}
