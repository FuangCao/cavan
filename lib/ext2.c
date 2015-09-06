// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 15 10:56:28 CST 2011

#include <cavan.h>
#include <cavan/ext2.h>
#include <cavan/text.h>

#define CAVAN_EXT2_DEBUG	1

static ssize_t ext2_read_block(struct ext2_desc *desc, u64 index, void *blocks, size_t count)
{
	off_t offset = block_index_to_offset(desc, index);

	return ffile_readfrom(desc->fd, blocks, desc->block_size * count, offset);
}

static ssize_t ext2_write_block(struct ext2_desc *desc, u64 index, const void *blocks, size_t count)
{
	off_t offset = block_index_to_offset(desc, index);

	return ffile_writeto(desc->fd, blocks, desc->block_size * count, offset);
}

static struct ext2_group_desc *ext2_read_gdt(struct ext2_desc *desc)
{
	ssize_t rdlen;
	struct ext2_group_desc *gdt;
	size_t gdt_size = desc->group_count * sizeof(*gdt);

	gdt = malloc(gdt_size);
	if (gdt == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	rdlen = ffile_readfrom(desc->fd, gdt, gdt_size, BOOT_BLOCK_SIZE + desc->block_size);
	if ((size_t) rdlen != gdt_size)
	{
		pr_error_info("ffile_readfrom");
		goto out_free_gdt;
	}

	return gdt;

out_free_gdt:
	free(gdt);
	return NULL;
}

int ext2_init(struct ext2_desc *desc, const char *dev_path)
{
	int ret;
	struct ext2_super_block *super_block;

	desc->fd = open(dev_path, O_RDONLY);
	if (desc->fd < 0)
	{
		print_error("open device \"%s\" failed", dev_path);
		return desc->fd;
	}

	super_block = &desc->super_block;

	ret = ext2_read_super_block(desc, super_block);
	if (ret < 0)
	{
		pr_red_info("ext2_read_super_block");
		goto out_close_device;
	}

	if (super_block->magic_number != EXT2_SUPER_BLOCK_MAGIC)
	{
		ret = -EINVAL;
		pr_red_info("magic_number = 0x%04x", super_block->magic_number);
		goto out_close_device;
	}

	desc->block_shift = super_block->log_block_size + 10;
	desc->block_size = 1 << desc->block_shift;

	desc->first_data_block = super_block->first_data_block;
	desc->blocks_count = super_block->blocks_count;
	desc->blocks_per_group = super_block->blocks_per_group;
	desc->inodes_count = super_block->inodes_count;
	desc->inodes_per_group = super_block->inodes_per_group;
	desc->inode_size = super_block->inode_size;
	desc->blocks_per_inode_table = (super_block->inodes_count * super_block->inode_size + desc->block_size - 1) >> desc->block_shift;

	desc->group_flex_shift = super_block->log_groups_per_flex;
	desc->groups_per_flex = 1 << desc->group_flex_shift;
	desc->group_count = (super_block->blocks_count + super_block->blocks_per_group - 1) / super_block->blocks_per_group;
	desc->flex_count = (desc->group_count + desc->group_flex_shift - 1) / desc->group_flex_shift;

	desc->gdt = ext2_read_gdt(desc);
	if (desc->gdt == NULL)
	{
		ret = -EFAULT;
		pr_red_info("ext2_read_gdt");
		goto out_free_gdt;
	}

#if CAVAN_EXT2_DEBUG
	show_ext2_desc(desc);
#endif

	desc->read_block = ext2_read_block;
	desc->write_block = ext2_write_block;

	return 0;

out_free_gdt:
	free(desc->gdt);
out_close_device:
	close(desc->fd);

	return ret;
}

void ext2_deinit(struct ext2_desc *desc)
{
	free(desc->gdt);
	close(desc->fd);
}

void show_ext2_desc(const struct ext2_desc *desc)
{
	unsigned int i;

	print_sep(60);
	pr_bold_info("ext2 desc %p", desc);

	println("block_shift = %" PRINT_FORMAT_SIZE, desc->block_shift);
	println("block_size = %" PRINT_FORMAT_SIZE, desc->block_size);
	println("flex_count = %" PRINT_FORMAT_SIZE, desc->flex_count);
	println("group_count = %" PRINT_FORMAT_SIZE, desc->group_count);
	println("groups_per_flex = %" PRINT_FORMAT_SIZE, desc->groups_per_flex);
	println("group_flex_shift = %" PRINT_FORMAT_SIZE, desc->group_flex_shift);

	show_ext2_super_block(&desc->super_block);

	for (i = 0; i < desc->group_count; i++)
	{
		show_ext2_group_desc(desc->gdt + i);
		println("gdt_index = %d", i);
	}
}

void show_ext2_super_block(const struct ext2_super_block *super_block)
{
	print_sep(60);
	pr_bold_info("ext2 super block %p", super_block);

	println("inodes_count = %d", super_block->inodes_count);
	println("blocks_count = %d", super_block->blocks_count);
	println("reserved_blocks_count = %d", super_block->reserved_blocks_count);
	println("free_blocks_count = %d", super_block->free_blocks_count);
	println("free_inodes_count = %d", super_block->free_inodes_count);
	println("first_data_block = %d", super_block->first_data_block);
	println("log_block_size = %d", super_block->log_block_size);
	println("log_frag_size = %d", super_block->log_frag_size);
	println("blocks_per_group = %d", super_block->blocks_per_group);
	println("frags_per_group = %d", super_block->frags_per_group);
	println("inodes_per_group = %d", super_block->inodes_per_group);
	println("mtime = %d", super_block->mtime);
	println("wtime = %d", super_block->wtime);
	println("mnt_count = %d", super_block->mnt_count);
	println("max_mnt_count = %d", super_block->max_mnt_count);
	println("magic_number = 0x%04x", super_block->magic_number);
	println("state = %d", super_block->state);
	println("errors = %d", super_block->errors);
	println("minor_rev_level = %d", super_block->minor_rev_level);
	println("lastcheck = %d", super_block->lastcheck);
	println("checkinterval = %d", super_block->checkinterval);
	println("creator_os = %d", super_block->creator_os);
	println("rev_level = %d", super_block->rev_level);
	println("def_resuid = %d", super_block->def_resuid);
	println("def_resgid = %d", super_block->def_resgid);
	println("first_inode = %d", super_block->first_inode);
	println("inode_size = %d", super_block->inode_size);
	println("block_group_nr = %d", super_block->block_group_nr);
	println("feature_compat = 0x%08x", super_block->feature_compat);
	println("feature_incompat = 0x%08x", super_block->feature_incompat);
	println("feature_ro_compat = 0x%08x", super_block->feature_ro_compat);
	println("uuid[16] = %s", text_header((char *) super_block->uuid, 16));
	println("volume_name[16] = %s", text_header(super_block->volume_name, 16));
	println("last_mounted[64] = %s", text_header(super_block->last_mounted, 64));
	println("algorithm_usage_bitmap = %d", super_block->algorithm_usage_bitmap);
	println("prealloc_blocks = %d", super_block->prealloc_blocks);
	println("prealloc_dir_blocks = %d", super_block->prealloc_dir_blocks);

	println("reserved_gdt_blocks = %d", super_block->reserved_gdt_blocks);
	// println("journal_uuid[16] = %d", super_block->journal_uuid[16]);
	println("journal_inum = %d", super_block->journal_inum);
	println("journal_dev = %d", super_block->journal_dev);
	println("last_orphan = %d", super_block->last_orphan);
	// println("hash_seed[4] = %d", super_block->hash_seed[4]);
	println("def_hash_version = %d", super_block->def_hash_version);
	println("jnl_backup_type = %d", super_block->jnl_backup_type);
	println("desc_size = %d", super_block->desc_size);
	println("default_mount_opts = %d", super_block->default_mount_opts);
	println("first_meta_bg = %d", super_block->first_meta_bg);
	println("mkfs_time = %d", super_block->mkfs_time);
	// println("jnl_blocks[17] = %d", super_block->jnl_blocks[17]);
	println("blocks_count_hi = %d", super_block->blocks_count_hi);
	println("r_blocks_count_hi = %d", super_block->r_blocks_count_hi);
	println("free_blocks_hi = %d", super_block->free_blocks_hi);
	println("min_extra_isize = %d", super_block->min_extra_isize);
	println("want_extra_isize = %d", super_block->want_extra_isize);
	println("flags = %d", super_block->flags);
	println("raid_stride = %d", super_block->raid_stride);
	println("mmp_update_interval = %d", super_block->mmp_update_interval);
	println("mmp_block = %" PRINT_FORMAT_INT64, super_block->mmp_block);
	println("raid_stripe_width = %d", super_block->raid_stripe_width);
	println("log_groups_per_flex = %d", super_block->log_groups_per_flex);
	println("reserved_char_pad = %d", super_block->reserved_char_pad);
	println("reserved_pad = %d", super_block->reserved_pad);
	println("kbytes_written = %" PRINT_FORMAT_INT64, super_block->kbytes_written);
	println("snapshot_inum = %d", super_block->snapshot_inum);
	println("snapshot_id = %d", super_block->snapshot_id);
	println("snapshot_r_blocks_count = %" PRINT_FORMAT_INT64, super_block->snapshot_r_blocks_count);
	println("snapshot_list = %d", super_block->snapshot_list);
	println("error_count = %d", super_block->error_count);
	println("first_error_time = %d", super_block->first_error_time);
	println("first_error_ino = %d", super_block->first_error_ino);
	println("first_error_block = %" PRINT_FORMAT_INT64, super_block->first_error_block);
	// println("first_error_func[32] = %d", super_block->first_error_func[32]);
	println("first_error_line = %d", super_block->first_error_line);
	println("last_error_time = %d", super_block->last_error_time);
	println("last_error_ino = %d", super_block->last_error_ino);
	println("last_error_line = %d", super_block->last_error_line);
	println("last_error_block = %" PRINT_FORMAT_INT64, super_block->last_error_block);
	// println("last_error_func[32] = %d", super_block->last_error_func[32]);
	// println("mount_opts[64] = %d", super_block->mount_opts[64]);
	println("usr_quota_inum = %d", super_block->usr_quota_inum);
	println("grp_quota_inum = %d", super_block->grp_quota_inum);
	println("overhead_blocks = %d", super_block->overhead_blocks);
	// println("backup_bgs[2] = %d", super_block->backup_bgs[2]);
	// println("reserved[106] = %d", super_block->reserved[106]);
	println("checksum = %d", super_block->checksum);
}

void show_ext2_group_desc(const struct ext2_group_desc *group_desc)
{
	print_sep(60);
	pr_bold_info("ext2 group desc %p", group_desc);

	println("block_bitmap = %d", group_desc->block_bitmap);
	println("inode_bitmap = %d", group_desc->inode_bitmap);
	println("inode_table = %d", group_desc->inode_table);
	println("free_blocks_count = %d", group_desc->free_blocks_count);
	println("free_inodes_count = %d", group_desc->free_inodes_count);
	println("used_dirs_count = %d", group_desc->used_dirs_count);
	println("pad = %d", group_desc->pad);
}

void show_ext2_directory_entry(const struct ext2_directory_entry *dir_entry)
{
	print_sep(60);
	pr_bold_info("ext2 directory entry %p", dir_entry);

	println("file_type = %d", dir_entry->file_type);
	println("inode = %d", dir_entry->inode);
	println("name = %s", dir_entry->name);
	println("name_len = %d", dir_entry->name_len);
	println("rec_len = %d", dir_entry->rec_len);
}

void show_ext2_inode(const struct ext2_inode *inode)
{
	int i;

	print_sep(60);
	pr_bold_info("ext2 inode %p", inode);

	println("mode = 0x%05o", inode->mode);
	println("uid = %d", inode->uid);
	println("size = %d", inode->size);
	println("atime = %d", inode->atime);
	println("ctime = %d", inode->ctime);
	println("mtime = %d", inode->mtime);
	println("dtime = %d", inode->dtime);
	println("gid = %d", inode->gid);
	println("links_count = %d", inode->links_count);
	println("blocks = %d", inode->blocks);
	println("flags = 0x%08x", inode->flags);
	println("version = %d", inode->version);
	println("file_acl = %d", inode->file_acl);
	println("dir_acl = %d", inode->dir_acl);
	println("faddr = %d", inode->faddr);

	if ((inode->flags & EXT2_INODE_FLAG_EXTENTS) == 0)
	{
		for (i = 0; i < EXT2_N_BLOCKS; i++)
		{
			println("inode->block[%d] = %d", i, inode->block[i]);
		}
	}
}

void show_ext4_extent_header(const struct ext4_extent_header *header)
{
	print_sep(60);
	pr_bold_info("ext4 extent header %p", header);

	println("magic = 0x%04x", header->magic);
	println("entries = %d", header->entries);
	println("max_entries = %d", header->max_entries);
	println("depth = %d", header->depth);
	println("generations = %d", header->generations);
}

void show_ext4_extent_index(const struct ext4_extent_index *index)
{
	print_sep(60);
	pr_bold_info("ext4 extent index %p", index);

	println("block = %d", index->block);
	println("leaf = %" PRINT_FORMAT_INT64, (u64) index->leaf_hi << 32 | index->leaf_lo);
}

void show_ext4_extent_leaf(const struct ext4_extent_leaf *leaf)
{
	print_sep(60);
	pr_bold_info("ext4 extent leaf %p", leaf);

	println("block = %d", leaf->block);
	println("length = %d", leaf->length);
	println("start = %" PRINT_FORMAT_INT64, (u64) leaf->start_hi << 32 | leaf->start_lo);
}

static int ext2_read_directory_entry(struct ext2_desc *desc, off_t offset, struct ext2_directory_entry *entry)
{
	ssize_t rdlen;

	rdlen = ffile_readfrom(desc->fd, entry, EXT2_DIR_ENTRY_HEADER_SIZE, offset);
	if (rdlen < 0)
	{
		pr_error_info("read");
		return rdlen;
	}

	if (entry->inode == 0)
	{
		pr_red_info("inode is zero");
		return -EINVAL;
	}

	if (entry->name_len == 0)
	{
		pr_red_info("name length is zero");
		return -EINVAL;
	}

	if (entry->rec_len < EXT2_DIR_ENTRY_HEADER_SIZE + entry->name_len)
	{
		pr_red_info("rec_len = %d", entry->rec_len);
		return -EINVAL;
	}

	rdlen = ffile_read(desc->fd, entry->name, entry->name_len);
	if (rdlen < 0)
	{
		pr_error_info("read");
		return rdlen;
	}

	entry->name[rdlen] = 0;

#if CAVAN_EXT2_DEBUG
	show_ext2_directory_entry(entry);
#endif

	return 0;
}

const char *ext2_filetype_to_text(int type)
{
	const char *ext2_filetypes[] = { "Unknown", "Regular", "Directory", "Char_dev", "Block_dev", "Pipe", "Socket", "Symlink" };

	if (type < 0 || (size_t) type >= ARRAY_SIZE(ext2_filetypes))
	{
		return ext2_filetypes[0];
	}

	return ext2_filetypes[type];
}

static int ext4_find_file_base(struct ext2_desc *desc, const char *filename, struct ext4_extent_header *header, struct ext2_directory_entry *entry)
{
	ssize_t rdlen;

	show_ext4_extent_header(header);

	if (header->depth > 0)
	{
		struct ext4_extent_index *index_end;
		struct ext4_extent_index *index = (struct ext4_extent_index *) (header + 1);

		for (index_end = index + header->entries; index < index_end; index++)
		{
			char buff[desc->block_size];

			show_ext4_extent_index(index);

			rdlen = desc->read_block(desc, (u64) index->leaf_hi << 32 | index->leaf_lo, buff, 1);
			if (rdlen < 0)
			{
				pr_error_info("desc->read_block");
				return rdlen;
			}

			if (ext4_find_file_base(desc, filename, (struct ext4_extent_header *) buff, entry) == 0)
			{
				return 0;
			}
		}
	}
	else
	{
		struct ext2_directory_entry *p, *p_end;
		struct ext4_extent_leaf *leaf_end;
		struct ext4_extent_leaf *leaf = (struct ext4_extent_leaf *) (header + 1);

		p = alloca(desc->block_size);
		p_end = ADDR_ADD(p, desc->block_size);

		for (leaf_end = leaf + header->entries; leaf < leaf_end; leaf++)
		{
			show_ext4_extent_leaf(leaf);

			rdlen = desc->read_block(desc, (u64) leaf->start_hi << 32 | leaf->start_lo, p, 1);
			if (rdlen < 0)
			{
				pr_error_info("desc->read_block");
				return rdlen;
			}

			while (p < p_end)
			{
#if CAVAN_EXT2_DEBUG
				p->name[p->name_len] = 0;
				show_ext2_directory_entry(p);
#endif
				if (text_ncmp(filename, p->name, p->name_len) == 0)
				{
					mem_copy(entry, p, EXT2_DIR_ENTRY_HEADER_SIZE + p->name_len);
					entry->name[entry->name_len] = 0;
					return 0;
				}

				p = ADDR_ADD(p, p->rec_len);
			}
		}
	}

	return -ENOENT;
}

static int ext2_find_file_base(struct ext2_desc *desc, const char *filename, const u32 *blocks, size_t count, struct ext2_directory_entry *entry)
{
	u32 index;

	for (index = 0; index < count; index++)
	{
		off_t seek_value, seek_end;

		seek_value = block_index_to_offset(desc, blocks[index]);
		seek_end = seek_value + desc->block_size;

		while (seek_value < seek_end)
		{
			int ret;

			ret = ext2_read_directory_entry(desc, seek_value, entry);
			if (ret < 0)
			{
				pr_red_info("ext2_read_directory_entry");
				return ret;
			}

#if CAVAN_EXT2_DEBUG
			println("%s[%d]: %s", ext2_filetype_to_text(entry->file_type), entry->inode, entry->name);
#endif

			if (text_ncmp(filename, entry->name, sizeof(entry->name)) == 0)
			{
				return 0;
			}

			seek_value += entry->rec_len;
		}
	}

	return -ENOENT;
}

int ext2_find_file(struct ext2_desc *desc, const char *pathname, struct ext2_inode *inode)
{
	struct ext2_directory_entry entry;
	char path_temp[1024], *filename, *p;

	text_copy(path_temp, pathname);
	filename = p = path_temp;

	entry.inode = 2;
	entry.file_type = EXT_FILE_TYPE_DIRECTORY;

	while (1)
	{
		int ret;
		ssize_t rdlen;

		rdlen = ext2_read_inode(desc, entry.inode, inode);
		if (rdlen < 0)
		{
			print_error("ext2_read_inode");
			return rdlen;
		}

#if CAVAN_EXT2_DEBUG
		show_ext2_inode(inode);
#endif

		while (*filename == '/')
		{
			filename++;
		}

		if (*filename == 0)
		{
			break;
		}

		if (entry.file_type != EXT_FILE_TYPE_DIRECTORY)
		{
			ERROR_RETURN(ENOENT);
		}

		for (p = filename; *p && *p != '/'; p++);

		*p = 0;

		if (inode->flags & EXT2_INODE_FLAG_EXTENTS)
		{
			ret = ext4_find_file_base(desc, filename, (struct ext4_extent_header *) inode->block, &entry);
		}
		else
		{
			ret = ext2_find_file_base(desc, filename, inode->block, inode->blocks, &entry);
		}

		if (ret < 0)
		{
			ERROR_RETURN(ENOENT);
		}

		filename = p + 1;
	}

	return 0;
}

int ext2_list_directory_base(struct ext2_desc *desc, struct ext2_inode *inode)
{
	int i;
	int block_count;
	off_t seek_value;

	block_count = cal_ext2_block_count(desc, inode);

	for (i = 0; i < block_count; i++)
	{
		off_t seek_end;
		struct ext2_directory_entry dir_entry;

		seek_value = block_index_to_offset(desc, inode->block[i]);
		seek_end = seek_value + desc->block_size;

		while (seek_value < seek_end)
		{
			int ret;

			ret = ext2_read_directory_entry(desc, seek_value, &dir_entry);
			if (ret < 0)
			{
				pr_red_info("ext2_read_directory_entry");
				return ret;
			}

			println("%s[%d]: %s", ext2_filetype_to_text(dir_entry.file_type), dir_entry.inode, dir_entry.name);

			seek_value += dir_entry.rec_len;
		}
	}

	return 0;
}

int ext2_list_directory(struct ext2_desc *desc, const char *pathname)
{
	int ret;
	struct ext2_inode inode;

	ret = ext2_find_file(desc, pathname, &inode);
	if (ret < 0)
	{
		error_msg("ext2_find_file");
		return ret;
	}

	if (!S_ISDIR(inode.mode))
	{
		error_msg("%s is not a directory", pathname);
		ERROR_RETURN(ENOTDIR);
	}

	return ext2_list_directory_base(desc, &inode);
}

ssize_t ext2_read_file_base(struct ext2_desc *desc, struct ext2_inode *inode, void *buff, size_t size)
{
	int i;
	int block_count;
	size_t total_len;

	println("file size = %s", size2text(inode->size));

	if (inode->size == 0)
	{
		return 0;
	}

	total_len = size > inode->size ? inode->size : size;
	block_count = cal_ext2_block_count(desc, inode);

	for (i = 0; i < block_count && size; i++)
	{
		off_t seek_value;
		ssize_t readlen;

		seek_value = block_index_to_offset(desc, inode->block[i]);
		seek_value = lseek(desc->fd, seek_value, SEEK_SET);
		if (seek_value < 0)
		{
			print_error("lseek");
			return seek_value;
		}

		readlen = size > desc->block_size ? desc->block_size : size;
		readlen = read(desc->fd, buff, readlen);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		buff = (char *) buff + readlen;
	}

	return total_len;
}

ssize_t ext2_read_file(struct ext2_desc *desc, const char *pathname, void *buff, size_t size)
{
	int ret;
	struct ext2_inode inode;

	ret = ext2_find_file(desc, pathname, &inode);
	if (ret < 0)
	{
		error_msg("ext2_find_file");
		return ret;
	}

	if (!S_ISREG(inode.mode))
	{
		error_msg("%s is not a file", pathname);
		ERROR_RETURN(EISDIR);
	}

	return ext2_read_file_base(desc, &inode, buff, size);
}

// ================================================================================

static int cavan_ext4_data_block_traversal(struct ext2_desc *desc, struct ext4_extent_header *header, struct cavan_ext2_traversal_option *option)
{
	int ret = CAVAN_EXT2_TRAVERSAL_EOF;
	ssize_t rdlen;

	show_ext4_extent_header(header);

	if (header->depth > 0)
	{
		char buff[desc->block_size];
		struct ext4_extent_index *index_end;
		struct ext4_extent_index *index = (struct ext4_extent_index *) (header + 1);

		for (index_end = index + header->entries; index < index_end; index++)
		{
			show_ext4_extent_index(index);

			rdlen = desc->read_block(desc, (u64) index->leaf_hi << 32 | index->leaf_lo, buff, 1);
			if (rdlen < 0)
			{
				pr_error_info("desc->read_block");
				return rdlen;
			}

			ret = cavan_ext4_data_block_traversal(desc, (struct ext4_extent_header *) buff, option);
			if (ret < 0)
			{
				pr_red_info("cavan_ext4_data_block_traversal");
				return ret;
			}

			if (ret != CAVAN_EXT2_TRAVERSAL_CONTINUE)
			{
				break;
			}
		}
	}
	else
	{
		struct ext4_extent_leaf *leaf_end;
		struct ext4_extent_leaf *leaf = (struct ext4_extent_leaf *) (header + 1);

		for (leaf_end = leaf + header->entries; leaf < leaf_end; leaf++)
		{
			char buff[desc->block_size * leaf->length];

			show_ext4_extent_leaf(leaf);

			rdlen = desc->read_block(desc, (u64) leaf->start_hi << 32 | leaf->start_lo, buff, leaf->length);
			if (rdlen < 0)
			{
				pr_error_info("desc->read_block");
				return rdlen;
			}

			ret = option->func(desc, buff, leaf->length, option);
			if (ret < 0)
			{
				pr_red_info("option->func");
				return ret;
			}

			if (ret != CAVAN_EXT2_TRAVERSAL_CONTINUE)
			{
				break;
			}
		}
	}

	return ret;
}

static int cavan_ext4_find_file_handler(struct ext2_desc *desc, void *block, size_t count, struct cavan_ext2_traversal_option *_option)
{
	struct ext2_directory_entry *entry, *entry_end;
	struct cavan_ext4_find_file_option *option = (struct cavan_ext4_find_file_option *) _option;

	entry = block;
	entry_end = ADDR_ADD(entry, desc->block_size * count);

	while (entry < entry_end)
	{
		entry->name[entry->name_len] = 0;

#if CAVAN_EXT2_DEBUG
		show_ext2_directory_entry(entry);
#endif

		if (text_cmp(option->filename, entry->name) == 0)
		{
			mem_copy(option->entry, entry, EXT2_DIR_ENTRY_HEADER_SIZE);
			text_ncopy(option->entry->name, entry->name, entry->name_len);
			return CAVAN_EXT2_TRAVERSAL_FOUND;
		}

		entry = ADDR_ADD(entry, entry->rec_len);
	}

	return CAVAN_EXT2_TRAVERSAL_CONTINUE;
}

static int cavan_ext4_find_file_base(struct ext2_desc *desc, const char *filename, struct ext4_extent_header *header, struct ext2_directory_entry *entry)
{
	int ret;
	struct cavan_ext4_find_file_option option =
	{
		.option =
		{
			.func = cavan_ext4_find_file_handler
		},
		.filename = filename,
		.entry = entry
	};

	ret = cavan_ext4_data_block_traversal(desc, header, &option.option);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_data_block_traversal");
		return ret;
	}

	if (ret != CAVAN_EXT2_TRAVERSAL_FOUND)
	{
		ERROR_RETURN(ENOENT);
	}

	return 0;
}

static int cavan_ext2_find_file_base(void)
{
	return -ENOENT;
}

static int cavan_ext2_find_file(struct ext2_desc *desc, struct cavan_ext2_file *file)
{
	struct ext2_directory_entry entry;
	char *filename, *p;

	filename = p = file->pathname;

	entry.inode = 2;
	entry.file_type = EXT_FILE_TYPE_DIRECTORY;

	while (1)
	{
		char c;
		int ret;
		ssize_t rdlen;

		rdlen = ext2_read_inode(desc, entry.inode, &file->inode);
		if (rdlen < 0)
		{
			print_error("ext2_read_inode");
			return rdlen;
		}

#if CAVAN_EXT2_DEBUG
		show_ext2_inode(&file->inode);
#endif

		while (*filename == '/')
		{
			filename++;
		}

		if (*filename == 0)
		{
			break;
		}

		if (entry.file_type != EXT_FILE_TYPE_DIRECTORY)
		{
			ERROR_RETURN(ENOENT);
		}

		for (p = filename; *p && *p != '/'; p++);

		c = *p;
		*p = 0;

		if (file->inode.flags & EXT2_INODE_FLAG_EXTENTS)
		{
			ret = cavan_ext4_find_file_base(desc, filename, (struct ext4_extent_header *) file->inode.block, &entry);
		}
		else
		{
			ret = cavan_ext2_find_file_base();
		}

		*p = c;

		if (ret < 0)
		{
			ERROR_RETURN(ENOENT);
		}

		filename = p + 1;
	}

	return 0;
}

struct cavan_ext2_file *cavan_ext2_open_file(struct ext2_desc *desc, const char *pathname, int flags, mode_t mode)
{
	int ret;
	struct cavan_ext2_file *fp;

	fp = malloc(sizeof(*fp));
	if (fp == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	text_copy(fp->pathname, pathname);

	ret = cavan_ext2_find_file(desc, fp);
	if (ret < 0)
	{
		pr_red_info("cavan_ext2_find_file");
		goto out_free_fp;
	}

	fp->desc = desc;

	return fp;

out_free_fp:
	free(fp);
	return NULL;
}

void cavan_ext2_close_file(struct cavan_ext2_file *file)
{
	free(file);
}

static int cavan_ext4_read_file_handler(struct ext2_desc *desc, void *block, size_t count, struct cavan_ext2_traversal_option *_option)
{
	int ret;
	struct cavan_ext4_read_file_option *option = (struct cavan_ext4_read_file_option *) _option;
	size_t size = desc->block_size * count;
	size_t remain = ADDR_SUB2(option->buff_end, option->buff);

	if (remain < size)
	{
		ret = CAVAN_EXT2_TRAVERSAL_COMPLETE;
		size = remain;
	}
	else
	{
		ret = CAVAN_EXT2_TRAVERSAL_CONTINUE;
	}

	mem_copy(option->buff, block, size);
	option->buff = ADDR_ADD(option->buff, size);

	return ret;
}

static ssize_t cavan_ext4_read_file_base(struct cavan_ext2_file *file, void *buff, size_t size)
{
	int ret;
	struct cavan_ext4_read_file_option option =
	{
		.option =
		{
			.func = cavan_ext4_read_file_handler
		},
		.file = file,
		.buff = buff,
		.buff_end = ADDR_ADD(buff, size > file->inode.size ? file->inode.size : size)
	};

	ret = cavan_ext4_data_block_traversal(file->desc, (struct ext4_extent_header *) file->inode.block, &option.option);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_data_block_traversal");
		return ret;
	}

	return ADDR_SUB2(option.buff, buff);
}

static ssize_t cavan_ext2_read_file_base(void)
{
	return 0;
}

ssize_t cavan_ext2_read_file(struct cavan_ext2_file *file, void *buff, size_t size)
{
	ssize_t rdlen;

	if (!S_ISREG(file->inode.mode))
	{
		error_msg("%s is not a file", file->pathname);
		ERROR_RETURN(EISDIR);
	}

	if (file->inode.flags & EXT2_INODE_FLAG_EXTENTS)
	{
		rdlen = cavan_ext4_read_file_base(file, buff, size);
	}
	else
	{
		rdlen = cavan_ext2_read_file_base();
	}

	return rdlen;
}
