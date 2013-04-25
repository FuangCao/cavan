// Fuang.Cao <cavan.cfa@gmail.com> Thu Mar 31 15:39:22 CST 2011

#include <cavan.h>
#include <cavan/tftp.h>
#include <cavan/progress.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>
#include <cavan/network.h>

#define MAX_BUFF_LEN	MB(1)

static ssize_t send_error_pkg(int sockfd, int err_code, const char *err_msg, const struct sockaddr_in *remote_addr)
{
	struct tftp_error_pkg err_pkg;

	err_pkg.op_code = htons(TFTP_ERROR);
	err_pkg.err_code = htons(err_code);
	strcpy(err_pkg.err_msg, err_msg);

	return inet_sendto(sockfd, &err_pkg, strlen(err_pkg.err_msg) + 5, remote_addr);
}

static void show_error_msg_pkg(const struct tftp_error_pkg *err_pkg_p)
{
	if (ntohs(err_pkg_p->op_code) != TFTP_ERROR)
	{
		error_msg("this package is not a tftp error package");
		return;
	}

	println("err_code = %d, err_msg = %s", ntohs(err_pkg_p->err_code), err_pkg_p->err_msg);
}

static ssize_t send_ack_pkg(int sockfd, u16 blk_num, const struct sockaddr_in *remote_addr, int times)
{
	int ret;
	struct tftp_ack_pkg ack_pkg;

	ack_pkg.op_code = htons(TFTP_ACK);
	ack_pkg.blk_num = htons(blk_num);

	while (times--)
	{
		ret = inet_sendto(sockfd, &ack_pkg, sizeof(ack_pkg), remote_addr);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

static ssize_t send_ack_nosocket(u16 blk_num, const struct sockaddr_in *remote_addr, int times)
{
	int sockfd;
	ssize_t sendlen;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	sendlen = send_ack_pkg(sockfd, blk_num, remote_addr, times);

	close(sockfd);

	return sendlen;
}

static ssize_t send_ack_select(int sockfd, int retry, u16 blk_num, const struct sockaddr_in *remote_addr)
{
	struct tftp_ack_pkg ack_pkg;

	ack_pkg.op_code = htons(TFTP_ACK);
	ack_pkg.blk_num = htons(blk_num);

	return sendto_select(sockfd, retry, &ack_pkg, sizeof(ack_pkg), remote_addr);
}

static ssize_t send_request(int sockfd, const char *filename, const char *mode, int retry, struct sockaddr_in *remote_addr, int read)
{
	struct tftp_request_pkg req_pkg;
	int filename_len, mode_len;

	req_pkg.op_code = htons(read ? TFTP_RRQ : TFTP_WRQ);

	filename_len = strlen(filename);
	mode_len = strlen(mode);

	strcpy(req_pkg.filename, filename);
	strcpy(req_pkg.filename + filename_len + 1, mode);

	return sendto_select(sockfd, retry, &req_pkg, filename_len + mode_len + 2 + MEMBER_OFFSET(struct tftp_request_pkg, filename), remote_addr);
}

static ssize_t send_dd_request(int sockfd, const char *filename, const char *mode, int retry, u32 offset, u32 size, struct sockaddr_in *remote_addr, int read)
{
	struct tftp_dd_request_pkg dd_req_pkg;
	int filename_len, mode_len;

	dd_req_pkg.op_code = htons(read ? TFTP_DD_RRQ : TFTP_DD_WRQ);
	dd_req_pkg.offset = htonl(offset);
	dd_req_pkg.size = htonl(size);

	filename_len = strlen(filename);
	mode_len = strlen(mode);

	strcpy(dd_req_pkg.filename, filename);
	strcpy(dd_req_pkg.filename + filename_len + 1, mode);

	return sendto_select(sockfd, retry, &dd_req_pkg, filename_len + mode_len + 2 + MEMBER_OFFSET(struct tftp_dd_request_pkg, filename), remote_addr);
}

static int check_ack(const struct tftp_ack_pkg *ack_pkg_p, u16 blk_num)
{
	if (ntohs(ack_pkg_p->op_code) != TFTP_ACK)
	{
		error_msg("ack operation code error");
		return -EINVAL;
	}

	if (ntohs(ack_pkg_p->blk_num) != blk_num)
	{
		error_msg("block number error");
		return -EINVAL;
	}

	return 0;
}

int send_mkdir_request(const char *ip, u16 port, const char *pathname, mode_t mode)
{
	int sockfd;
	int ret;
	union tftp_pkg pkg;
	ssize_t recvlen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	pkg.op_code = htons(TFTP_MKDIR_REQ);
	pkg.mkdir.mode = mode;
	strcpy(pkg.mkdir.pathname, pathname);

	inet_sockaddr_init(&remote_addr, ip, port);

	recvlen = sendto_receive(sockfd, TFTP_TIMEOUT_VALUE, 1, &pkg, strlen(pathname) + 1 + MEMBER_OFFSET(struct tftp_mkdir_pkg, pathname), &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
	if (recvlen < 0)
	{
		ret = recvlen;
		goto out_close_socket;
	}

	ret = check_ack(&pkg.ack, 0);

out_close_socket:
	close(sockfd);

	return ret;
}

int vsend_command_request(const char *ip, u16 port, const char *command, va_list ap)
{
	int sockfd;
	int ret;
	union tftp_pkg pkg;
	ssize_t recvlen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	pkg.command.op_code = htons(TFTP_COMMAND_REQ);
	vsprintf(pkg.command.command, command, ap);

	inet_sockaddr_init(&remote_addr, ip, port);

	println("Send command \"%s\"", pkg.command.command);

	recvlen = sendto_receive(sockfd, TFTP_TIMEOUT_VALUE, 1, &pkg, strlen(pkg.command.command) + 1 + MEMBER_OFFSET(struct tftp_command_pkg, command), &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
	if (recvlen < 0)
	{
		ret = recvlen;
		goto out_close_socket;
	}

	ret = check_ack(&pkg.ack, 0);
	if (ret < 0)
	{
		goto out_close_socket;
	}

	ret = inet_recvfrom_timeout(sockfd, &pkg, sizeof(pkg), &remote_addr, &remote_addr_len, TFTP_COMMAND_EXEC_TIME);
	if (ret < 0)
	{
		goto out_close_socket;
	}

	ret = check_ack(&pkg.ack, 0);

out_close_socket:
	close(sockfd);

	return ret;
}

int send_command_request(const char *ip, u16 port, const char *command, ...)
{
	int ret;
	va_list ap;

	va_start(ap, command);
	ret = vsend_command_request(ip, port, command, ap);
	va_end(ap);

	return ret;
}

int send_command_request_show(const char *ip, u16 port, const char *command, ...)
{
	int cmd_ret, ret;
	va_list ap;
	const char *log_path = TFTP_COMMAND_LOG_FILE;

	va_start(ap, command);
	cmd_ret = vsend_command_request(ip, port, command, ap);
	va_end(ap);

	ret = tftp_client_receive_all(ip, port, log_path, log_path);
	if (ret < 0)
	{
		error_msg("receive command response failed");
		return ret;
	}

	ret = cat_file(log_path);
	if (ret < 0)
	{
		error_msg("cat file \"%s\" failed", log_path);
		return ret;
	}

	return cmd_ret;
}

int send_mknode_request(const char *ip, u16 port, const char *pathname, mode_t mode, dev_t dev)
{
	int sockfd;
	int ret;
	union tftp_pkg pkg;
	ssize_t recvlen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	pkg.mknode.op_code = htons(TFTP_MKNODE_REQ);
	pkg.mknode.mode = mode;
	pkg.mknode.dev = dev;
	strcpy(pkg.mknode.pathname, pathname);

	inet_sockaddr_init(&remote_addr, ip, port);

	println("Request mknode %s", pathname);

	recvlen = sendto_receive(sockfd, TFTP_TIMEOUT_VALUE, 1, &pkg, strlen(pathname) + 1 + MEMBER_OFFSET(struct tftp_mknode_pkg, pathname), &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
	if (recvlen < 0)
	{
		ret = recvlen;
		goto out_close_socket;
	}

	ret = check_ack(&pkg.ack, 0);

out_close_socket:
	close(sockfd);

	return ret;
}

int send_symlink_request(const char *ip, u16 port, const char *file_in, const char *file_out)
{
	int sockfd;
	int ret;
	union tftp_pkg pkg;
	int oldpath_len, newpath_len;
	ssize_t recvlen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;
	char *link;

	oldpath_len = strlen(file_out);
	link = pkg.symlink.pathname + oldpath_len + 1;

	newpath_len = readlink(file_in, link, sizeof(pkg) - MEMBER_OFFSET(struct tftp_symlink_pkg, pathname) - oldpath_len - 1);
	if (newpath_len < 0)
	{
		print_error("readlink \"%s\" failed", file_in);
		return newpath_len;
	}

	link[newpath_len] = 0;
	println("symlink = %s, newpath_len = %d", pkg.symlink.pathname + oldpath_len + 1, newpath_len);

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	pkg.symlink.op_code = htons(TFTP_SYMLINK_REQ);
	strcpy(pkg.symlink.pathname, file_out);

	inet_sockaddr_init(&remote_addr, ip, port);

	recvlen = sendto_receive(sockfd, TFTP_TIMEOUT_VALUE, 1, &pkg, oldpath_len + newpath_len + 2 + MEMBER_OFFSET(struct tftp_symlink_pkg, pathname), &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
	if (recvlen < 0)
	{
		ret = recvlen;
		goto out_close_socket;
	}

	ret = check_ack(&pkg.ack, 0);

out_close_socket:
	close(sockfd);

	return ret;
}

int tftp_client_receive_file(const char *ip, u16 port, const char *file_in, const char *file_out, u32 offset_in, u32 offset_out, u32 size)
{
	int sockfd, fd;
	int ret;
	ssize_t sendlen, recvlen, writelen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;
	union tftp_pkg pkg;
	u16 blk_num;

	fd = open(file_out, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0777);
	if (fd < 0)
	{
		print_error("open file \"%s\" failed", file_out);
		return fd;
	}

	if (offset_out)
	{
		ret = lseek(fd, offset_out, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek file \"%s\" failed", file_out);
			goto out_close_fd;
		}
	}

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("Can't create socket");
		ret = sockfd;
		goto out_close_fd;
	}

	inet_sockaddr_init(&remote_addr, ip, port);

	if (offset_in || size)
	{
		sendlen = send_dd_request(sockfd, file_in, "octet", TFTP_RETRY_COUNT, offset_in, size, &remote_addr, 1);
	}
	else
	{
		sendlen = send_request(sockfd, file_in, "octet", TFTP_RETRY_COUNT, &remote_addr, 1);
	}

	if (sendlen < 0)
	{
		error_msg("send read request failed");
		ret = sendlen;
		goto out_close_socket;
	}

	println("skip = %s", size2text(offset_in));
	println("seek = %s", size2text(offset_out));
	println("size = %s", size2text(size));
	println("%s @ %d : %s => %s", ip, port, file_in, file_out);

	blk_num = 1;

	while (1)
	{
		recvlen = inet_recvfrom(sockfd, &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
		if (recvlen < 0)
		{
			print_error("receive data timeout");
			ret = recvlen;
			goto out_close_socket;
		}

		switch (ntohs(pkg.op_code))
		{
		case TFTP_DATA:
			if (ntohs(pkg.data.blk_num) != blk_num)
			{
				warning_msg("blk_num != %d", blk_num);
				goto lable_send_ack;
			}

			writelen = write(fd, pkg.data.data, recvlen - 4);
			if (writelen < 0)
			{
				print_error("write data failed");
				ret = writelen;
				send_error_pkg(sockfd, 0, "write data failed", &remote_addr);
				goto out_close_socket;
			}

			print_char('.');

			blk_num++;

			if (writelen < TFTP_DATA_LEN)
			{
				println(" Receive data complete");
				send_ack_pkg(sockfd, blk_num, &remote_addr, TFTP_LAST_ACK_TIMES);
				ret = 0;
				goto out_success;
			}

lable_send_ack:
			sendlen = send_ack_select(sockfd, TFTP_RETRY_COUNT, blk_num, &remote_addr);
			if (sendlen < 0)
			{
				error_msg("send ack failed");
				ret = sendlen;
				goto out_close_socket;
			}
			break;

		case TFTP_ERROR:
			error_msg("error pkg received");
			show_error_msg_pkg(&pkg.err);
			ret = -1;
			goto out_close_socket;

		default:
			error_msg("unknown operation code");
			send_error_pkg(sockfd, 0, "unknown op_code", &remote_addr);
			ret = -1;
			goto out_close_socket;
		}
	}

out_success:
	ret = 0;
out_close_socket:
	close(sockfd);
out_close_fd:
	close(fd);

	return ret;
}

int tftp_client_receive_all(const char *ip, u16 port, const char *file_in, const char *file_out)
{
	char temp_name[512];

	if (file_test(file_out, "d") == 0)
	{
		sprintf(temp_name, "%s/%s", file_out, text_basename(file_in));
		file_out = temp_name;
	}

	return tftp_client_receive_file(ip, port, file_in, file_out, 0, 0, 0);
}

int tftp_client_send_file(const char *ip, u16 port, const char *file_in, const char *file_out, u32 offset_in, u32 offset_out, u32 size)
{
	int sockfd, fd;
	int ret;
	ssize_t readlen, recvlen, sendlen;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;
	union tftp_pkg pkg;
	struct tftp_data_pkg data_pkg;
	u16 blk_num;
	struct progress_bar bar;
	struct stat st;

	fd = open(file_in, O_RDONLY | O_BINARY);
	if (fd < 0)
	{
		print_error("open file \"%s\" failed", file_in);
		return fd;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		print_error("fstat failed");
		goto out_close_fd;

	}

	if (offset_in)
	{
		ret = lseek(fd, offset_in, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek file \"%s\" failed", file_in);
			goto out_close_fd;
		}
	}

	if (size == 0)
	{
		size = st.st_size;
		if (size <= offset_in)
		{
			warning_msg("no data to send");
			ret = 0;
			goto out_close_fd;
		}
		size -= offset_in;
	}

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("Can't create socket");
		ret = sockfd;
		goto out_close_fd;
	}

	inet_sockaddr_init(&remote_addr, ip, port);

	if (offset_out || size)
	{
		sendlen = send_dd_request(sockfd, file_out, "octet", TFTP_RETRY_COUNT, offset_out, size, &remote_addr, 0);
	}
	else
	{
		sendlen = send_request(sockfd, file_out, "octet", TFTP_RETRY_COUNT, &remote_addr, 0);
	}

	if (sendlen < 0)
	{
		error_msg("send write request failed");
		ret = sendlen;
		goto out_close_socket;
	}

	println("skip = %s", size2text(offset_in));
	println("seek = %s", size2text(offset_out));
	println("size = %s", size2text(size));
	println("%s => %s @ %d : %s", file_in, ip, port, file_out);

	blk_num = 0;
	readlen = TFTP_DATA_LEN;
	remote_addr_len = sizeof(remote_addr);
	progress_bar_init(&bar, size);

	while (1)
	{
		recvlen = inet_recvfrom(sockfd, &pkg, sizeof(pkg), &remote_addr, &remote_addr_len);
		if (recvlen < 0)
		{
			print_error("receive data failed");
			ret = recvlen;
			break;
		}

		switch (ntohs(pkg.op_code))
		{
		case TFTP_ACK:
			if (ntohs(pkg.ack.blk_num) != blk_num)
			{
				warning_msg("blk_num != %d", blk_num);
				goto lable_send_data;
			}

			if (readlen < TFTP_DATA_LEN)
			{
				progress_bar_finish(&bar);
				println("Send data complete");
				goto out_success;
			}

			readlen = read(fd, data_pkg.data, size > TFTP_DATA_LEN ? TFTP_DATA_LEN : size);
			if (readlen < 0)
			{
				print_error("read data failed");
				ret = readlen;
				send_error_pkg(sockfd, 0, "read data failed", &remote_addr);
				goto out_close_socket;
			}

			size -= readlen;
			blk_num++;
			progress_bar_add(&bar, readlen);

			data_pkg.op_code = htons(TFTP_DATA);
			data_pkg.blk_num = htons(blk_num);

lable_send_data:
			sendlen = sendto_select(sockfd, TFTP_RETRY_COUNT, &data_pkg, readlen + 4, &remote_addr);
			if (sendlen < 0)
			{
				error_msg("Send data timeout");
				ret = sendlen;
				goto out_close_socket;
			}
			break;

		case TFTP_ERROR:
			error_msg("error pkg received");
			show_error_msg_pkg(&pkg.err);
			ret = -1;
			goto out_close_socket;

		default:
			error_msg("unknown operation code");
			ret = -1;
			send_error_pkg(sockfd, 0, "unknown op_code", &remote_addr);
			goto out_close_socket;
		}
	}

out_success:
	ret = 0;
out_close_socket:
	close(sockfd);
out_close_fd:
	close(fd);

	return ret;
}

int tftp_client_send_directory(const char *ip, u16 port, const char *dir_in, const char *dir_out)
{
	int ret;
	DIR *src_dir;
	struct dirent *dt;
	char temp_name_in[512], temp_name_out[512];
	char *p_in, *p_out;

	src_dir = opendir(dir_in);
	if (src_dir == NULL)
	{
		print_error("open directory \"%s\" failed", dir_in);
		return -1;
	}

	ret = send_mkdir_request(ip, port, dir_out, 0777);
	if (ret < 0)
	{
		error_msg("send directory request failed");
		goto out_close_dir;
	}

	p_in = text_path_cat(temp_name_in, dir_in, NULL);
	p_out = text_path_cat(temp_name_out, dir_out, NULL);

	while (1)
	{
		dt = readdir(src_dir);
		if (dt == NULL)
		{
			break;
		}

		println("filename = %s", dt->d_name);

		if (dt->d_name[0] == '.')
		{
			if (dt->d_name[1] == 0)
			{
				continue;
			}

			if (dt->d_name[1] == '.' && dt->d_name[2] == 0)
			{
				continue;
			}
		}

		text_copy(p_in, dt->d_name);
		text_copy(p_out, dt->d_name);

		ret = tftp_client_send_all(ip, port, temp_name_in, temp_name_out);
		if (ret < 0)
		{
			error_msg("tftp_client_send_all failed");
			return ret;
		}

	}

	ret = 0;
out_close_dir:
	closedir(src_dir);

	return ret;

}

int tftp_client_send_all(const char *ip, u16 port, const char *file_in, const char *file_out)
{
	int ret;
	struct stat st;

	ret = file_lstat(file_in, &st);
	if (ret < 0)
	{
		print_error("get file \"%s\" size failed", file_in);
		return ret;;
	}

	switch (st.st_mode & S_IFMT)
	{
	case S_IFBLK:
	case S_IFCHR:
		return send_mknode_request(ip, port, file_out, st.st_mode, st.st_rdev);

	case S_IFLNK:
		return send_symlink_request(ip, port, file_in, file_out);

	case S_IFDIR:
		return tftp_client_send_directory(ip, port, file_in, file_out);

	case S_IFREG:
		return tftp_client_send_file(ip, port, file_in, file_out, 0, 0, 0);

	default:
		error_msg("unknown file type");
	}

	return -EINVAL;
}

int tftp_service_receive_data(const char *file_out, u32 offset_out, const char *file_mode, struct sockaddr_in *remote_addr)
{
	int ret;
	int sockfd, fd;
	ssize_t writelen, sendlen, recvlen;
	u16 blk_num;
	union tftp_pkg pkg;
	socklen_t remote_addr_len;

	println("Handle write request filename = %s, mode = %s", file_out, file_mode);

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	fd = open(file_out, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0777);
	if (fd < 0)
	{
		print_error("open file \"%s\" failed", file_out);
		send_error_pkg(sockfd, 0, "open file failed", remote_addr);
		ret = fd;
		goto out_close_socket;
	}

	if (offset_out)
	{
		ret = lseek(fd, offset_out, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek file \"%s\" failed", file_out);
			send_error_pkg(sockfd, 0, "seek file failed", remote_addr);
			goto out_close_fd;
		}
	}

	println("offset = %s", size2text(offset_out));
	println("%s @ %d => %s", inet_ntoa(remote_addr->sin_addr), ntohs(remote_addr->sin_port), file_out);

	if (file_test(file_out, "b") == 0)
	{
		umount_device(file_out, MNT_DETACH);
	}

	blk_num = 0;
	remote_addr_len = sizeof(*remote_addr);
	goto lable_send_ack;

	while (1)
	{
		if (writelen < TFTP_DATA_LEN)
		{
			send_ack_pkg(sockfd, blk_num, remote_addr, TFTP_LAST_ACK_TIMES);
			println(" Receive data complete");
			ret = 0;
			break;
		}

lable_send_ack:
		sendlen = send_ack_select(sockfd, TFTP_RETRY_COUNT, blk_num, remote_addr);
		if (sendlen < 0)
		{
			error_msg("send ACK failed");
			ret = sendlen;
			goto out_close_fd;
		}

		recvlen = inet_recvfrom(sockfd, &pkg, sizeof(pkg), remote_addr, &remote_addr_len);
		if (recvlen < 0)
		{
			print_error("receive data failed");
			ret = recvlen;
			break;
		}

		switch (ntohs(pkg.op_code))
		{
		case TFTP_DATA:
			if ((int)ntohs(pkg.data.blk_num) != blk_num + 1)
			{
				warning_msg("data blk_num != %d", blk_num + 1);
				goto lable_send_ack;
			}

			writelen = write(fd, pkg.data.data, recvlen - 4);
			if (writelen < 0)
			{
				print_error("write data failed");
				send_error_pkg(sockfd, 0, "write data failed", remote_addr);
				ret = writelen;
				goto out_close_fd;
			}

			blk_num++;

			print_char('.');
			break;

		case TFTP_ERROR:
			error_msg("recv error msg");
			show_error_msg_pkg(&pkg.err);
			ret = -EINVAL;
			goto out_close_fd;

		default:
			error_msg("unknown operation code");
			ret = -EINVAL;
			goto out_close_fd;
		}
	}

out_close_fd:
	close(fd);
out_close_socket:
	close(sockfd);

	return ret;
}


int tftp_service_send_data(const char *file_in, u32 offset_in, u32 size, const char *file_mode, struct sockaddr_in *remote_addr)
{
	int ret;
	int sockfd, fd;
	ssize_t readlen, recvlen;
	u16 blk_num;
	struct tftp_data_pkg data_pkg;
	union tftp_pkg pkg;
	struct stat st;
	struct progress_bar bar;
	socklen_t remote_addr_len;

	println("Handle read request filename = %s, mode = %s", file_in, file_mode);

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0)
	{
		print_error("create socket failed");
		return sockfd;
	}

	fd = open(file_in, O_RDONLY | O_BINARY);
	if (fd < 0)
	{
		print_error("open file \"%s\" failed", file_in);
		send_error_pkg(sockfd, 0, "open file failed", remote_addr);
		ret = fd;
		goto out_close_socket;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		print_error("get file \"%s\" stat failed", file_in);
		send_error_pkg(sockfd, 0, "open file failed", remote_addr);
		goto out_close_socket;
	}

	println("offset = %s", size2text(offset_in));
	println("%s => %s @ %d", file_in, inet_ntoa(remote_addr->sin_addr), ntohs(remote_addr->sin_port));

	if (offset_in)
	{
		ret = lseek(fd, offset_in, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek file failed");
			send_error_pkg(sockfd, 0, "seek file failed", remote_addr);
			goto out_close_fd;
		}
	}

	if (size == 0)
	{
		size = st.st_size;
		if (size < offset_in)
		{
			warning_msg("no data to send");
			ret = 0;
			goto out_close_fd;
		}
		size -= offset_in;
	}

	println("size = %s", size2text(size));

	blk_num = 1;
	remote_addr_len = sizeof(*remote_addr);
	progress_bar_init(&bar, size);

	while (1)
	{
		readlen = read(fd, data_pkg.data, size > TFTP_DATA_LEN ? TFTP_DATA_LEN : size);
		if (readlen < 0)
		{
			print_error("read data failed");
			send_error_pkg(sockfd, 0, "open file failed", remote_addr);
			ret = readlen;
			break;
		}

		data_pkg.op_code = htons(TFTP_DATA);
		data_pkg.blk_num = htons(blk_num);

lable_send_data:
		recvlen = sendto_receive(sockfd, TFTP_TIMEOUT_VALUE, 5, &data_pkg, readlen + 4, &pkg, sizeof(pkg), remote_addr, &remote_addr_len);
		if (recvlen < 0)
		{
			error_msg("timeout_senddata falied");
			ret = recvlen;
			goto out_close_fd;
		}

		switch (ntohs(pkg.op_code))
		{
		case TFTP_ACK:
			if ((int)ntohs(pkg.ack.blk_num) != blk_num + 1)
			{
				warning_msg("blk_num %d != %d", ntohs(pkg.ack.blk_num), blk_num + 1);
				goto lable_send_data;
			}

			progress_bar_add(&bar, readlen);

			if (readlen < TFTP_DATA_LEN)
			{
				progress_bar_finish(&bar);
				println("Send data complete");
				ret = 0;
				goto out_close_fd;
			}

			blk_num++;
			size -= readlen;
			break;

		case TFTP_ERROR:
			error_msg("recv error msg");
			show_error_msg_pkg(&pkg.err);
			ret = -EINVAL;
			goto out_close_fd;

		default:
			error_msg("unknown operation code");
			ret = -EINVAL;
			goto out_close_fd;
		}
	}

out_close_fd:
	close(fd);
out_close_socket:
	close(sockfd);

	return ret;
}

int tftp_mkdir(struct tftp_mkdir_pkg *mkdir_pkg_p, const struct sockaddr_in *remote_addr)
{
	int ret;

	println("Create directory %s", mkdir_pkg_p->pathname);

	ret = mkdir(mkdir_pkg_p->pathname, mkdir_pkg_p->mode);
	if (ret < 0 && errno != EEXIST)
	{
		print_error("create directory failed");
		send_ack_nosocket(1, remote_addr, TFTP_LAST_ACK_TIMES);
		return ret;
	}

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	return 0;
}

static int write_response_to(FILE *fp, const char *file_out)
{
	int ret;
	int fd_out;
	size_t readlen;
	ssize_t writelen;
	char buff[MAX_BUFF_LEN];

	fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_out < 0)
	{
		print_error("open file \"%s\" failed", file_out);
		return fd_out;
	}

	while (1)
	{
		readlen = fread(buff, 1, sizeof(buff), fp);
		if (readlen == 0)
		{
			break;
		}

		writelen = write(fd_out, buff, readlen);
		if (writelen < 0)
		{
			print_error("write");
			ret = writelen;
			goto out_close_out_file;
		}
	}

	if (ferror(fp))
	{
		ret = -1;
	}
	else
	{
		ret = 0;
	}

out_close_out_file:
	close(fd_out);

	return ret;
}

int tftp_command(struct tftp_command_pkg *command_pkg_p, const struct sockaddr_in *remote_addr)
{
	int ret;

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	ret = system_command("%s | tee %s", command_pkg_p->command, TFTP_COMMAND_LOG_FILE);
	if (ret < 0)
	{
		print_error("Excute command \"%s\" failed", command_pkg_p->command);
		send_ack_nosocket(1, remote_addr, TFTP_LAST_ACK_TIMES);
		return -1;
	}

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	return 0;
}

int tftp_command_pipe(struct tftp_command_pkg *command_pkg_p, const struct sockaddr_in *remote_addr)
{
	FILE *fp;

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	fp = pipe_command_verbose("%s", command_pkg_p->command);
	if (fp == NULL || write_response_to(fp, TFTP_COMMAND_LOG_FILE) < 0)
	{
		print_error("Excute command \"%s\" failed", command_pkg_p->command);
		send_ack_nosocket(1, remote_addr, TFTP_LAST_ACK_TIMES);
		return -1;
	}

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	return 0;
}

int tftp_mknode(struct tftp_mknode_pkg *mknode_pkg_p, const struct sockaddr_in *remote_addr)
{
	int ret;

#if __WORDSIZE == 64
	println("Create device %s, sizeof(dev_t) = %ld", mknode_pkg_p->pathname, sizeof(dev_t));
	println("dev = 0x%lx", mknode_pkg_p->dev);
#else
	println("Create device %s, sizeof(dev_t) = %d", mknode_pkg_p->pathname, sizeof(dev_t));
#if CONFIG_BUILD_FOR_ANDROID
	println("dev = 0x%x", mknode_pkg_p->dev);
#else
	println("dev = 0x%llx", mknode_pkg_p->dev);
#endif
#endif

	remove(mknode_pkg_p->pathname);

	ret = remknod(mknode_pkg_p->pathname, mknode_pkg_p->mode, mknode_pkg_p->dev);
	if (ret < 0)
	{
		print_error("create mknode failed");
		send_ack_nosocket(1, remote_addr, TFTP_LAST_ACK_TIMES);
		return ret;
	}

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	return 0;
}

int tftp_symlink(struct tftp_symlink_pkg *symlink_pkg_p, const struct sockaddr_in *remote_addr)
{
	int ret;

	println("Create symlink %s => %s", symlink_pkg_p->pathname, symlink_pkg_p->pathname + strlen(symlink_pkg_p->pathname) + 1);

	remove(symlink_pkg_p->pathname);

	ret = symlink(symlink_pkg_p->pathname + strlen(symlink_pkg_p->pathname) + 1, symlink_pkg_p->pathname);
	if (ret < 0)
	{
		print_error("create symlink failed");
		send_ack_nosocket(1, remote_addr, TFTP_LAST_ACK_TIMES);
		return ret;
	}

	send_ack_nosocket(0, remote_addr, TFTP_LAST_ACK_TIMES);

	return 0;
}
