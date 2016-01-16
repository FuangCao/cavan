#pragma once

/*
 * File:			codec.h
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

#define CAVAN_LZ77_WIN_SIZE			0xFF
#define CAVAN_HUFFMAN_SYMBOLS		256
#define CAVAN_HUFFMAN_TREE_SIZE		(CAVAN_HUFFMAN_SYMBOLS * 2)

typedef u64 huffman_size_t;

#pragma pack(1)

struct cavan_lz77_node {
	u8 offset;
	u8 length;
	u8 value;
};

struct cavan_huffman_node_save {
	u8 value;
	huffman_size_t count;
};

#pragma pack()

struct cavan_lz77_window {
	u8 data[CAVAN_LZ77_WIN_SIZE];
	int head;
};

struct cavan_huffman_node {
	u8 value;
	u64 code;
	int length;
	huffman_size_t count;
};

struct cavan_huffman_tree_node {
	huffman_size_t count;
	struct cavan_huffman_node *node;
	struct cavan_huffman_tree_node *left, *right;
};

struct cavan_huffman_encoder {
	struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS];
	struct cavan_huffman_tree_node tree_nodes[CAVAN_HUFFMAN_TREE_SIZE];

	u8 remain;
	int offset;
	u64 code;
	int length;
	const u8 *data;
};

struct cavan_huffman_decoder {
	struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS];
	struct cavan_huffman_tree_node tree_nodes[CAVAN_HUFFMAN_TREE_SIZE];
	struct cavan_huffman_tree_node *root;
};

int cavan_lz77_encode(const void *data, size_t length, const char *dest_file);
int cavan_lz77_encode2(const char *src_file, const char *dest_file);
int cavan_lz77_decode(const char *src_file, const char *dest_file);

void cavan_huffman_build_freq_table(const u8 *mem, size_t size, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
int cavan_huffman_build_freq_table_fd(int fd, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
int cavan_huffman_build_freq_table_file(const char *pathname, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
struct cavan_huffman_tree_node *cavan_huffman_build_tree(struct cavan_huffman_tree_node tree_nodes[CAVAN_HUFFMAN_TREE_SIZE], struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
int cavan_huffman_write_freq_table(int fd, const struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
int cavan_huffman_read_freq_table(int fd, struct cavan_huffman_node nodes[CAVAN_HUFFMAN_SYMBOLS]);
int cavan_huffman_encode(int fd_src, int fd_dest);
int cavan_huffman_encode_file(const char *src_file, const char *dest_file);
int cavan_huffman_decode(int fd_src, int fd_dest);
int cavan_huffman_decode_file(const char *src_file, const char *dest_file);
