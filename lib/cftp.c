#include <cavan.h>
#include <cavan/cftp.h>
#include <cavan/progress.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-19 14:41:46

void cftp_descriptor_init(struct cftp_descriptor *desc)
{
	memset(desc, 0, sizeof(*desc));

	desc->max_xfer_length = CFTP_MIN_PACKAGE_LENGTH;
	desc->timeout_ms = CFTP_TIMEOUT_VALUE;
	desc->retry_count = CFTP_RETRY_COUNT;
}

static ssize_t cftp_send_data(struct cftp_descriptor *desc, const void *buff, size_t size)
{
	if (desc->send)
	{
		return desc->send(desc->data, buff, size);
	}
	else if (desc->send_timeout)
	{
		return desc->send_timeout(desc->data, buff, size, desc->timeout_ms);
	}
	else if (desc->fd > 0)
	{
		return write(desc->fd, buff, size);
	}
	else
	{
		return -EFAULT;
	}
}

static ssize_t cftp_receive_data(struct cftp_descriptor *desc, void *buff, size_t size)
{
	if (desc->receive)
	{
		return desc->receive(desc->data, buff, size);
	}
	else if (desc->receive_timeout)
	{
		return desc->receive_timeout(desc->data, buff, size, desc->timeout_ms);
	}
	else if (desc->fd > 0)
	{
		return read(desc->fd, buff, size);
	}
	else
	{
		return -EFAULT;
	}
}

static bool cftp_can_receive(struct cftp_descriptor *desc)
{
	if (desc->can_receive)
	{
		return desc->can_receive(desc->data, desc->timeout_ms);
	}
	else if (desc->fd > 0)
	{
		return file_poll_input(desc->fd, desc->timeout_ms);
	}
	else
	{
		return true;
	}
}

static ssize_t cftp_send_data_retry(struct cftp_descriptor *desc, const void *buff, size_t size, int retry)
{
	ssize_t sendlen;
	int ret;

	while (retry--)
	{
		sendlen = cftp_send_data(desc, buff, size);
		if (sendlen < 0)
		{
			return sendlen;
		}

		ret = cftp_can_receive(desc);
		if (ret < 0)
		{
			return ret;
		}
		else if (ret > 0)
		{
			return sendlen;
		}

		println("timeout retry = %d", retry);
	}

	return -ETIMEDOUT;
}

#if 0
static ssize_t cftp_receive_data_timeout(struct cftp_descriptor *desc, void *buff, size_t size)
{
	int ret;

	if (desc->receive_timeout)
	{
		return desc->receive_timeout(desc->data, buff, size, desc->timeout_value);
	}

	ret = cftp_can_receive(desc);
	if (ret <= 0)
	{
		return -ETIMEDOUT;
	}

	return cftp_receive_data(desc, buff, size);
}
#endif

static ssize_t cftp_send_error_message(struct cftp_descriptor *desc, struct cftp_error_message *msg, const char *fmt, ...)
{
	va_list ap;
	ssize_t sendlen;

	msg->type = CFTP_PACKAGE_ERROR;

	if (errno)
	{
		msg->err_code = errno;
	}
	else
	{
		msg->err_code = EFAULT;
	}

	va_start(ap, fmt);
	sendlen = vsprintf(msg->message, fmt, ap) + sizeof(*msg) + 1;
	va_end(ap);

	return cftp_send_data(desc, msg, sendlen);
}

static ssize_t cftp_send_ack_message(struct cftp_descriptor *desc, struct cftp_ack_message *msg, u16 blk_num, int retry)
{
	msg->type = CFTP_PACKAGE_ACK;
	msg->blk_num = blk_num;

	if (retry)
	{
		return cftp_send_data_retry(desc, msg, sizeof(*msg), retry);
	}
	else
	{
		return cftp_send_data(desc, msg, sizeof(*msg));
	}
}

static ssize_t cftp_send_file_reuest(struct cftp_descriptor *desc, struct cftp_file_request *req, const char *filename, struct stat *st, u32 offset, u32 size, int read)
{
	ssize_t sendlen;

	if (read)
	{
		req->type = CFTP_PACKAGE_FILE_READ;
	}
	else
	{
		req->type = CFTP_PACKAGE_FILE_WRITE;
		req->st_mode = st->st_mode;
		req->st_rdev = st->st_rdev;
	}

	req->size = size;
	req->offset = offset;

	sendlen = text_copy(req->filename, filename) - (char *) req + 1;

	return cftp_send_data_retry(desc, req, sendlen, desc->retry_count);
}

static void show_error_message(struct cftp_error_message *msg)
{
	println("Error package received: message = %s", msg->message);
	println("Error: %s", strerror(msg->err_code));
}

int cftp_client_receive_file(struct cftp_descriptor *desc, const char *file_in, u32 offset_in, const char *file_out, u32 offset_out, size_t size)
{
	int fd;
	int ret;
	ssize_t recvlen, sendlen;
	u16 blk_num;
	union cftp_message *msg;
	size_t max_xfer_length;

	fd = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		print_error("open file %s failed", file_out);
		return fd;
	}

	if (offset_out)
	{
		ret = lseek(fd, offset_out, SEEK_SET);
		if (ret < 0)
		{
			print_error("lseek");
			goto out_close_file;
		}
	}

	max_xfer_length = desc->max_xfer_length;

	msg = malloc(max_xfer_length);
	if (msg == NULL)
	{
		print_error("malloc");
		ret = -ENOMEM;
		goto out_close_file;
	}

	sendlen = cftp_send_file_reuest(desc, (void *) msg, file_in, NULL, offset_in, size, 1);
	if (sendlen < 0)
	{
		error_msg("cftp_send_data_retry");
		ret = sendlen;
		goto out_free_msg;
	}

	println("Remote@%s => Local@%s", file_in, file_out);
	println("seek = %s", size2text(offset_out));
	println("skip = %s", size2text(offset_in));
	println("size = %s", size2text(size));

	blk_num = 0;

	while (1)
	{
		recvlen = cftp_receive_data(desc, msg, max_xfer_length);
		if (recvlen < 0)
		{
			error_msg("cftp_receive_data");
			ret = recvlen;
			goto out_free_msg;
		}

		switch (msg->type)
		{
		case CFTP_PACKAGE_ERROR:
			show_error_message((struct cftp_error_message *)msg);
			ret = -EFAULT;
			goto out_free_msg;

		case CFTP_PACKAGE_DATA:
			if (msg->data_pkg.blk_num == blk_num)
			{
				sendlen = write(fd, msg->data_pkg.data, recvlen - sizeof(msg->data_pkg));
				if (sendlen < 0)
				{
					print_error("write");
					cftp_send_error_message(desc, (struct cftp_error_message *) msg, "write file failed");
					ret = sendlen;
					goto out_free_msg;
				}

				blk_num++;

				if ((blk_num & 0xFF) == 0)
				{
					print_char('.');
				}

				if ((size_t)recvlen < max_xfer_length)
				{
					println(" Receive data complete");
					cftp_send_ack_message(desc, (struct cftp_ack_message *) msg, blk_num, 0);
					ret = 0;
					goto out_free_msg;
				}
			}
			else
			{
				warning_msg("%d != %d", msg->data_pkg.blk_num, blk_num);
			}

			sendlen = cftp_send_ack_message(desc, (struct cftp_ack_message *) msg, blk_num, desc->retry_count);
			if (sendlen < 0)
			{
				error_msg("cftp_send_ack_message");
				ret = sendlen;
				goto out_free_msg;
			}
			break;

		default:
			error_msg("invalid package type");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "invalid package type");
			ret = -EINVAL;
			goto out_free_msg;
		}
	}

out_free_msg:
	free(msg);
out_close_file:
	close(fd);

	return ret;
}

static int cftp_client_send_directory(struct cftp_descriptor *desc, const char *dir_in, const char *dir_out)
{
	DIR *dp;
	struct dirent *entry;
	int ret;
	char tmp_file_in[256], tmp_file_out[256];
	char *p_in, *p_out;

	dp = opendir(dir_in);
	if (dp == NULL)
	{
		print_error("fail to open directory %s", dir_in);
		return -EFAULT;
	}

	p_in = text_path_cat(tmp_file_in, sizeof(tmp_file_in), dir_in, NULL);
	p_out = text_path_cat(tmp_file_out, sizeof(tmp_file_out), dir_out, NULL);

	while ((entry = readdir(dp)))
	{
		if (text_is_dot_name(entry->d_name))
		{
			continue;
		}

		text_copy(p_in, entry->d_name);
		text_copy(p_out, entry->d_name);

		ret = cftp_client_send_file(desc, tmp_file_in, 0, tmp_file_out, 0, 0);
		if (ret < 0)
		{
			goto out_close_dir;
		}
	}

	ret = 0;

out_close_dir:
	closedir(dp);

	return ret;
}

static int cftp_client_send_special_file(struct cftp_descriptor * desc, struct stat *st, struct cftp_file_request *req, ssize_t size)
{
	union cftp_message *msg;
	ssize_t recvlen, sendlen;

	req->type = CFTP_PACKAGE_FILE_WRITE;
	req->st_mode = st->st_mode;
	req->st_rdev = st->st_rdev;

	sendlen = cftp_send_data_retry(desc, req, sizeof(*req) + size, desc->retry_count);
	if (sendlen < 0)
	{
		error_msg("cftp_send_data_retry");
		return sendlen;
	}

	msg = (union cftp_message *) req;

	recvlen = cftp_receive_data(desc, msg, desc->max_xfer_length);
	if (recvlen < 0)
	{
		error_msg("cftp_receive_data");
		return recvlen;
	}

	switch (msg->type)
	{
	case CFTP_PACKAGE_ACK:
		return 0;

	case CFTP_PACKAGE_ERROR:
		show_error_message(&msg->err_msg);
		return -msg->err_msg.err_code;

	default:
		error_msg("unknown package type");
		return -EINVAL;
	}

	return 0;
}

int cftp_client_send_file(struct cftp_descriptor *desc, const char *file_in, u32 offset_in, const char *file_out, u32 offset_out, size_t size)
{
	int fd;
	int ret;
	struct stat st;
	ssize_t recvlen, sendlen, readlen;
	u16 blk_num;
	union cftp_message *msg;
	struct cftp_data_package *data_msg;
	size_t max_xfer_length, max_data_length;
	struct progress_bar bar;

	ret = file_lstat(file_in, &st);
	if (ret < 0)
	{
		error_msg("fstat");
		return ret;
	}

	max_xfer_length = desc->max_xfer_length;

	msg = malloc(max_xfer_length);
	if (msg == NULL)
	{
		print_error("malloc");
		return -ENOMEM;
	}

	switch (st.st_mode & S_IFMT)
	{
	case S_IFREG:
		println("Send File: %s => %s", file_in, file_out);
		break;

	case S_IFBLK:
	case S_IFCHR:
		println("Send Devicce: %s => %s", file_in, file_out);
		return cftp_client_send_special_file(desc, &st, &msg->file_req, text_copy(msg->file_req.filename, file_out) - msg->file_req.filename + 1);

	case S_IFLNK:
		println("Send Symlink: %s => %s", file_in, file_out);
		{
			char *p;

			p = text_copy(msg->file_req.filename, file_out) + 1;

			ret = readlink(file_in, p, 1024);
			if (ret < 0)
			{
				print_error("readlink");
				return ret;
			}

			return cftp_client_send_special_file(desc, &st, &msg->file_req, p - msg->file_req.filename + ret + 1);
		}

	case S_IFDIR:
		println("Send Directory: %s => %s", file_in, file_out);
		ret = cftp_client_send_special_file(desc, &st, &msg->file_req, text_copy(msg->file_req.filename, file_out) - msg->file_req.filename + 1);
		if (ret < 0)
		{
			error_msg("cftp_client_send_special_file");
			return ret;
		}
		return cftp_client_send_directory(desc, file_in, file_out);

	default:
		warning_msg("File %s type is unknown", file_in);
		return 0;
	}

	fd = open(file_in, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s failed", file_in);
		goto out_free_msg;
	}

	if (offset_in)
	{
		ret = lseek(fd, offset_in, SEEK_SET);
		if (ret < 0)
		{
			print_error("lseek");
			goto out_close_file;
		}
	}

	if (size == 0)
	{
		size = st.st_size - offset_in;
	}

	data_msg = malloc(max_xfer_length);
	if (data_msg == NULL)
	{
		print_error("malloc");
		ret = -ENOMEM;
		goto out_close_file;
	}

	sendlen = cftp_send_file_reuest(desc, (void *) msg, file_out, &st, offset_out, size, 0);
	if (sendlen < 0)
	{
		ret = sendlen;
		error_msg("cftp_send_data_retry");
		goto out_free_data_msg;
	}

	println("seek = %s", size2text(offset_out));
	println("skip = %s", size2text(offset_in));
	println("size = %s", size2text(size));

	blk_num = 0;
	data_msg->type = CFTP_PACKAGE_DATA;
	max_data_length = max_xfer_length - sizeof(*data_msg);
	progress_bar_init(&bar, size);
	readlen = max_data_length;

	while (1)
	{
		recvlen = cftp_receive_data(desc, msg, max_xfer_length);
		if (recvlen < 0)
		{
			error_msg("cftp_receive_data");
			ret = recvlen;
			goto out_free_data_msg;
		}

		switch (msg->type)
		{
		case CFTP_PACKAGE_ERROR:
			show_error_message((struct cftp_error_message *)msg);
			ret = -EFAULT;
			goto out_free_data_msg;

		case CFTP_PACKAGE_ACK:
			if (msg->ack_msg.blk_num == blk_num)
			{
				if ((size_t)readlen < max_data_length)
				{
					ret = 0;
					progress_bar_finish(&bar);
					println("Send data complete");
					goto out_free_data_msg;
				}

				readlen = read(fd, data_msg->data, max_data_length);
				if (readlen < 0)
				{
					print_error("read");
					cftp_send_error_message(desc, (struct cftp_error_message *) msg, "read file failed");
					ret = readlen;
					goto out_free_data_msg;
				}

				data_msg->blk_num = blk_num++;
				progress_bar_add(&bar, readlen);
			}
			else
			{
				warning_msg("%d != %d", msg->ack_msg.blk_num, blk_num);

				if (blk_num == 0)
				{
					continue;
				}
			}

			sendlen = cftp_send_data_retry(desc, data_msg, sizeof(*data_msg) + readlen, desc->retry_count);
			if (sendlen < 0)
			{
				error_msg("cftp_send_data_retry");
				goto out_free_data_msg;
			}
			break;

		default:
			error_msg("invalid package type");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "invalid package type");
			ret = -EINVAL;
			goto out_free_data_msg;
		}
	}

out_free_data_msg:
	free(data_msg);
out_close_file:
	close(fd);
out_free_msg:
	free(msg);

	return ret;
}

int cftp_server_receive_file(struct cftp_descriptor *desc, const char *filename, mode_t mode, u32 offset, size_t size)
{
	int fd;
	int ret;
	union cftp_message *msg;
	ssize_t recvlen, sendlen;
	u16 blk_num;
	struct progress_bar bar;
	size_t max_xfer_length;

	max_xfer_length = desc->max_xfer_length;

	msg = malloc(max_xfer_length);
	if (msg == NULL)
	{
		print_error("malloc");
		return -ENOMEM;
	}

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (fd < 0)
	{
		print_error("open file %s faild", filename);
		cftp_send_error_message(desc, (struct cftp_error_message *) msg, "open file %s faild", filename);
		ret = fd;
		goto out_free_msg;
	}

	if (offset)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0)
		{
			print_error("lseek");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "seek file %s faild", filename);
			goto out_free_msg;
		}
	}

	println("offset = %s", size2text(offset));
	println("size = %s", size2text(size));

	blk_num = 0;
	progress_bar_init(&bar, size);

	while (1)
	{
		sendlen = cftp_send_ack_message(desc, (struct cftp_ack_message *) msg, blk_num, desc->retry_count);
		if (sendlen < 0)
		{
			print_error("cftp_send_ack_message");
			ret = sendlen;
			goto out_close_file;
		}

		recvlen = cftp_receive_data(desc, msg, max_xfer_length);
		if (recvlen < 0)
		{
			print_error("cftp_receive_data");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "receive file failed");
			ret = recvlen;
			goto out_close_file;
		}

		switch (msg->type)
		{
		case CFTP_PACKAGE_ERROR:
			show_error_message((struct cftp_error_message *)msg);
			ret = -EFAULT;
			goto out_close_file;

		case CFTP_PACKAGE_DATA:
			if (msg->data_pkg.blk_num != blk_num)
			{
				warning_msg("blk_num %d != %d", msg->data_pkg.blk_num, blk_num);
				continue;
			}

			sendlen = write(fd, msg->data_pkg.data, recvlen - sizeof(msg->data_pkg));
			if (sendlen < 0)
			{
				print_error("write");
				cftp_send_error_message(desc, (struct cftp_error_message *) msg, "write file failed");
				ret = sendlen;
				goto out_close_file;
			}

			blk_num++;

			if ((size_t)recvlen < max_xfer_length)
			{
				cftp_send_ack_message(desc, (struct cftp_ack_message *) msg, blk_num, 0);
				progress_bar_finish(&bar);
				println("Receive data complete");
				ret = 0;
				goto out_close_file;
			}
			else
			{
				progress_bar_add(&bar, sendlen);
			}
			break;

		default:
			error_msg("invalid package type");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "invalid package type");
			ret = -EINVAL;
			goto out_close_file;
		}
	}

out_close_file:
	close(fd);
out_free_msg:
	free(msg);

	return ret;
}

int cftp_server_send_file(struct cftp_descriptor *desc, const char *filename, u32 offset, size_t size)
{
	int fd;
	int ret;
	union cftp_message *msg;
	struct cftp_data_package *data_pkg;
	ssize_t recvlen, sendlen, readlen;
	u16 blk_num;
	struct progress_bar bar;
	size_t max_xfer_length, max_data_length;
	struct stat st;

	max_xfer_length = desc->max_xfer_length;
	max_data_length = max_xfer_length - sizeof(msg->data_pkg);

	msg = malloc(max_xfer_length);
	if (msg == NULL)
	{
		print_error("malloc");
		return -ENOMEM;
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s faild", filename);
		cftp_send_error_message(desc, (struct cftp_error_message *) msg, "open file %s faild", filename);
		ret = fd;
		goto out_free_msg;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		print_error("fstat");
		cftp_send_error_message(desc, (struct cftp_error_message *) msg, "fstat");
		goto out_close_file;
	}

	if (offset)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0)
		{
			print_error("lseek");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "seek file %s faild", filename);
			goto out_close_file;
		}
	}

	if (size == 0)
	{
		size = st.st_size - offset;
	}

	data_pkg = malloc(max_xfer_length);
	if (data_pkg == NULL)
	{
		print_error("malloc");
		cftp_send_error_message(desc, (struct cftp_error_message *) msg, "malloc");
		ret = -ENOMEM;
		goto out_close_file;
	}

	println("filename = %s", filename);
	println("offset = %s", size2text(offset));
	println("size = %s", size2text(size));

	blk_num = 0;
	data_pkg->type = CFTP_PACKAGE_DATA;
	progress_bar_init(&bar, size);

	while (1)
	{
		readlen = read(fd, data_pkg->data, max_data_length);
		if (readlen < 0)
		{
			print_error("read file failed");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "read file failed");
			ret = readlen;
			goto out_free_data_pkg;
		}

		data_pkg->blk_num = blk_num++;

label_send_data:
		sendlen = cftp_send_data_retry(desc, data_pkg, sizeof(*data_pkg) + readlen, desc->retry_count);
		if (sendlen < 0)
		{
			error_msg("cftp_send_ack_message");
			ret = sendlen;
			goto out_free_data_pkg;
		}

		recvlen = cftp_receive_data(desc, msg, max_xfer_length);
		if (recvlen < 0)
		{
			error_msg("cftp_receive_data");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "receive file failed");
			ret = recvlen;
			goto out_free_data_pkg;
		}

		switch (msg->type)
		{
		case CFTP_PACKAGE_ERROR:
			show_error_message((struct cftp_error_message *)msg);
			ret = -EFAULT;
			goto out_free_data_pkg;

		case CFTP_PACKAGE_ACK:
			if (msg->ack_msg.blk_num != blk_num)
			{
				warning_msg("blk_num %d != %d", msg->ack_msg.blk_num, blk_num);
				goto label_send_data;
			}

			if ((size_t)readlen < max_data_length)
			{
				progress_bar_finish(&bar);
				println("Send data complete");
				ret = 0;
				goto out_free_data_pkg;
			}

			progress_bar_add(&bar, max_data_length);
			break;

		default:
			error_msg("invalid package type");
			cftp_send_error_message(desc, (struct cftp_error_message *) msg, "invalid package type");
			ret = -EINVAL;
			goto out_free_data_pkg;
		}
	}

out_free_data_pkg:
	free(data_pkg);
out_close_file:
	close(fd);
out_free_msg:
	free(msg);

	return ret;
}

static int cftp_receive_handle(struct cftp_descriptor *desc, struct cftp_file_request *req)
{
	int ret;

	switch (req->st_mode & S_IFMT)
	{
	case S_IFBLK:
	case S_IFCHR:
		println("Create Device: %s", req->filename);
		remove(req->filename);
		ret = mknod(req->filename, req->st_mode, req->st_rdev);
		break;

	case S_IFLNK:
		println("Create Symlink: %s", req->filename);
		remove(req->filename);
		ret = symlink(req->filename + text_len(req->filename) + 1, req->filename);
		if (ret < 0 && errno == EEXIST)
		{
			ret = 0;
		}
		break;

	case S_IFDIR:
		println("Create Directory: %s", req->filename);
		ret = mkdir(req->filename, req->st_mode);
		if (ret < 0 && errno == EEXIST)
		{
			ret = 0;
		}
		break;

	case S_IFREG:
		println("Receive Regular File: %s", req->filename);
		if (desc->receive_handle)
		{
			return desc->receive_handle(desc->data, req);
		}

		return cftp_server_receive_file(desc, req->filename, req->st_mode, req->offset, req->size);

	default:
		error_msg("unknown file type");
		cftp_send_error_message(desc, (struct cftp_error_message *) req, "unknown file type");
		return -EINVAL;
	}

	if (ret < 0)
	{
		cftp_send_error_message(desc, (struct cftp_error_message *) req, __FUNCTION__);
	}
	else
	{
		cftp_send_ack_message(desc, (struct cftp_ack_message *) req, 0, 0);
	}

	return ret;
}

static int cftp_send_handle(struct cftp_descriptor *desc, struct cftp_file_request *req)
{
	if (desc->send_handle)
	{
		return desc->send_handle(desc->data, req);
	}

	return cftp_server_send_file(desc, req->filename, req->offset, req->size);
}

static int cftp_command_handle(struct cftp_descriptor *desc, struct cftp_command_request *req)
{
	int ret;

	ret = system(req->command);
	if (ret < 0)
	{
		cftp_send_error_message(desc, (struct cftp_error_message *) req, "system");
	}
	else
	{
		cftp_send_ack_message(desc, (struct cftp_ack_message *) req, 0, 0);
	}

	return ret;
}

void *cftp_service_heandle(void *data)
{
	struct cftp_descriptor *desc = data;
	union cftp_message *msg;
	ssize_t recvlen;
	size_t max_xfer_length;

	max_xfer_length = desc->max_xfer_length;

	msg = malloc(max_xfer_length);
	if (msg == NULL)
	{
		print_error("malloc");
		return NULL;
	}

	while (1)
	{
		recvlen = cftp_receive_data(desc, msg, max_xfer_length);
		if (recvlen < 0)
		{
			print_error("cftp_receive_data");
			break;
		}

		switch (msg->type)
		{
		case CFTP_PACKAGE_FILE_READ:
			cftp_send_handle(desc, (struct cftp_file_request *) msg);
			break;

		case CFTP_PACKAGE_FILE_WRITE:
			cftp_receive_handle(desc, (struct cftp_file_request *) msg);
			break;

		case CFTP_PACKAGE_COMMAND:
			cftp_command_handle(desc, (struct cftp_command_request *) msg);
			break;

		default:
			pr_red_info("invalid request %d", msg->type);
		}
	}

	free(msg);

	return NULL;
}

ssize_t cftp_udp_receive_data(cavan_shared_data_t data, void *buff, size_t size)
{
	struct cftp_udp_link_descriptor *link = data.type_void;

	return inet_recvfrom(link->sockfd, buff, size, &link->client_addr, &link->addrlen);
}

ssize_t cftp_udp_send_data(cavan_shared_data_t data, const void *buff, size_t size)
{
	struct cftp_udp_link_descriptor *link = data.type_void;

	return inet_sendto(link->sockfd, buff, size, &link->client_addr);
}

ssize_t cftp_usb_receive_data(cavan_shared_data_t data, void *buff, size_t size)
{
	return cavan_usb_read_data(data.type_void, buff, size);
}

ssize_t cftp_usb_send_data(cavan_shared_data_t data, const void *buff, size_t size)
{
	return cavan_usb_write_data(data.type_void, buff, size);
}

ssize_t cftp_adb_receive_data(cavan_shared_data_t data, void *buff, size_t size)
{
	return cavan_adb_read_data(data.type_int, buff, size);
}

ssize_t cftp_adb_send_data(cavan_shared_data_t data, const void *buff, size_t size)
{
	return cavan_adb_write_data(data.type_int, buff, size);
}
