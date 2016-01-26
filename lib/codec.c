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

#define CAVAN_HUFFMAN_DEBUG		0

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

// ================================================================================

static void cavan_huffman_freq_table_init(struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	int i;

	for (i = 0; i < CAVAN_HUFFMAN_SYMBOLS; i++) {
		nodes[i].value = i;
		nodes[i].count = 0;
	}
}

void cavan_huffman_build_freq_table(const u8 *mem, size_t size, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	const u8 *mem_end;

	for (mem_end = mem + size; mem < mem_end; mem++) {
		nodes[*mem].count++;
	}
}

int cavan_huffman_build_freq_table_fd(int fd, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	ssize_t rdlen;
	u8 buff[1024];

	while (1) {
		rdlen = read(fd, buff, sizeof(buff));
		if (rdlen <= 0) {
			return rdlen;
		}

		cavan_huffman_build_freq_table(buff, rdlen, nodes);
	}

	return 0;
}

int cavan_huffman_build_freq_table_file(const char *pathname, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	int fd;
	int ret;

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pr_err_info("open file %s", pathname);
		return fd;
	}

	ret = cavan_huffman_build_freq_table_fd(fd, nodes);
	close(fd);

	return ret;
}

static void cavan_huffman_tree_sort(struct cavan_huffman_tree_node *tree_pnodes[CAVAN_HUFFMAN_SYMBOLS], int start, int end)
{
	int start_bak = start, end_bak = end;
	struct cavan_huffman_tree_node *middle = tree_pnodes[start];

	while (1) {
		for (; tree_pnodes[end]->node->count > middle->node->count && start < end; end--);
		if (start < end) {
			tree_pnodes[start++] = tree_pnodes[end];
		} else {
			break;
		}

		for (; tree_pnodes[start]->node->count < middle->node->count && start < end; start++);
		if (start < end) {
			tree_pnodes[end--] = tree_pnodes[start];
		} else {
			break;
		}
	}

	tree_pnodes[start] = middle;

	if (start - start_bak > 1) {
		cavan_huffman_tree_sort(tree_pnodes, start_bak, start - 1);
	}

	if (end_bak - end > 1) {
		cavan_huffman_tree_sort(tree_pnodes, end + 1, end_bak);
	}
}

static void cavan_huffman_tree_set_code(struct cavan_huffman_tree_node *root, u64 code, u8 length)
{
#if 0
	if (root->left) {
		cavan_huffman_tree_set_code(root->left, code << 1, length + 1);

		if (root->right) {
			cavan_huffman_tree_set_code(root->right, code << 1 | 1, length + 1);
		}
	} else if (root->right) {
		cavan_huffman_tree_set_code(root->right, code << 1 | 1, length + 1);
#else
	if (root->left) {
		cavan_huffman_tree_set_code(root->left, code << 1, length + 1);
		cavan_huffman_tree_set_code(root->right, code << 1 | 1, length + 1);
#endif
	} else {
		struct cavan_huffman_node *node = root->node;

#if 1
		int i;
		u64 code_new = 0;

		for (i = 0; i < length; i++) {
			if (code & BIT64(i)) {
				code_new |= BIT64(length - i - 1);
			}
		}

		node->code = code_new;
#else
		node->code = code;
#endif

		node->length = length;

#if CAVAN_HUFFMAN_DEBUG
		println("node[%d] = '%c', code = 0x%lx => 0x%lx, length = %d", node->value, node->value, code, node->code, node->length);
#endif
	}
}

struct cavan_huffman_tree_node *cavan_huffman_build_tree(struct cavan_huffman_tree_node tree_nodes[CAVAN_HUFFMAN_SYMBOLS], struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	int i;
	int index, root;
	struct cavan_huffman_tree_node *root_node;
	struct cavan_huffman_tree_node *tree_pnodes[CAVAN_HUFFMAN_TREE_SIZE];

	for (i = 0; i < CAVAN_HUFFMAN_SYMBOLS; i++) {
		tree_nodes[i].node = nodes + i;
		tree_pnodes[i] = tree_nodes + i;
	}

	cavan_huffman_tree_sort(tree_pnodes, 0, CAVAN_HUFFMAN_SYMBOLS - 1);

	for (index = 0; index < CAVAN_HUFFMAN_SYMBOLS && tree_pnodes[index]->node->count == 0; index++);

	for (i = index; i < CAVAN_HUFFMAN_SYMBOLS; i++) {
		struct cavan_huffman_node *node = tree_pnodes[i]->node;

		tree_pnodes[i]->left = NULL;
		tree_pnodes[i]->right = NULL;
		tree_pnodes[i]->count = node->count;

#if CAVAN_HUFFMAN_DEBUG
		println("tree[%d] = '%c', count = %ld", i, node->value, node->count);
#endif
	}

	root_node = tree_pnodes[index];

	for (root = CAVAN_HUFFMAN_SYMBOLS, index++; index < root; index += 2, root++) {
		struct cavan_huffman_tree_node *left = tree_pnodes[index - 1];
		struct cavan_huffman_tree_node *right = tree_pnodes[index];

		root_node = tree_nodes + root;
		root_node->left = left;
		root_node->right = right;
		root_node->count = left->count + right->count;

		for (i = root - 1; tree_pnodes[i]->count > root_node->count; i--) {
			tree_pnodes[i + 1] = tree_pnodes[i];
		}

		tree_pnodes[i + 1] = root_node;
	}

	return root_node;
}

int cavan_huffman_write_freq_table(int fd, const struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	int ret;
	int i, count;
	struct cavan_huffman_node_save save_nodes[CAVAN_HUFFMAN_SYMBOLS];

	for (i = 0, count = 0; i < CAVAN_HUFFMAN_SYMBOLS; i++) {
		const struct cavan_huffman_node *node = nodes + i;

		if (node->count > 0) {
			save_nodes[count].value = node->value;
			save_nodes[count].count = node->count;
			count++;
		}
	}

	ret = ffile_write(fd, &count, 2);
	if (ret < 0) {
		pr_err_info("ffile_write: %d", ret);
		return ret;
	}

	return ffile_write(fd, save_nodes, sizeof(struct cavan_huffman_node_save) * count);
}

int cavan_huffman_read_freq_table(int fd, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS])
{
	int ret;
	int i, count = 0;
	struct cavan_huffman_node_save save_nodes[CAVAN_HUFFMAN_SYMBOLS];

	ret = ffile_read(fd, &count, 2);
	if (ret < 0) {
		pr_err_info("ffile_read");
		return ret;
	}

	ret = ffile_read(fd, save_nodes, sizeof(struct cavan_huffman_node_save) * count);
	if (ret < 0) {
		pr_err_info("ffile_read");
		return ret;
	}

	for (i = 0; i < count; i++) {
		struct cavan_huffman_node *node = nodes + save_nodes[i].value;

		node->count = save_nodes[i].count;
	}

	return 0;
}

void cavan_huffman_encoder_init(struct cavan_huffman_encoder *encoder)
{
	cavan_huffman_freq_table_init(encoder->nodes);
}

int cavan_huffman_encode(int fd_src, int fd_dest)
{
	int ret;
	u8 value;
	int offset;
	struct cavan_huffman_tree_node *root;
	struct cavan_huffman_encoder encoder;

	if (lseek(fd_src, 0, SEEK_SET) != 0) {
		pr_err_info("lseek");
		return -EINVAL;
	}

	cavan_huffman_encoder_init(&encoder);

	ret = cavan_huffman_build_freq_table_fd(fd_src, encoder.nodes);
	if (ret < 0) {
		pr_red_info("cavan_huffman_build_freq_table_fd");
		return ret;
	}

	root = cavan_huffman_build_tree(encoder.tree_nodes, encoder.nodes);
	if (root->left == NULL && root->right == NULL) {
		cavan_huffman_tree_set_code(root, 0, 1);
	} else {
		cavan_huffman_tree_set_code(root, 0, 0);
	}

	if (lseek(fd_src, 0, SEEK_SET) != 0) {
		pr_err_info("lseek");
		return -EINVAL;
	}

	ret = cavan_huffman_write_freq_table(fd_dest, encoder.nodes);
	if (ret < 0) {
		pr_red_info("cavan_huffman_write_freq_table: %d", ret);
		return ret;
	}

	value = 0;
	offset = 0;

	while (1) {
		ssize_t rdlen;
		u8 buff[1024], *mem, *mem_end;

		rdlen = ffile_read(fd_src, buff, sizeof(buff));
		if (rdlen <= 0) {
			if (rdlen == 0) {
				break;
			}

			pr_err_info("ffile_read");
			return rdlen;
		}

		for (mem = buff, mem_end = mem + rdlen; mem < mem_end; mem++) {
			struct cavan_huffman_node *node = encoder.nodes + (*mem);
			u64 code = node->code;
			int length = node->length;

			while (1) {
				int remain = 8 - offset;

				value |= code << offset;
				if (remain > length) {
					offset += length;
					break;
				}

				ret = ffile_write(fd_dest, &value, 1);
				if (ret < 0) {
					pr_err_info("ffile_write");
					return ret;
				}

				value = 0;
				offset = 0;

				if (length == remain) {
					break;
				}

				length -= remain;
				code >>= remain;
			}
		}
	}

	if (offset) {
		ret = ffile_write(fd_dest, &value, 1);
		if (ret < 0) {
			pr_err_info("ffile_write");
			return ret;
		}
	}

	return 0;
}

int cavan_huffman_encode_file(const char *src_file, const char *dest_file)
{
	int ret;
	int fd_src, fd_dest;

	println("%s => %s", src_file, dest_file);

	fd_src = open(src_file, O_RDONLY);
	if (fd_src < 0) {
		pr_err_info("open file %s", src_file);
		return fd_src;
	}

	fd_dest = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dest < 0) {
		pr_err_info("open file %s", dest_file);
		ret = fd_dest;
		goto out_close_fd_src;
	}

	ret = cavan_huffman_encode(fd_src, fd_dest);

	close(fd_dest);
out_close_fd_src:
	close(fd_src);
	return ret;
}

void cavan_huffman_decoder_init(struct cavan_huffman_decoder *decoder)
{
	cavan_huffman_freq_table_init(decoder->nodes);
}

int cavan_huffman_decode(int fd_src, int fd_dest)
{
	int ret;
	struct cavan_huffman_tree_node *root;
	struct cavan_huffman_decoder decoder;

	if (lseek(fd_src, 0, SEEK_SET) != 0) {
		pr_err_info("lseek");
		return -EINVAL;
	}

	cavan_huffman_decoder_init(&decoder);

	ret = cavan_huffman_read_freq_table(fd_src, decoder.nodes);
	if (ret < 0) {
		pr_red_info("cavan_huffman_read_freq_table");
		return ret;
	}

	decoder.root = cavan_huffman_build_tree(decoder.tree_nodes, decoder.nodes);
	root = decoder.root;

	while (1) {
		ssize_t rdlen;
		u8 buff[1024], *mem, *mem_end;

		rdlen = ffile_read(fd_src, buff, sizeof(buff));
		if (rdlen <= 0) {
			if (rdlen == 0) {
				break;
			}

			pr_err_info("ffile_read");
			return rdlen;
		}

		for (mem = buff, mem_end = mem + rdlen; mem < mem_end; mem++) {
			int offset;
			u8 value = *mem;

			for (offset = 0; offset < 8; offset++) {
				struct cavan_huffman_tree_node *node;

				if (value & BIT8(offset)) {
					node = root->right;
				} else {
					node = root->left;
				}

				if (node->left) {
					root = node;
				} else {
					ret = ffile_write(fd_dest, &node->node->value, 1);
					if (ret < 0) {
						pr_err_info("ffile_write");
						return ret;
					}

					root = decoder.root;
				}
			}
		}
	}

	return 0;
}

int cavan_huffman_decode_file(const char *src_file, const char *dest_file)
{
	int ret;
	int fd_src, fd_dest;

	println("%s => %s", src_file, dest_file);

	fd_src = open(src_file, O_RDONLY);
	if (fd_src < 0) {
		pr_err_info("open file %s", src_file);
		return fd_src;
	}

	fd_dest = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_dest < 0) {
		pr_err_info("open file %s", dest_file);
		ret = fd_dest;
		goto out_close_fd_src;
	}

	ret = cavan_huffman_decode(fd_src, fd_dest);

	close(fd_dest);
out_close_fd_src:
	close(fd_src);
	return ret;
}
