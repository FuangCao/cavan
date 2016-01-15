/*
 * File:		codec.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-14 12:16:39
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/codec.h>

static void cavan_lz77_get_node(const u8 *win, const u8 *data, const u8 *data_end, struct cavan_lz77_node *node)
{
	const u8 *win_end, *win_bak = win;

	node->length = 0;
	node->offset = 0;
	node->value = *data;

	for (win_end = win + CAVAN_LZ77_WIN_SIZE; win < win_end; win++) {
		size_t length;
		const u8 *p, *pw;

		for (p = data, pw = win; p < data_end && pw < win_end && *p == *pw; p++, pw++);

		length = p - data;
		if (length > node->length) {
			node->offset = win - win_bak;
			node->length = length;
			node->value = *p;
		}
	}
}

static void cavan_lz77_window_init(struct cavan_lz77_window *win, const u8 data[CAVAN_LZ77_WIN_SIZE])
{
	memcpy(win->data, data, CAVAN_LZ77_WIN_SIZE);
	win->head = 0;
}

static int cavan_lz77_window_append_data(struct cavan_lz77_window *win, const u8 *data, int length)
{
	int rlen;

	rlen = CAVAN_LZ77_WIN_SIZE - win->head;
	if (length > rlen) {
		memcpy(win->data + win->head, data, rlen);
		memcpy(win->data, data + rlen, length - rlen);
	} else {
		memcpy(win->data + win->head, data, length);
	}

	win->head = (win->head + length) % CAVAN_LZ77_WIN_SIZE;

	return length;
}

static int cavan_lz77_window_append_node(struct cavan_lz77_window *win, struct cavan_lz77_node *node, u8 *buff)
{
	int rlen;
	int offset = (win->head + node->offset) % CAVAN_LZ77_WIN_SIZE;

	rlen = CAVAN_LZ77_WIN_SIZE - offset;
	if (node->length > rlen) {
		memcpy(buff, win->data + offset, rlen);
		memcpy(buff + rlen, win->data, node->length - rlen);
	} else if (node->length > 0) {
		memcpy(buff, win->data + offset, node->length);
	}

	buff[node->length] = node->value;

	return cavan_lz77_window_append_data(win, buff, node->length + 1);
}

int cavan_lz77_encode(const void *data, size_t length, const char *dest_file)
{
	int fd;
	int ret;
	const u8 *win, *win_end, *data_end;

	fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open");
		return fd;
	}

	if (length <= CAVAN_LZ77_WIN_SIZE) {
		ret = ffile_write(fd, data, length);
		if (ret < 0) {
			pr_err_info("ffile_write");
		}

		goto out_close_fd;
	}

	ret = ffile_write(fd, data, CAVAN_LZ77_WIN_SIZE);
	if (ret < 0) {
		pr_err_info("ffile_write");
		goto out_close_fd;
	}

	win = data;
	data_end = win + length;
	win_end = data_end - CAVAN_LZ77_WIN_SIZE;

	while (1) {
		struct cavan_lz77_node node;

		cavan_lz77_get_node(win, win + CAVAN_LZ77_WIN_SIZE, data_end, &node);
		win += node.length + 1;

		if (win < win_end) {
			ret = ffile_write(fd, &node, sizeof(node));
			if (ret < 0) {
				pr_err_info("ffile_write");
				goto out_close_fd;
			}
		} else {
			if (win > win_end) {
				node.length--;
				node.value = data_end[-1];
			}

			ret = ffile_write(fd, &node, sizeof(node));
			if (ret < 0) {
				pr_err_info("ffile_write");
			}

			break;
		}
	}

out_close_fd:
	close(fd);
	return ret;
}

int cavan_lz77_encode2(const char *src_file, const char *dest_file)
{
	int fd;
	int ret;
	void *data;
	size_t length;

	println("%s => %s", src_file, dest_file);

	fd = file_mmap(src_file, &data, &length, O_RDONLY);
	if (fd < 0) {
		pr_err_info("file_mmap");
		return fd;
	}

	ret = cavan_lz77_encode(data, length, dest_file);
	file_unmap(fd, data, length);

	return ret;
}

int cavan_lz77_decode(const char *src_file, const char *dest_file)
{
	int ret;
	int fd_src;
	int fd_dest;
	u8 buff[CAVAN_LZ77_WIN_SIZE];
	struct cavan_lz77_window win;

	println("%s => %s", src_file, dest_file);

	fd_src = open(src_file, O_RDONLY);
	if (fd_src < 0) {
		pr_err_info("open file %s", src_file);
		return fd_src;
	}

	fd_dest = open(dest_file, O_WRONLY | O_TRUNC | O_CREAT, 0777);
	if (fd_dest < 0) {
		pr_err_info("open file %s", dest_file);
		ret = fd_dest;
		goto out_close_fd_src;
	}

	ret = ffile_read(fd_src, buff, sizeof(buff));
	if (ret < 0) {
		pr_err_info("ffile_read");
		goto out_close_fd_dest;
	}

	ret = ffile_write(fd_dest, buff, ret);
	if (ret < 0) {
		pr_err_info("ffile_write");
		goto out_close_fd_dest;
	}

	cavan_lz77_window_init(&win, buff);

	while (1) {
		int length;
		struct cavan_lz77_node node;

		ret = ffile_read(fd_src, &node, sizeof(node));
		if (ret <= 0) {
			if (ret == 0) {
				break;
			}

			pr_err_info("ffile_read");
			goto out_close_fd_dest;
		}

		// println("offset = %d, length = %d, value = %c", node.offset, node.length, node.value);

		length = cavan_lz77_window_append_node(&win, &node, buff);
		ret = ffile_write(fd_dest, buff, length);
		if (ret < 0) {
			pr_err_info("ffile_write");
			goto out_close_fd_dest;
		}
	}

out_close_fd_dest:
	close(fd_dest);
out_close_fd_src:
	close(fd_src);
	return ret;
}
