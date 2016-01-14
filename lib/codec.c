/*
 * File:			codec.c
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

static void cavan_lz77_get_node(u8 *win, u8 *data, u8 *data_end, struct cavan_lz77_node *node)
{
	u8 *win_end;
	u8 *win_bak = win;

	node->length = 0;
	node->offset = 0;
	node->value = *data;

	for (win_end = win + CAVAN_LZ77_WIN_SIZE; win < win_end; win++) {
		u8 *p, *pw;
		size_t length;

		for (p = data, pw = win; p < data_end && pw < win_end && *p == *pw; p++, pw++);

		length = p - data;
		if (length > node->length) {
			node->offset = win - win_bak;
			node->length = length;
			node->value = *p;
		}
	}
}

int cavan_lz77_encode(const char *src_file, const char *dest_file)
{
	int fd;
	int ret;
	void *data;
	size_t length;
	u8 *win, *p, *p_end;

	println("%s => %s", src_file, dest_file);

	ret = file_mmap(src_file, &data, &length, O_RDONLY);
	if (ret < 0) {
		pr_err_info("file_mmap");
		return ret;
	}

	fd = open(dest_file, O_WRONLY | O_CREAT, 0777);
	if (fd < 0) {
		pr_err_info("open");
		ret = fd;
		goto out_file_unmap;
	}

	ret = ffile_write(fd, data, CAVAN_LZ77_WIN_SIZE);
	if (ret < 0) {
		pr_err_info("ffile_write");
		goto out_close_fd;
	}

	win = data;
	p_end = ADDR_ADD(data, length);

	while (1) {
		struct cavan_lz77_node node;

		p = win + CAVAN_LZ77_WIN_SIZE;
		if (p >= p_end) {
			break;
		}

		cavan_lz77_get_node(win, p, p_end, &node);

		println("offset = %d, length = %d, value = 0x%02x", node.offset, node.length, node.value);

		ret = ffile_write(fd, &node, sizeof(node));
		if (ret < 0) {
			pr_err_info("ffile_write");
			goto out_close_fd;
		}

		win += node.length + 1;
	}

out_close_fd:
	close(fd);
out_file_unmap:
	file_unmap(fd, data, length);
	return ret;
}

int cavan_lz77_decode(const char *src_file, const char *dest_file)
{
	return 0;
}
