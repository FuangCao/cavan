/*
 * File:		ext4.c
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
#include <cavan/math.h>
#include <cavan/ext4.h>

#define CAVAN_EXT4_DEBUG				0
#define CAVAN_DUMP_DIR_ENTRY_DETAIL		0

char *cavan_ext4_uuid_tostring(const u8 uuid[16], char *buff, size_t size)
{
	int i = 0;
	char *buff_bak = buff;
	char *buff_end = buff + size;
	int steps[] = {4, 2, 2, 2, 6};

	while (buff < buff_end)
	{
		int step = steps[i];

		while (buff < buff_end && step--)
		{
			buff += snprintf(buff, buff_end - buff, "%02x", *uuid++);
		}

		if (++i >= NELEM(steps))
		{
			break;
		}

		if (buff < buff_end)
		{
			*buff++ = '-';
		}
	}

	return buff_bak;
}

void cavan_ext4_dump_ext4_extent_header(const struct ext4_extent_header *header)
{
	print_sep(60);
	pr_bold_info("ext4 extent header %p", header);

	println("magic = 0x%04x", header->magic);
	println("entries = %d", header->entries);
	println("max_entries = %d", header->max_entries);
	println("depth = %d", header->depth);
	println("generations = %d", header->generations);
}

void cavan_ext4_dump_ext4_extent_index(const struct ext4_extent_index *index)
{
	print_sep(60);
	pr_bold_info("ext4 extent index %p", index);

	println("block = %d", index->block);
	println("leaf = %" PRINT_FORMAT_INT64, (u64) index->leaf_hi << 32 | index->leaf_lo);
}

void cavan_ext4_dump_ext4_extent_leaf(const struct ext4_extent_leaf *leaf)
{
	print_sep(60);
	pr_bold_info("ext4 extent leaf %p", leaf);

	println("block = %d", leaf->block);
	println("length = %d", leaf->length);
	println("start = %" PRINT_FORMAT_INT64, (u64) leaf->start_hi << 32 | leaf->start_lo);
}

void cavan_ext4_dump_ext4_extent_list(const struct ext4_extent_header *header)
{
	int i;

	cavan_ext4_dump_ext4_extent_header(header);

	if (header->depth > 0)
	{
		struct ext4_extent_index *p;

		for (i = 0, p = (struct ext4_extent_index *) (header + 1); i < header->entries; i++)
		{
			cavan_ext4_dump_ext4_extent_index(p + i);
			println("index = %d", i);
		}
	}
	else
	{
		struct ext4_extent_leaf *p;

		for (i = 0, p = (struct ext4_extent_leaf *) (header + 1); i < header->entries; i++)
		{
			cavan_ext4_dump_ext4_extent_leaf(p + i);
			println("index = %d", i);
		}
	}
}

void cavan_ext4_dump_ext2_acl_header(const struct ext2_acl_header *header)
{
	print_sep(60);
	pr_bold_info("ext4 acl header %p", header);

	println("aclh_size = %d", header->aclh_size);
	println("aclh_file_count = %d", header->aclh_file_count);
	println("aclh_acle_count = %d", header->aclh_acle_count);
	println("aclh_first_acle = %d", header->aclh_first_acle);
}

void cavan_ext4_dump_ext2_acl_entry	(const struct ext2_acl_entry *entry)
{
	print_sep(60);
	pr_bold_info("ext4 acl entry %p", entry);

	println("acle_size = %d", entry->acle_size);
	println("acle_perms = %d", entry->acle_perms);
	println("acle_type = %d", entry->acle_type);
	println("acle_tag = %d", entry->acle_tag);
	println("acle_pad1 = %d", entry->acle_pad1);
	println("acle_next = %d", entry->acle_next);
}

static void cavan_ext4_dump_ext2_group_desc_base(const struct ext2_group_desc *desc)
{
	println("bg_block_bitmap = %d", desc->bg_block_bitmap);
	println("bg_inode_bitmap = %d", desc->bg_inode_bitmap);
	println("bg_inode_table = %d", desc->bg_inode_table);
	println("bg_free_blocks_count = %d", desc->bg_free_blocks_count);
	println("bg_free_inodes_count = %d", desc->bg_free_inodes_count);
	println("bg_used_dirs_count = %d", desc->bg_used_dirs_count);
	println("bg_flags = %d", desc->bg_flags);
	println("bg_exclude_bitmap_lo = %d", desc->bg_exclude_bitmap_lo);
	println("bg_block_bitmap_csum_lo = %d", desc->bg_block_bitmap_csum_lo);
	println("bg_inode_bitmap_csum_lo = %d", desc->bg_inode_bitmap_csum_lo);
	println("bg_itable_unused = %d", desc->bg_itable_unused);
	println("bg_checksum = 0x%04x", desc->bg_checksum);
}

void cavan_ext4_dump_ext2_group_desc(const struct ext2_group_desc *desc)
{
	print_sep(60);
	pr_bold_info("ext2 group desc %p", desc);
	cavan_ext4_dump_ext2_group_desc_base(desc);
}

void cavan_ext4_dump_ext4_group_desc(const struct ext4_group_desc *desc)
{
	print_sep(60);
	pr_bold_info("ext4 group desc %p", desc);

	cavan_ext4_dump_ext2_group_desc_base((const struct ext2_group_desc *) desc);
	println("bg_block_bitmap_hi = %d", desc->bg_block_bitmap_hi);
	println("bg_inode_bitmap_hi = %d", desc->bg_inode_bitmap_hi);
	println("bg_inode_table_hi = %d", desc->bg_inode_table_hi);
	println("bg_free_blocks_count_hi = %d", desc->bg_free_blocks_count_hi);
	println("bg_free_inodes_count_hi = %d", desc->bg_free_inodes_count_hi);
	println("bg_used_dirs_count_hi = %d", desc->bg_used_dirs_count_hi);
	println("bg_itable_unused_hi = %d", desc->bg_itable_unused_hi);
	println("bg_exclude_bitmap_hi = %d", desc->bg_exclude_bitmap_hi);
	println("bg_block_bitmap_csum_hi = %d", desc->bg_block_bitmap_csum_hi);
	println("bg_inode_bitmap_csum_hi = %d", desc->bg_inode_bitmap_csum_hi);
	println("bg_reserved = %d", desc->bg_reserved);
}

void cavan_ext4_dump_ext2_dx_root_info(const struct ext2_dx_root_info *info)
{
	print_sep(60);
	pr_bold_info("ext2 dx root info %p", info);

	println("reserved_zero = %d", info->reserved_zero);
	println("hash_version = %d", info->hash_version);
	println("info_length = %d", info->info_length);
	println("indirect_levels = %d", info->indirect_levels);
	println("unused_flags = %d", info->unused_flags);
}

void cavan_ext4_dump_ext2_dx_entry(const struct ext2_dx_entry *entry)
{
	print_sep(60);
	pr_bold_info("ext4 extent leaf %p", entry);

	println("hash = %d", entry->hash);
	println("block = %d", entry->block);
}

void cavan_ext4_dump_ext2_dx_countlimit(const struct ext2_dx_countlimit *countlimit)
{
	print_sep(60);
	pr_bold_info("ext2 dx countlimit %p", countlimit);

	println("limit = %d", countlimit->limit);
	println("count = %d", countlimit->count);
}

void cavan_ext4_dump_ext2_new_group_input(const struct ext2_new_group_input *input)
{
	print_sep(60);
	pr_bold_info("ext2 new group input %p", input);

	println("group = %d", input->group);
	println("block_bitmap = %d", input->block_bitmap);
	println("inode_bitmap = %d", input->inode_bitmap);
	println("inode_table = %d", input->inode_table);
	println("blocks_count = %d", input->blocks_count);
	println("reserved_blocks = %d", input->reserved_blocks);
	println("unused = %d", input->unused);
}

void cavan_ext4_dump_ext4_new_group_input(const struct ext4_new_group_input *input)
{
	print_sep(60);
	pr_bold_info("ext4 new group input %p", input);

	println("group = %d", input->group);
	println("block_bitmap = %" PRINT_FORMAT_INT64, input->block_bitmap);
	println("inode_bitmap = %" PRINT_FORMAT_INT64, input->inode_bitmap);
	println("inode_table = %" PRINT_FORMAT_INT64, input->inode_table);
	println("blocks_count = %d", input->blocks_count);
	println("reserved_blocks = %d", input->reserved_blocks);
	println("unused = %d", input->unused);
}

static void cavan_ext4_dump_ext2_inode_base(const struct ext2_inode *inode)
{
	println("i_mode = %06o", inode->i_mode);
	println("i_uid = %d", inode->i_uid);
	println("i_size = %d", inode->i_size);
	println("i_atime = %d", inode->i_atime);
	println("i_ctime = %d", inode->i_ctime);
	println("i_mtime = %d", inode->i_mtime);
	println("i_dtime = %d", inode->i_dtime);
	println("i_gid = %d", inode->i_gid);
	println("i_links_count = %d", inode->i_links_count);
	println("i_blocks = %d", inode->i_blocks);
	println("i_flags = 0x%08x", inode->i_flags);

	if (inode->i_flags & EXT4_EXTENTS_FL)
	{
		cavan_ext4_dump_ext4_extent_list((struct ext4_extent_header *) inode->i_block);
	}
	else if (S_ISLNK(inode->i_mode))
	{
		println("i_block = %s", (char *) inode->i_block);
	}
	else
	{
		int i;

		for (i = 0; i < EXT2_N_BLOCKS; i++)
		{
			println("i_block[%d] = %d", i, inode->i_block[i]);
		}
	}

	println("i_generation = %d", inode->i_generation);
	println("i_file_acl = %d", inode->i_file_acl);
	println("i_size_high = %d", inode->i_size_high);
	println("i_faddr = %d", inode->i_faddr);
}

void cavan_ext4_dump_ext2_inode(const struct ext2_inode *inode)
{
	print_sep(60);
	pr_bold_info("ext2 inode %p", inode);
	cavan_ext4_dump_ext2_inode_base(inode);
}

void cavan_ext4_dump_ext2_inode_large(const struct ext2_inode_large *inode)
{
	print_sep(60);
	pr_bold_info("ext2 inode large %p", inode);

	cavan_ext4_dump_ext2_inode_base((const struct ext2_inode *) inode);

	println("i_extra_isize = %d", inode->i_extra_isize);
	println("i_checksum_hi = %d", inode->i_checksum_hi);
	println("i_ctime_extra = %d", inode->i_ctime_extra);
	println("i_mtime_extra = %d", inode->i_mtime_extra);
	println("i_atime_extra = %d", inode->i_atime_extra);
	println("i_crtime = %d", inode->i_crtime);
	println("i_crtime_extra = %d", inode->i_crtime_extra);
	println("i_version_hi = %d", inode->i_version_hi);
}

void cavan_ext4_dump_ext2_super_block(const struct ext2_super_block *super)
{
	int i;
	char buff[1024];

	print_sep(60);
	pr_bold_info("ext2 super block %p", super);

	println("s_inodes_count = %d", super->s_inodes_count);
	println("s_blocks_count = %d", super->s_blocks_count);
	println("s_r_blocks_count = %d", super->s_r_blocks_count);
	println("s_free_blocks_count = %d", super->s_free_blocks_count);
	println("s_free_inodes_count = %d", super->s_free_inodes_count);
	println("s_first_data_block = %d", super->s_first_data_block);
	println("s_log_block_size = %d", super->s_log_block_size);
	println("s_log_cluster_size = %d", super->s_log_cluster_size);
	println("s_blocks_per_group = %d", super->s_blocks_per_group);
	println("s_clusters_per_group = %d", super->s_clusters_per_group);
	println("s_inodes_per_group = %d", super->s_inodes_per_group);
	println("s_mtime = %d", super->s_mtime);
	println("s_wtime = %d", super->s_wtime);
	println("s_mnt_count = %d", super->s_mnt_count);
	println("s_max_mnt_count = %d", super->s_max_mnt_count);
	println("s_magic = 0x%04x", super->s_magic);
	println("s_state = %d", super->s_state);
	println("s_errors = %d", super->s_errors);
	println("s_minor_rev_level = %d", super->s_minor_rev_level);
	println("s_lastcheck = %d", super->s_lastcheck);
	println("s_checkinterval = %d", super->s_checkinterval);
	println("s_creator_os = %d", super->s_creator_os);
	println("s_rev_level = %d", super->s_rev_level);
	println("s_def_resuid = %d", super->s_def_resuid);
	println("s_def_resgid = %d", super->s_def_resgid);
	println("s_first_ino = %d", super->s_first_ino);
	println("s_inode_size = %d", super->s_inode_size);
	println("s_block_group_nr = %d", super->s_block_group_nr);
	println("s_feature_compat = 0x%08x", super->s_feature_compat);
	println("s_feature_incompat = 0x%08x", super->s_feature_incompat);
	println("s_feature_ro_compat = 0x%08x", super->s_feature_ro_compat);
	println("s_uuid[16] = %s", cavan_ext4_uuid_tostring(super->s_uuid, buff, sizeof(buff)));
	println("s_volume_name[16] = %s", super->s_volume_name);
	println("s_last_mounted[64] = %s", super->s_last_mounted);
	println("s_algorithm_usage_bitmap = %d", super->s_algorithm_usage_bitmap);
	println("s_prealloc_blocks = %d", super->s_prealloc_blocks);
	println("s_prealloc_dir_blocks = %d", super->s_prealloc_dir_blocks);
	println("s_reserved_gdt_blocks = %d", super->s_reserved_gdt_blocks);
	println("s_journal_uuid[16] = %s", cavan_ext4_uuid_tostring(super->s_journal_uuid, buff, sizeof(buff)));
	println("s_journal_inum = %d", super->s_journal_inum);
	println("s_journal_dev = %d", super->s_journal_dev);
	println("s_last_orphan = %d", super->s_last_orphan);

	for (i = 0; i < NELEM(super->s_hash_seed); i++)
	{
		println("s_hash_seed[%d] = 0x%08x", i, super->s_hash_seed[i]);
	}

	println("s_def_hash_version = %d", super->s_def_hash_version);
	println("s_jnl_backup_type = %d", super->s_jnl_backup_type);
	println("s_desc_size = %d", super->s_desc_size);
	println("s_default_mount_opts = %d", super->s_default_mount_opts);
	println("s_first_meta_bg = %d", super->s_first_meta_bg);
	println("s_mkfs_time = %d", super->s_mkfs_time);

	for (i = 0; i < NELEM(super->s_jnl_blocks); i++)
	{
		println("s_jnl_blocks[%d] = %d", i, super->s_jnl_blocks[i]);
	}

	println("s_blocks_count_hi = %d", super->s_blocks_count_hi);
	println("s_r_blocks_count_hi = %d", super->s_r_blocks_count_hi);
	println("s_free_blocks_hi = %d", super->s_free_blocks_hi);
	println("s_min_extra_isize = %d", super->s_min_extra_isize);
	println("s_want_extra_isize = %d", super->s_want_extra_isize);
	println("s_flags = %d", super->s_flags);
	println("s_raid_stride = %d", super->s_raid_stride);
	println("s_mmp_update_interval = %d", super->s_mmp_update_interval);
	println("s_mmp_block = %" PRINT_FORMAT_INT64, super->s_mmp_block);
	println("s_raid_stripe_width = %d", super->s_raid_stripe_width);
	println("s_log_groups_per_flex = %d", super->s_log_groups_per_flex);
	println("s_reserved_char_pad = %d", super->s_reserved_char_pad);
	println("s_reserved_pad = %d", super->s_reserved_pad);
	println("s_kbytes_written = %" PRINT_FORMAT_INT64, super->s_kbytes_written);
	println("s_snapshot_inum = %d", super->s_snapshot_inum);
	println("s_snapshot_id = %d", super->s_snapshot_id);
	println("s_snapshot_r_blocks_count = %" PRINT_FORMAT_INT64, super->s_snapshot_r_blocks_count);
	println("s_snapshot_list = %d", super->s_snapshot_list);
	println("s_error_count = %d", super->s_error_count);
	println("s_first_error_time = %d", super->s_first_error_time);
	println("s_first_error_ino = %d", super->s_first_error_ino);
	println("s_first_error_block = %" PRINT_FORMAT_INT64, super->s_first_error_block);
	// println("s_first_error_func[32] = %d", super->s_first_error_func[32]);
	println("s_first_error_line = %d", super->s_first_error_line);
	println("s_last_error_time = %d", super->s_last_error_time);
	println("s_last_error_ino = %d", super->s_last_error_ino);
	println("s_last_error_line = %d", super->s_last_error_line);
	println("s_last_error_block = %" PRINT_FORMAT_INT64, super->s_last_error_block);
	// println("s_last_error_func[32] = %d", super->s_last_error_func[32]);
	// println("s_mount_opts[64] = %d", super->s_mount_opts[64]);
	println("s_usr_quota_inum = %d", super->s_usr_quota_inum);
	println("s_grp_quota_inum = %d", super->s_grp_quota_inum);
	println("s_overhead_blocks = %d", super->s_overhead_blocks);
	// println("s_backup_bgs[2] = %d", super->s_backup_bgs[2]);
	// println("s_reserved[106] = %d", super->s_reserved[106]);
	println("s_checksum = %d", super->s_checksum);
}

void cavan_ext4_dump_ext2_dir_entry(const struct ext2_dir_entry *entry)
{
#if CAVAN_DUMP_DIR_ENTRY_DETAIL
	print_sep(60);
	pr_bold_info("ext2 dir entry %p", entry);

	println("inode = %d", entry->inode);
	println("rec_len = %d", entry->rec_len);
#endif

	println("name[%d] = %s", entry->name_len, entry->name);
}

void cavan_ext4_dump_ext2_dir_entry_2(const struct ext2_dir_entry_2 *entry)
{
#if CAVAN_DUMP_DIR_ENTRY_DETAIL
	print_sep(60);
	pr_bold_info("ext2 dir entry2 %p", entry);

	println("inode = %d", entry->inode);
	println("rec_len = %d", entry->rec_len);
	println("file_type = %d", entry->file_type);
#endif

	println("name[%d] = %s", entry->name_len, text_header(entry->name, entry->name_len));
}

void cavan_ext4_dump_mmp_struct(const struct mmp_struct *mmp)
{
	print_sep(60);
	pr_bold_info("mmap struct %p", mmp);

	println("mmp_magic = 0x%08x", mmp->mmp_magic);
	println("mmp_seq = %d", mmp->mmp_seq);
	println("mmp_time = %" PRINT_FORMAT_INT64, mmp->mmp_time);
	println("mmp_nodename[64] = %s", mmp->mmp_nodename);
	println("mmp_bdevname[32] = %s", mmp->mmp_bdevname);
	println("mmp_check_interval = %d", mmp->mmp_check_interval);
	println("mmp_pad1 = %d", mmp->mmp_pad1);
	// println("mmp_pad2[227] = %d", mmp->mmp_pad2[227]);
}

void cavan_ext4_dump_gdt(struct cavan_ext4_fs *fs)
{
	int i;

	if (fs->gdt32)
	{
		for (i = 0; i < (int) fs->group_count; i++)
		{
			cavan_ext4_dump_ext2_group_desc(fs->gdt32 + i);
			println("index = %d", i);
		}
	}
	else
	{
		for (i = 0; i < (int) fs->group_count; i++)
		{
			cavan_ext4_dump_ext4_group_desc(fs->gdt64 + i);
			println("index = %d", i);
		}
	}
}

// ================================================================================

static inline size_t cavan_ext4_get_block_hw_addr(struct cavan_ext4_fs *fs, size_t index)
{
	return ((index - fs->first_data_block) << fs->hw_blocks_per_block_shift) + fs->hw_boot_block_count;
}

static inline ssize_t cavan_ext4_read_block(struct cavan_ext4_fs *fs, size_t index, void *buff, size_t count)
{
	index = cavan_ext4_get_block_hw_addr(fs, index);
	return fs->bdev->read_block(fs->bdev, index, buff, count << fs->hw_blocks_per_block_shift);
}

static inline ssize_t cavan_ext4_write_block(struct cavan_ext4_fs *fs, size_t index, const void *buff, size_t count)
{
	index = cavan_ext4_get_block_hw_addr(fs, index);
	return fs->bdev->write_block(fs->bdev, index, buff, count << fs->hw_blocks_per_block_shift);
}

static struct ext2_group_desc *cavan_ext4_get_group(struct cavan_ext4_fs *fs, u32 index)
{
	if (fs->gdt32)
	{
		return fs->gdt32 + index;
	}

	return (struct ext2_group_desc *) (fs->gdt64 + index);
}

static inline u64 cavan_ext4_inode_index_to_group(struct cavan_ext4_fs *fs, u32 index)
{
	return index / fs->inodes_per_group;
}

static inline u32 cavan_ext4_inode_index_to_table(struct cavan_ext4_fs *fs, u32 index)
{
	u32 group = cavan_ext4_inode_index_to_group(fs, index);

	return cavan_ext4_get_group(fs, group)->bg_inode_table;
}

static inline int cavan_ext4_get_dir_entry_length(struct ext2_dir_entry_2 *entry)
{
	return CAVAN_EXT4_DIR_ENTRY_HEADER_LEN + entry->name_len;
}

ssize_t cavan_ext4_read_inode(struct cavan_ext4_fs *fs, u32 index, struct ext2_inode_large *inode)
{
	u32 table = cavan_ext4_inode_index_to_table(fs, index);
	u64 offset = ((index - 1) % fs->inodes_per_group) * fs->inode_size;

	return fs->bdev->read_byte(fs->bdev, cavan_ext4_get_block_hw_addr(fs, table) + (offset >> fs->bdev->block_shift), offset & fs->bdev->block_mask, inode, fs->inode_size);
}

static inline ssize_t cavan_ext4_read_super_block(struct cavan_ext4_fs *fs, struct ext2_super_block *super)
{
	return fs->bdev->read_byte(fs->bdev, CAVAN_EXT4_BOOT_BLOCK_SIZE >> fs->bdev->block_shift, 0, super, sizeof(*super));
}

static void *cavan_ext4_read_gdt(struct cavan_ext4_fs *fs)
{
	void *gdt;
	size_t size;
	ssize_t rdlen;

	size = fs->super.s_desc_size * fs->group_count;
	gdt = malloc(size);
	if (gdt == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	rdlen = fs->bdev->read_byte(fs->bdev, cavan_ext4_get_block_hw_addr(fs, fs->first_data_block + 1), 0, gdt, size);
	if (rdlen < 0)
	{
		pr_red_info("fs->read_block");
		goto out_free_gdt;
	}

	return gdt;

out_free_gdt:
	free(gdt);
	return NULL;
}

int cavan_ext4_init(struct cavan_ext4_fs *fs, struct cavan_block_device *bdev)
{
	int ret;
	void *gdt;
	struct ext2_super_block *super = &fs->super;

	fs->bdev = bdev;

	ret = cavan_ext4_read_super_block(fs, super);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_read_super_block");
		return ret;
	}

	if (super->s_magic != EXT2_SUPER_MAGIC)
	{
		pr_red_info("invalid magic number 0x%04x", super->s_magic);
		return -EINVAL;
	}

	if ((super->s_feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) == 0 || super->s_desc_size == 0)
	{
		super->s_desc_size = EXT2_MIN_DESC_SIZE;
	}

#if CAVAN_EXT4_DEBUG
	cavan_ext4_dump_ext2_super_block(super);
#endif

	fs->block_shift = super->s_log_block_size + 10;
	fs->block_size = 1 << fs->block_shift;
	fs->block_mask = fs->block_size - 1;

#if CAVAN_EXT4_DEBUG
	println("block_shift = %d", fs->block_shift);
	println("block_size = %d", fs->block_size);
#endif

	if (fs->block_size < fs->bdev->block_size)
	{
		pr_red_info("block device block size to large");
		return -EINVAL;
	}

	fs->hw_boot_block_count = fs->block_size > CAVAN_EXT4_BOOT_BLOCK_SIZE ? 0 : (CAVAN_EXT4_BOOT_BLOCK_SIZE >> fs->bdev->block_shift);
	fs->hw_boot_block_size = fs->hw_boot_block_count << fs->bdev->block_shift;
	fs->hw_blocks_per_block_shift = fs->block_shift - fs->bdev->block_shift;
	fs->hw_blocks_per_block_count = 1 << fs->hw_blocks_per_block_shift;
	fs->group_count = DIV_CEIL(super->s_blocks_count - super->s_first_data_block, super->s_blocks_per_group);

#if CAVAN_EXT4_DEBUG
	println("hw_boot_block_count = %d", fs->hw_boot_block_count);
	println("hw_blocks_per_block_shift = %d", fs->hw_blocks_per_block_shift);
	println("hw_blocks_per_block_count = %d", fs->hw_blocks_per_block_count);
	println("group_count = %d", fs->group_count);
#endif

	gdt = cavan_ext4_read_gdt(fs);
	if (gdt == NULL)
	{
		pr_red_info("cavan_ext4_read_gdt");
		return -EIO;
	}

	if (super->s_desc_size < EXT2_MIN_DESC_SIZE_64BIT)
	{
		fs->gdt32 = gdt;
		fs->gdt64 = NULL;
	}
	else
	{
		fs->gdt64 = gdt;
		fs->gdt32 = NULL;
	}

#if CAVAN_EXT4_DEBUG
	cavan_ext4_dump_gdt(fs);
#endif

	fs->first_data_block = super->s_first_data_block;
	fs->inode_size = super->s_inode_size;
	fs->inodes_per_group = super->s_inodes_per_group;
	fs->inodes_per_block = fs->block_size / fs->inode_size;

	return 0;
}

void cavan_ext4_deinit(struct cavan_ext4_fs *fs)
{
	if (fs->gdt64)
	{
		free(fs->gdt64);
	}
	else
	{
		free(fs->gdt32);
	}
}

static int cavan_ext4_walker_write_block(struct cavan_ext4_walker *walker, void *block, size_t count)
{
	size_t size;

	size = count << walker->fs->block_shift;
	if (size > walker->remain)
	{
		size = walker->remain;
		if (size == 0)
		{
			return WALKER_ACTION_EOF;
		}
	}

	walker->remain -= size;

	if (walker->skip > 0)
	{
		if (size <= walker->skip)
		{
			walker->skip -= size;
			return WALKER_ACTION_CONTINUE;
		}

		block = ADDR_ADD(block, walker->skip);
		size -= walker->skip;
		walker->skip = 0;
	}

	return walker->write_byte(walker, block, size);
}

static int cavan_ext4_traversal_extent(struct cavan_ext4_walker *walker, struct ext4_extent_header *header)
{
	int ret;
	u64 start;
	ssize_t rdlen;

	if (header->magic != CAVAN_EXT4_EXTENT_MAGIC)
	{
		pr_red_info("invalid magic 0x%04x", header->magic);
		return -EINVAL;
	}

	if (header->depth > 0)
	{
		struct ext4_extent_index *index_end;
		struct ext4_extent_index *index = (struct ext4_extent_index *) (header + 1);

		for (index_end = index + header->entries; index < index_end; index++)
		{
			char buff[walker->fs->block_size];

			start = (u64) index->leaf_hi << 32 | index->leaf_lo;
			rdlen = cavan_ext4_read_block(walker->fs, start, buff, 1);
			if (rdlen < 0)
			{
				pr_red_info("cavan_ext4_read_block");
				return rdlen;
			}

			ret = cavan_ext4_traversal_extent(walker, (struct ext4_extent_header *) buff);
			if (ret != WALKER_ACTION_CONTINUE)
			{
				return ret;
			}
		}
	}
	else
	{
		struct ext4_extent_leaf *leaf_end;
		struct ext4_extent_leaf *leaf = (struct ext4_extent_leaf *) (header + 1);

		for (leaf_end = leaf + header->entries; leaf < leaf_end; leaf++)
		{
			u64 end;
			char buff[walker->fs->block_size];

			start = (u64) leaf->start_hi << 32 | leaf->start_lo;
			for (end = start + leaf->length; start < end; start++)
			{
				rdlen = cavan_ext4_read_block(walker->fs, start, buff, 1);
				if (rdlen < 0)
				{
					pr_red_info("cavan_ext4_read_block");
					return rdlen;
				}

				ret = cavan_ext4_walker_write_block(walker, buff, 1);
				if (ret != WALKER_ACTION_CONTINUE)
				{
					return ret;
				}
			}
		}
	}

	return WALKER_ACTION_CONTINUE;
}

static int cavan_ext4_traversal_indirect(struct cavan_ext4_walker *walker, u32 *blocks, size_t count, int level)
{
	u32 *block_end;

	for (block_end = blocks + count; blocks < block_end && *blocks; blocks++)
	{
		int ret;
		ssize_t rdlen;
		char buff[walker->fs->block_size];

		rdlen = cavan_ext4_read_block(walker->fs, *blocks, buff, 1);
		if (rdlen < 0)
		{
			pr_red_info("cavan_ext4_read_block");
			return rdlen;
		}

		if (level > 0)
		{
			ret = cavan_ext4_traversal_indirect(walker, (u32 *) buff, walker->fs->block_size >> 2, level - 1);
		}
		else
		{
			ret = cavan_ext4_walker_write_block(walker, buff, 1);
		}

		if (ret != WALKER_ACTION_CONTINUE)
		{
			return ret;
		}
	}

	return WALKER_ACTION_CONTINUE;
}

static int cavan_ext4_traversal_direct_indirect(struct cavan_ext4_walker *walker, u32 *blocks)
{
	int i;
	static const char steps[] = {12, 1, 1, 1};

	for (i = 0; i < NELEM(steps); i++)
	{
		int ret;

		ret = cavan_ext4_traversal_indirect(walker, blocks, steps[i], i);
		if (ret != WALKER_ACTION_CONTINUE)
		{
			return ret;
		}

		blocks += steps[i];
	}

	return 0;
}

static int cavan_ext4_traversal_inode(struct cavan_ext4_fs *fs, struct ext2_inode_large *inode, size_t skip, struct cavan_ext4_walker *walker, void *context)
{
	walker->fs = fs;
	walker->inode = inode;
	walker->context = context;

	walker->skip = skip;
	walker->remain = inode->i_size;

	if (walker->remain == 0)
	{
		if (walker->remain <= sizeof(inode->i_block))
		{
			return walker->write_byte(walker, inode->i_block, walker->remain);
		}

		return 0;
	}
	else if (inode->i_flags & EXT4_EXTENTS_FL)
	{
		return cavan_ext4_traversal_extent(walker, (struct ext4_extent_header *) inode->i_block);
	}
	else
	{
		return cavan_ext4_traversal_direct_indirect(walker, inode->i_block);
	}
}

static ssize_t cavan_ext4_read_symlink_by_inode(struct cavan_ext4_fs *fs, struct ext2_inode_large *inode, char *buff, size_t size)
{
	size_t length;

	length = sizeof(inode->i_block);
	if (length < inode->i_size)
	{
		return cavan_ext4_read_inode_data(fs, inode, 0, buff, size, NULL, NULL);
	}

	length = size < inode->i_size ? size : inode->i_size;
	mem_copy(buff, inode->i_block, length);

	return length;
}

ssize_t cavan_ext4_read_symlink(struct cavan_ext4_file *file, char *buff, size_t size)
{
	return cavan_ext4_read_symlink_by_inode(file->fs, &file->inode_large, buff, size);
}

static int cavan_ext4_find_file_write_byte(struct cavan_ext4_walker *walker, void *buff, size_t size)
{
	struct ext2_dir_entry_2 *entry, *entry_end;
	struct cavan_ext4_find_file_context *context = walker->context;

	for (entry = buff, entry_end = ADDR_ADD(entry, size - CAVAN_EXT4_DIR_ENTRY_MIN_LEN); entry < entry_end; entry = ADDR_ADD(entry, entry->rec_len))
	{
#if CAVAN_EXT4_DEBUG
		cavan_ext4_dump_ext2_dir_entry_2(entry);
#endif

		if (entry->name_len == 0 || entry->rec_len == 0)
		{
			pr_red_info("invalid directory entry");
			return -EINVAL;
		}

		if (entry->name_len == context->name_len && strncmp(entry->name, context->filename, context->name_len) == 0)
		{
			context->inode = entry->inode;
			return WALKER_ACTION_STOP;
		}
	}

	return 0;
}

static int cavan_ext4_find_file(struct cavan_ext4_fs *fs, struct cavan_ext4_file *file, const char *pathname)
{
	u32 inode_index;
	char symlink[1024];
	struct ext2_inode *inode = &file->inode;
	struct cavan_ext4_find_file_context context;
	struct cavan_ext4_walker walker =
	{
		.write_byte = cavan_ext4_find_file_write_byte
	};

	context.inode = inode_index = EXT2_ROOT_INO;

	while (1)
	{
		int ret;
		ssize_t rdlen;
		const char *p;

		rdlen = cavan_ext4_read_inode(fs, context.inode, &file->inode_large);
		if (rdlen < 0)
		{
			pr_error_info("cavan_ext4_read_inode");
			return rdlen;
		}

#if CAVAN_EXT4_DEBUG
		cavan_ext4_dump_ext2_inode(inode);
#endif

		while (*pathname == CAVAN_EXT4_PATH_SEP)
		{
			pathname++;
		}

#if CAVAN_EXT4_DEBUG
		println("pathname = %s", pathname);
#endif

		if (S_ISLNK(inode->i_mode))
		{
			rdlen = cavan_ext4_read_symlink_by_inode(fs, &file->inode_large, symlink, sizeof(symlink) - 1);
			if (rdlen < 0)
			{
				pr_red_info("cavan_ext4_read_inode");
				return rdlen;
			}

			if (rdlen > 1 && symlink[rdlen - 1] == CAVAN_EXT4_PATH_SEP)
			{
				rdlen--;
			}

			if (*pathname)
			{
				text_path_cat(symlink + rdlen, sizeof(symlink) - rdlen, NULL, pathname);
				println("%s -> %s", pathname, symlink);
			}
			else
			{
				symlink[rdlen] = 0;
				println("symlink = %s", symlink);
			}

			pathname = symlink;
			context.inode = pathname[0] == CAVAN_EXT4_PATH_SEP ? EXT2_ROOT_INO : inode_index;
			continue;
		}

		if (*pathname == 0)
		{
			break;
		}

		if (S_ISDIR(inode->i_mode) == 0)
		{
			ERROR_RETURN(ENOENT);
		}

		for (p = pathname; *p && *p != CAVAN_EXT4_PATH_SEP; p++);

		context.filename = pathname;
		context.name_len = p - pathname;

		inode_index = context.inode;
		context.inode = 0;

		ret = cavan_ext4_traversal_inode(fs, &file->inode_large, 0, &walker, &context);
		if (ret < 0 || context.inode == 0)
		{
			ERROR_RETURN(ENOENT);
		}

		pathname = p;
	}

	return 0;
}

struct cavan_ext4_file *cavan_ext4_open_file(struct cavan_ext4_fs *fs, const char *pathname)
{
	int ret;
	struct cavan_ext4_file *file;

	file = malloc(sizeof(*file));
	if (file == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	file->fs = fs;

	ret = cavan_ext4_find_file(fs, file, pathname);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_find_file");
		goto out_free_file;
	}

	file->pathname = pathname;

	return file;

out_free_file:
	free(file);
	return NULL;
}

static int cavan_ext4_read_inode_data_write_byte(struct cavan_ext4_walker *walker, void *buff, size_t size)
{
	int ret;
	int action;
	size_t remain;
	struct cavan_ext4_read_inode_data_context *context = walker->context;

	remain = context->size - context->length;
	if (size < remain)
	{
		action = WALKER_ACTION_CONTINUE;
	}
	else
	{
		size = remain;
		action = WALKER_ACTION_STOP;
	}

	ret = context->handler(context, buff, context->length, size);
	if (ret < 0)
	{
		pr_red_info("context->handler");
		return ret;
	}

	context->length += size;

	return action;
}

static int cavan_ext4_read_inode_data_to_buff_handler(struct cavan_ext4_read_inode_data_context *context, void *buff, off_t offset, size_t size)
{
	mem_copy(ADDR_ADD(context->buff, offset), buff, size);
	return size;
}

static int cavan_ext4_read_inode_data_to_file_handler(struct cavan_ext4_read_inode_data_context *context, void *buff, off_t offset, size_t size)
{
	return ffile_write(*(int *) context->data, buff, size);
}

ssize_t cavan_ext4_read_inode_data(struct cavan_ext4_fs *fs, struct ext2_inode_large *inode, size_t skip, void *buff, size_t size, int (*handler)(struct cavan_ext4_read_inode_data_context *context, void *buff, off_t offset, size_t size), void *data)
{
	int ret;
	struct cavan_ext4_walker walker;
	struct cavan_ext4_read_inode_data_context context;

	if (buff && size > 0)
	{
		context.handler = cavan_ext4_read_inode_data_to_buff_handler;
	}
	else if (handler)
	{
		if (size == 0)
		{
			size = inode->i_size;
		}

		context.handler = handler;
	}
	else
	{
		if (inode->i_size == 0)
		{
			return 0;
		}

		pr_red_info("buff and handler must set one");

		return -EINVAL;
	}

	walker.write_byte = cavan_ext4_read_inode_data_write_byte;

	context.data = data;
	context.length = 0;
	context.size = size;
	context.buff = buff;

	ret = cavan_ext4_traversal_inode(fs, inode, skip, &walker, &context);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_traversal_inode");
		return ret;
	}

	return context.length;
}

ssize_t cavan_ext4_read_file(struct cavan_ext4_file *file, size_t skip, void *buff, size_t size)
{
	return cavan_ext4_read_inode_data(file->fs, &file->inode_large, skip, buff, size, NULL, NULL);
}

ssize_t cavan_ext4_read_file2(struct cavan_ext4_file *file, size_t skip, int fd)
{
	return cavan_ext4_read_inode_data(file->fs, &file->inode_large, skip, NULL, 0, cavan_ext4_read_inode_data_to_file_handler, &fd);
}

ssize_t cavan_ext4_read_file3(struct cavan_ext4_file *file, size_t skip, const char *pathname, int flags)
{
	int fd;
	ssize_t rdlen;

	fd = open(pathname, O_WRONLY | O_CREAT | flags, 0777);
	if (fd < 0)
	{
		pr_error_info("open file %s", pathname);
		return fd;
	}

	rdlen = cavan_ext4_read_file2(file, skip, fd);

	close(fd);

	return rdlen;
}

ssize_t cavan_ext4_read_file4(struct cavan_ext4_file *file, size_t skip, size_t size, int (*handler)(struct cavan_ext4_read_inode_data_context *context, void *buff, off_t offset, size_t size), void *data)
{
	return cavan_ext4_read_inode_data(file->fs, &file->inode_large, skip, NULL, size, handler, data);
}

static int cavan_ext4_list_dir_write_byte(struct cavan_ext4_walker *walker, void *buff, size_t size)
{
	struct ext2_dir_entry_2 *entry, *entry_end;
	struct cavan_ext4_list_dir_context *context = walker->context;

	for (entry = buff, entry_end = ADDR_ADD(entry, size - CAVAN_EXT4_DIR_ENTRY_MIN_LEN); entry < entry_end; entry = ADDR_ADD(entry, entry->rec_len))
	{
#if CAVAN_EXT4_DEBUG
		cavan_ext4_dump_ext2_dir_entry_2(entry);
#endif

		if (entry->name_len == 0 || entry->rec_len == 0)
		{
			pr_red_info("invalid directory entry");
			return -EINVAL;
		}

		context->handler(entry, context->data);
	}

	return 0;
}

int cavan_ext4_list_dir(struct cavan_ext4_file *file, void (*handler)(struct ext2_dir_entry_2 *entry, void *data), void *data)
{
	struct cavan_ext4_walker walker;
	struct cavan_ext4_list_dir_context context;

	if (handler == NULL)
	{
		pr_red_info("handler is null");
		return -EINVAL;
	}

	if (S_ISDIR(file->inode.i_mode) == 0)
	{
		pr_red_info("this is not a directory");
		return -ENOTDIR;
	}

	walker.write_byte = cavan_ext4_list_dir_write_byte;

	context.data = data;
	context.file = file;
	context.handler = handler;

	return cavan_ext4_traversal_inode(file->fs, &file->inode_large, 0, &walker, &context);
}

void cavan_ext4_close_file(struct cavan_ext4_file *file)
{
	free(file);
}
