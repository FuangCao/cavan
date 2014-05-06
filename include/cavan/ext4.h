#pragma once

/*
 * File:			ext4.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-05 10:24:28
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <ext2fs/ext2fs.h>

#define CAVAN_EXT4_BOOT_BLOCK_SIZE		1024

typedef enum
{
	WALKER_ACTION_CONTINUE,
	WALKER_ACTION_EOF,
	WALKER_ACTION_STOP
} walker_action_t;

struct ext4_extent_header
{
	u16 magic;
	u16 entries;
	u16 max_entries;
	u16 depth;
	u32 generations;
};

struct ext4_extent_index
{
	u32 block;
	u32 leaf_lo;
	u16 leaf_hi;
	u16 unused;
};

struct ext4_extent_leaf
{
	u32 block;
	u16 length;
	u16 start_hi;
	u32 start_lo;
};

struct cavan_ext4_fs
{
	void *hw_data;
	int hw_block_shift;
	u16 hw_block_size;

	int block_shift;
	u16 block_size;

	u16 hw_boot_block_count;
	int hw_blocks_per_block_shift;
	u16 hw_blocks_per_block_count;

	u32 group_count;
	u16 inode_size;
	u32 inodes_per_group;
	u32 inodes_per_block;
	u32 first_data_block;

	struct ext2_super_block super;
	struct ext2_group_desc *gdt32;
	struct ext4_group_desc *gdt64;

	ssize_t (*read_block)(struct cavan_ext4_fs *fs, size_t index, void *buff, size_t count);
	ssize_t (*write_block)(struct cavan_ext4_fs *fs, size_t index, const void *buff, size_t count);
};

struct cavan_ext4_file
{
	struct cavan_ext4_fs *fs;

	union
	{
		char inode_data[256];
		struct ext2_inode inode;
		struct ext2_inode_large inode_large;
	};
};

struct cavan_ext4_walker
{
	void *context;
	size_t count;
	size_t max_count;
	int (*put_block)(struct cavan_ext4_walker *walker, struct cavan_ext4_fs *fs, void *data, size_t count);
};

struct cavan_ext4_find_file_context
{
	bool found;
	u16 name_len;
	const char *filename;
	struct ext2_dir_entry_2 entry;
};

struct cavan_ext4_read_file_context
{
	void *buff;
	void *buff_end;
	struct cavan_ext4_file *file;
};

// ================================================================================

char *cavan_ext4_uuid_tostring(const u8 uuid[16], char *buff, size_t size);
void cavan_ext4_dump_ext4_extent_header(const struct ext4_extent_header *header);
void cavan_ext4_dump_ext4_extent_index(const struct ext4_extent_index *index);
void cavan_ext4_dump_ext4_extent_leaf(const struct ext4_extent_leaf *leaf);
void cavan_ext4_dump_ext4_extent_list(const struct ext4_extent_header *header);
void cavan_ext4_dump_ext2_acl_header(const struct ext2_acl_header *header);
void cavan_ext4_dump_ext2_acl_entry (const struct ext2_acl_entry *entry);
void cavan_ext4_dump_ext2_group_desc(const struct ext2_group_desc *desc);
void cavan_ext4_dump_ext4_group_desc(const struct ext4_group_desc *desc);
void cavan_ext4_dump_ext2_dx_root_info(const struct ext2_dx_root_info *info);
void cavan_ext4_dump_ext2_dx_entry(const struct ext2_dx_entry *entry);
void cavan_ext4_dump_ext2_dx_countlimit(const struct ext2_dx_countlimit *countlimit);
void cavan_ext4_dump_ext2_new_group_input(const struct ext2_new_group_input *input);
void cavan_ext4_dump_ext4_new_group_input(const struct ext4_new_group_input *input);
void cavan_ext4_dump_ext2_inode(const struct ext2_inode *inode);
void cavan_ext4_dump_ext2_inode_large(const struct ext2_inode_large *inode);
void cavan_ext4_dump_ext2_super_block(const struct ext2_super_block *super);
void cavan_ext4_dump_ext2_dir_entry(const struct ext2_dir_entry *entry);
void cavan_ext4_dump_ext2_dir_entry_2(const struct ext2_dir_entry_2 *entry);
void cavan_ext4_dump_mmp_struct(const struct mmp_struct *mmp);
void cavan_ext4_dump_gdt(struct cavan_ext4_fs *fs);

// ================================================================================

int cavan_ext4_init(struct cavan_ext4_fs *fs);
void cavan_ext4_deinit(struct cavan_ext4_fs *fs);
struct cavan_ext4_file *cavan_ext4_open_file(struct cavan_ext4_fs *fs, const char *pathname);
ssize_t cavan_ext4_read_file(struct cavan_ext4_file *file, void *buff, size_t size);
void cavan_ext4_close_file(struct cavan_ext4_file *file);
