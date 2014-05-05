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
	size_t hw_block_size;

	int block_shift;
	size_t block_size;

	size_t hw_boot_block_count;
	int hw_blocks_per_block_shift;
	size_t hw_blocks_per_block_count;

	size_t group_count;

	struct ext2_super_block super;
	struct ext2_group_desc *gdt32;
	struct ext4_group_desc *gdt64;

	ssize_t (*read_block)(struct cavan_ext4_fs *fs, size_t index, void *buff, size_t count);
	ssize_t (*write_block)(struct cavan_ext4_fs *fs, size_t index, const void *buff, size_t count);
};

struct cavan_ext4_file
{
	struct ext2_inode *inode;
	char pathname[1024];
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

static inline size_t cavan_ext4_get_block_hw_addr(struct cavan_ext4_fs *fs, size_t index)
{
	return (index << fs->hw_blocks_per_block_shift) + fs->hw_boot_block_count;
}

static inline ssize_t cavan_ext4_read_block(struct cavan_ext4_fs *fs, size_t index, void *buff, size_t count)
{
	index = cavan_ext4_get_block_hw_addr(fs, index);
	return fs->read_block(fs, index, buff, count << fs->hw_blocks_per_block_shift);
}

static inline ssize_t cavan_ext4_write_block(struct cavan_ext4_fs *fs, size_t index, const void *buff, size_t count)
{
	index = cavan_ext4_get_block_hw_addr(fs, index);
	return fs->write_block(fs, index, buff, count << fs->hw_blocks_per_block_shift);
}
