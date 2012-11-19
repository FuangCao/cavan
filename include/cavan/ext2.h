#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 15 10:56:28 CST 2011

#define BOOT_BLOCK_SIZE			KB(1)
#define EXT2_N_BLOCKS			15
#define EXT2_NAME_LEN			255
#define EXT2_SUPER_BLOCK_OFFSET	BOOT_BLOCK_SIZE

#define EXT2_CAL_BLOCK_SIZE(log_block_size) \
	(1 << ((log_block_size) + 10))

#define EXT2_GDT_OFFSET(log_block_size) \
	(BOOT_BLOCK_SIZE + EXT2_CAL_BLOCK_SIZE(log_block_size))

enum ext2_file_type
{
	EXT_FILE_TYPE_UNKNOWN,
	EXT_FILE_TYPE_REDULAR,
	EXT_FILE_TYPE_DIRECTORY,
	EXT_FILE_TYPE_CHAR_DEV,
	EXT_FILE_TYPE_BLOCK_DEV,
	EXT_FILE_TYPE_PIPE,
	EXT_FILE_TYPE_SOCKET,
	EXT_FILE_TYPE_SYMLINK,
};

struct ext2_super_block
{
	u32 inodes_count; // 索引节点的总数
	u32 blocks_count; // 以块为单位的文件系统的大小
	u32 reserved_blocks_count; // 保留的块数
	u32 free_blocks_count; // 空闲块计数器
	u32 free_inodes_count; // 空闲索引节点计数器
	u32 first_data_block; // 第一个使用的块号(总为 1)
	u32 log_block_size; // 块的大小
	s32 log_frag_size; // 片的大小
	u32 blocks_per_group; // 每组中的块数
	u32 frags_per_group; // 每组中的片数
	u32 inodes_per_group; // 每组中的节点数
	u32 mtime; // 最后一次安装操作的时间
	u32 wtime; // 最后一次写操作的时间
	u16 mnt_count; // 安装操作计数器
	u16 max_mnt_count; // 检查之前安装操作的次数
	u16 magic_number; // 魔数签名
	u16 state; // 状态标志
	u16 errors; // 当检测到错误时的行为
	u16 minor_rev_level; // 次版本号
	u32 lastcheck; // 最后一次检查的时间
	u32 checkinterval; // 两次检查之间的时间间隔
	u32 creator_os; // 创建文件系统的操作系统
	u32 rev_level; // 版本号
	u16 def_resuid; // 保留块的缺省 UID
	u16 def_resgid; // 保留块的缺省 GID
	u32 first_inode; // 第一个非保留的索引节点号
	u16 inode_size; // 磁盘上索引节点结构的大小
	u16 block_group_nr; // 这个超级块的块组号
	u32 feature_compat; // 具有兼容特点的位图
	u32 feature_incompat; // 具有非兼容特点的位图
	u32 feature_ro_compat; // 只读兼容特点的位图
	u8 uuid[16]; // 128 位文件系统标识符
	char volume_name[16]; // 卷名
	char last_mounted[64]; // 最后一个安装点的路径名
	u32 algorithm_usage_bitmap; // 用于压缩
	u8 prealloc_blocks; // 预分配的块数
	u8 prealloc_dir_blocks; // 为目录预分配的块数
	u16 padding1; // 按字对齐
	u32 reserved[1024]; // 用 null 填充 1024 字节
};

struct ext2_group_desc
{
	u32 block_bitmap; // 块位图的块号
	u32 inode_bitmap; // 索引节点位图的块号
	u32 inode_table; // 第一个索引节点表块的块号
	u16 free_blocks_count; // 组中空闲块的个数
	u16 free_inodes_count; // 组中索引节点的个数
	u16 used_dirs_count; // 组中目录的个数
	u16 pad; // 按字对齐
	u32 reserved[3]; // 用 null 填充 24 个字节
};

struct ext2_inode
{
	u16 mode; // 文件类型和访问权限
	u16 uid; // 拥有者标识符
	u32 size; // 以字节为单位的文件长度
	u32 atime; // 最后一次访问文件的时间
	u32 ctime; // 索引节点最后改变的时间
	u32 mtime; // 文件内容最后改变的时间
	u32 dtime; // 文件删除的时间
	u16 gid; // 组标识符
	u16 links_count; // 硬链接计数器
	u32 blocks; // 文件的数据块数
	u32 flags; // 文件标志
	u32 osd1; // 特定的操作系统信息
	u32 block[EXT2_N_BLOCKS]; // 指向数据块的指针
	u32 version; // 文件版本(当网络文件系统访问文件时使用)
	u32 file_acl; // 文件访问控制表
	u32 dir_acl; // 目录访问控制表
	u32 faddr; // 片的地址
	u32 osd2[3]; // 特定的操作系统信息
};

struct ext2_directory_entry
{
	u32 inode; // 索引节点号
	u16 rec_len; // 目录项长度
	u8 name_len; // 文件名长度
	u8 file_type; // 文件类型
	char name[EXT2_NAME_LEN]; // name 文件名
};

struct ext2_desc
{
	int fd;

	size_t group_count;

	size_t block_size;
	size_t block_shift;

	u32 first_data_block;
	u32 blocks_per_group;
	u32 inodes_per_group;
	u32 inodes_count;
	u32 blocks_per_inode_table;
	u32 blocks_count;
	size_t inode_size;

	struct ext2_super_block super_block;
	struct ext2_group_desc *gdt;
};

int ext2_init(struct ext2_desc *desc, const char *dev_path);
void ext2_uninit(struct ext2_desc *desc);
void show_ext2_super_block(struct ext2_super_block *super_block);
void show_ext2_group_desc(struct ext2_group_desc *gdt);
void show_ext2_desc(struct ext2_desc *desc);
int ext2_find_file(struct ext2_desc *desc, const char *pathname, struct ext2_inode *inode);
int ext2_list_directory_base(struct ext2_desc *desc, struct ext2_inode *inode);
int ext2_list_directory(struct ext2_desc *desc, const char *pathname);
ssize_t ext2_read_file_base(struct ext2_desc *desc, struct ext2_inode *inode, void *buff, size_t size);
ssize_t ext2_read_file(struct ext2_desc *desc, const char *pathname, void *buff, size_t size);

static inline int ext2_read_super_block(struct ext2_desc *desc, struct ext2_super_block *super_block)
{
	return ffile_readfrom(desc->fd, super_block, sizeof(*super_block), BOOT_BLOCK_SIZE) == sizeof(*super_block) ? 0 : -EFAULT;
}

static inline int ext2_read_gdt(struct ext2_desc *desc, struct ext2_group_desc *gdt)
{
	size_t gdt_size = desc->group_count * sizeof(*gdt);

	return (size_t)ffile_readfrom(desc->fd, gdt, gdt_size, BOOT_BLOCK_SIZE + desc->block_size) == gdt_size ? 0 : -EFAULT;
}

static inline off_t block_index_to_offset(struct ext2_desc *desc, u32 block_index)
{
	return BOOT_BLOCK_SIZE + ((block_index - desc->first_data_block) << desc->block_shift);
}

static inline u32 get_block_group_index(struct ext2_desc *desc, u32 block_index)
{
	return (block_index - desc->first_data_block) / desc->blocks_per_group;
}

static inline u32 get_group_first_block_index(struct ext2_desc *desc, u32 group_index)
{
	return group_index * desc->blocks_per_group + desc->first_data_block;
}

static inline u32 get_group_first_data_block_index(struct ext2_desc *desc, u32 group_index)
{
	return desc->gdt[group_index].inode_table + desc->blocks_per_inode_table;
}

static inline u32 inode_index_to_group(struct ext2_desc *desc, u32 inode_index)
{
	return inode_index / desc->inodes_per_group;
}

static inline u32 inode_index_to_start_block_index(struct ext2_desc *desc, u32 inode_index)
{
	u32 group_index = inode_index_to_group(desc, inode_index);

	return desc->gdt[group_index].inode_table;
}

static inline u32 get_inode_offset(struct ext2_desc *desc, u32 inode_index)
{
	u32 start_block = inode_index_to_start_block_index(desc, inode_index);

	return block_index_to_offset(desc, start_block) + ((inode_index - 1) % desc->inodes_per_group) * desc->inode_size;
}

static inline ssize_t ext2_read_inode(struct ext2_desc *desc, u32 inode_index, struct ext2_inode *inode)
{
	off_t offset = get_inode_offset(desc, inode_index);

	return ffile_readfrom(desc->fd, inode, sizeof(*inode), offset);
}

static inline u32 cal_ext2_block_count(struct ext2_desc *desc, struct ext2_inode *inode)
{
	return inode->blocks / (desc->block_shift - 8);
}

