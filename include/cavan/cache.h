#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Mon Jun 27 09:56:25 CST 2011

#include <cavan.h>

#define DEFAULT_BLOCK_SIZE		512
#define DEFAULT_CACHE_BLOCKS	128

struct mem_cache
{
	char *buff, *end;
	size_t size, free_size;
	char *head;
	const char *tail;
};

struct file_cache
{
	int fd;
	char pathname[1024];

	size_t cache_blocks;
	size_t block_size;

	off_t start_block;
	off_t read_pointer, write_pointer;
	struct mem_cache read_cache, write_cache;

	ssize_t (*read_blocks)(struct file_cache *cache, void *buff, size_t count);
	ssize_t (*write_blocks)(struct file_cache *cache, const void *buff, size_t count);
};

int mem_cache_init(struct mem_cache *cache, size_t size);
void mem_cache_reinit(struct mem_cache *cache);
void mem_cache_uninit(struct mem_cache *cache);
size_t mem_cache_discard(struct mem_cache *cache, size_t size);
size_t mem_cache_write(struct mem_cache *cache, const char *buff, size_t size);
size_t mem_cache_read(struct mem_cache *cache, char *buff, size_t size);
size_t mem_cache_clean(struct mem_cache *cache, char *buff);

int ffile_cache_init(struct file_cache *cache, size_t read_size, size_t write_size);
int file_cache_init(struct file_cache *cache, const char *pathname, int flags, mode_t mode);
void file_cache_uninit(struct file_cache *cache);
ssize_t file_cache_read(struct file_cache *cache, void *buff, size_t size);
ssize_t file_cache_fflush(struct file_cache *cache);
ssize_t file_cache_clean(struct file_cache *cache);
ssize_t file_cache_write(struct file_cache *cache, const void *buff, size_t size);
off_t file_cache_seek(struct file_cache *cache, off_t offset);

