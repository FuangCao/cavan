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

struct cavan_cache
{
	char *head;
	char *tail;

	char *mem;
	char *mem_end;

	bool closed;
	size_t size;

	pthread_mutex_t lock;
	pthread_cond_t rdcond;
	pthread_cond_t wrcond;
};

struct cavan_fifo
{
	char *mem;
	char *mem_end;
	char *data;
	char *data_end;
	size_t size;
	void *private_data;
	pthread_mutex_t lock;

	ssize_t (*read)(struct cavan_fifo *fifo, void *buff, size_t size);
	ssize_t (*write)(struct cavan_fifo *fifo, const void *buff, size_t size);
};

int mem_cache_init(struct mem_cache *cache, size_t size);
void mem_cache_reinit(struct mem_cache *cache);
void mem_cache_deinit(struct mem_cache *cache);
size_t mem_cache_discard(struct mem_cache *cache, size_t size);
size_t mem_cache_write(struct mem_cache *cache, const char *buff, size_t size);
size_t mem_cache_read(struct mem_cache *cache, char *buff, size_t size);
size_t mem_cache_clean(struct mem_cache *cache, char *buff);

int ffile_cache_init(struct file_cache *cache, size_t read_size, size_t write_size);
int file_cache_init(struct file_cache *cache, const char *pathname, int flags, mode_t mode);
void file_cache_deinit(struct file_cache *cache);
ssize_t file_cache_read(struct file_cache *cache, void *buff, size_t size);
ssize_t file_cache_fflush(struct file_cache *cache);
ssize_t file_cache_clean(struct file_cache *cache);
ssize_t file_cache_write(struct file_cache *cache, const void *buff, size_t size);
off_t file_cache_seek(struct file_cache *cache, off_t offset);

int cavan_cache_init(struct cavan_cache *cache, void *mem, size_t size);
void cavan_cache_deinit(struct cavan_cache *cache);
struct cavan_cache *cavan_cache_create(size_t size);
void cavan_cache_destroy(struct cavan_cache *cache);
void cavan_cache_open(struct cavan_cache *cache);
void cavan_cache_close(struct cavan_cache *cache);
ssize_t cavan_cache_free_space(struct cavan_cache *cache);
ssize_t cavan_cache_used_space(struct cavan_cache *cache);
char *cavan_cache_pointer_add(struct cavan_cache *cache, char *pointer, off_t offset);
ssize_t cavan_cache_write(struct cavan_cache *cache, const char *buff, size_t size);
ssize_t cavan_cache_read(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t cavan_cache_fill(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t cavan_cache_read_line(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout);

int cavan_fifo_init(struct cavan_fifo *fifo, size_t size, void *data);
void cavan_fifo_deinit(struct cavan_fifo *fifo);
ssize_t cavan_fifo_read(struct cavan_fifo *fifo, void *buff, size_t size);
ssize_t cavan_fifo_read_cache(struct cavan_fifo *fifo, void *buff, size_t size);
ssize_t cavan_fifo_read_line(struct cavan_fifo *fifo, char *buff, size_t size);
ssize_t cavan_fifo_fill(struct cavan_fifo *fifo, void *buff, size_t size);
ssize_t cavan_fifo_write(struct cavan_fifo *fifo, const void *buff, size_t size);
ssize_t cavan_fifo_fflush(struct cavan_fifo *fifo);
size_t cavan_fifo_vprintf(struct cavan_fifo *fifo, const char *format, va_list ap);
size_t cavan_fifo_printf(struct cavan_fifo *fifo, const char *format, ...);

static inline void cavan_fifo_lock(struct cavan_fifo *fifo)
{
	pthread_mutex_lock(&fifo->lock);
}

static inline void cavan_fifo_unlock(struct cavan_fifo *fifo)
{
	pthread_mutex_unlock(&fifo->lock);
}
