// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 15 10:56:28 CST 2011

#include <cavan.h>
#include <cavan/ext2.h>
#include <cavan/text.h>

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

	desc->group_count = (super_block->blocks_count + super_block->blocks_per_group - 1) / super_block->blocks_per_group;

	desc->gdt = malloc(desc->group_count * sizeof(*desc->gdt));
	if (desc->gdt == NULL)
	{
		ret = -ENOMEM;
		goto out_close_device;
	}

	ret = ext2_read_gdt(desc, desc->gdt);
	if (ret < 0)
	{
		goto out_free_gdt;
	}

	// show_ext2_desc(desc);

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

void show_ext2_desc(struct ext2_desc *desc)
{
	unsigned int i;

	print_sep(60);
#if __WORDSIZE == 64
	println("block_shift = %ld", desc->block_shift);
	println("block_size = %ld", desc->block_size);
	println("group_count = %ld", desc->group_count);
#else
	println("block_shift = %d", desc->block_shift);
	println("block_size = %d", desc->block_size);
	println("group_count = %d", desc->group_count);
#endif
	print_sep(60);
	show_ext2_super_block(&desc->super_block);

	for (i = 0; i < desc->group_count; i++)
	{
		print_sep(60);
		println("gdt_index = %d", i);
		show_ext2_group_desc(desc->gdt + i);
	}
	print_sep(60);
}

void show_ext2_super_block(struct ext2_super_block *super_block)
{
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
	println("magic_number = %d", super_block->magic_number);
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
	println("feature_compat = %d", super_block->feature_compat);
	println("feature_incompat = %d", super_block->feature_incompat);
	println("feature_ro_compat = %d", super_block->feature_ro_compat);
	println("uuid[16] = %s", text_header((char *)super_block->uuid, 16));
	println("volume_name[16] = %s", text_header(super_block->volume_name, 16));
	println("last_mounted[64] = %s", text_header(super_block->last_mounted, 64));
	println("algorithm_usage_bitmap = %d", super_block->algorithm_usage_bitmap);
	println("prealloc_blocks = %d", super_block->prealloc_blocks);
	println("prealloc_dir_blocks = %d", super_block->prealloc_dir_blocks);
	println("padding1 = %d", super_block->padding1);
}

void show_ext2_group_desc(struct ext2_group_desc *group_desc)
{
	println("block_bitmap = %d", group_desc->block_bitmap);
	println("inode_bitmap = %d", group_desc->inode_bitmap);
	println("inode_table = %d", group_desc->inode_table);
	println("free_blocks_count = %d", group_desc->free_blocks_count);
	println("free_inodes_count = %d", group_desc->free_inodes_count);
	println("used_dirs_count = %d", group_desc->used_dirs_count);
	println("pad = %d", group_desc->pad);
}

void show_ext2_directory_entry(struct ext2_directory_entry *dir_entry)
{
	println("file_type = %d", dir_entry->file_type);
	println("inode = %d", dir_entry->inode);
	println("name = %s", dir_entry->name);
	println("name_len = %d", dir_entry->name_len);
	println("rec_len = %d", dir_entry->rec_len);
}

void show_ext2_inode(struct ext2_inode *inode)
{
	int i;

	println("mode = %d", inode->mode);
	println("uid = %d", inode->uid);
	println("size = %d", inode->size);
	println("atime = %d", inode->atime);
	println("ctime = %d", inode->ctime);
	println("mtime = %d", inode->mtime);
	println("dtime = %d", inode->dtime);
	println("gid = %d", inode->gid);
	println("links_count = %d", inode->links_count);
	println("blocks = %d", inode->blocks);
	println("flags = %d", inode->flags);
	println("version = %d", inode->version);
	println("file_acl = %d", inode->file_acl);
	println("dir_acl = %d", inode->dir_acl);
	println("faddr = %d", inode->faddr);

	for (i = 0; i < EXT2_N_BLOCKS; i++)
	{
		println("inode->block[%d] = %d", i, inode->block[i]);
	}
}

const char *ext2_filetype_to_text(int type)
{
	const char *ext2_filetypes[] = {"Unknown", "Regular", "Directory", "Char_dev", "Block_dev", "Pipe", "Socket", "Symlink"};

	if (type < 0 || (size_t)type >= ARRAY_SIZE(ext2_filetypes))
	{
		return ext2_filetypes[0];
	}

	return ext2_filetypes[type];
}

int ext2_find_file(struct ext2_desc *desc, const char *pathname, struct ext2_inode *inode)
{
	int i;
	ssize_t readlen;
	struct ext2_directory_entry dir_entry;
	off_t seek_value;
	char path_temp[1024], *p, *q;

	text_copy(path_temp, pathname);
	p = q = path_temp;
	dir_entry.inode = 2;
	dir_entry.file_type = EXT_FILE_TYPE_DIRECTORY;

	while (1)
	{
		int block_count;

		seek_value = lseek(desc->fd, get_inode_offset(desc, dir_entry.inode), SEEK_SET);
		if (seek_value < 0)
		{
			print_error("lseek");
			return seek_value;
		}

		readlen = read(desc->fd, inode, sizeof(*inode));
		if (readlen < 0)
		{
			print_error("ext2_read_inode");
			return readlen;
		}

		while (*p == '/')
		{
			p++;
		}

		if (*p == 0)
		{
			break;
		}

		if (dir_entry.file_type != EXT_FILE_TYPE_DIRECTORY)
		{
			ERROR_RETURN(ENOENT);
		}

		for (q = p; *q && *q != '/'; q++);

		*q = 0;

		block_count = cal_ext2_block_count(desc, inode);

		for (i = 0; i < block_count; i++)
		{
			off_t seek_end;

			seek_value = block_index_to_offset(desc, inode->block[i]);
			seek_end = seek_value + desc->block_size;

			while (seek_value < seek_end)
			{
				seek_value = lseek(desc->fd, seek_value, SEEK_SET);
				if (seek_value < 0)
				{
					print_error("lseek");
					return seek_value;
				}

				readlen = read(desc->fd, &dir_entry, sizeof(dir_entry));
				if (readlen < 0)
				{
					print_error("read");
					return readlen;
				}

				dir_entry.name[dir_entry.name_len] = 0;
				// println("%s[%d]: %s", ext2_filetype_to_text(dir_entry.file_type), dir_entry.inode, dir_entry.name);

				if (text_ncmp(p, dir_entry.name, sizeof(dir_entry.name)) == 0)
				{
					goto label_find_next_path;
				}

				seek_value += dir_entry.rec_len;
			}
		}

		ERROR_RETURN(ENOENT);

label_find_next_path:
		p = q + 1;
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
		ssize_t readlen;
		struct ext2_directory_entry dir_entry;

		seek_value = block_index_to_offset(desc, inode->block[i]);
		seek_end = seek_value + desc->block_size;

		while (seek_value < seek_end)
		{
			seek_value = lseek(desc->fd, seek_value, SEEK_SET);
			if (seek_value < 0)
			{
				print_error("lseek");
				return seek_value;
			}

			readlen = read(desc->fd, &dir_entry, sizeof(dir_entry));
			if (readlen < 0)
			{
				print_error("read");
				return readlen;
			}

			dir_entry.name[dir_entry.name_len] = 0;
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

		buff = (char *)buff + readlen;
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

