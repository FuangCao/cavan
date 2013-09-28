// Fuang.Cao <cavan.cfa@gmail.com> Mon Jun 27 09:56:25 CST 2011

#include <cavan.h>
#include <cavan/cache.h>
#include <cavan/timer.h>

#define CAVAN_CACHE_DEBUG	0

int mem_cache_init(struct mem_cache *cache, size_t size)
{
	if(cache == NULL)
	{
		return -EINVAL;
	}

	if (size == 0)
	{
		cache->buff = NULL;
		return 0;
	}

	cache->buff = malloc(size);
	if (cache->buff == NULL)
	{
		return -ENOMEM;
	}

	cache->size = cache->free_size = size;
	cache->end = cache->buff + size;
	cache->tail = cache->head = cache->buff;

	return 0;
}

void mem_cache_deinit(struct mem_cache *cache)
{
	if (cache == NULL || cache->buff == NULL)
	{
		return;
	}

	free(cache->buff);
}

size_t mem_cache_write(struct mem_cache *cache, const char *buff, size_t size)
{
	size_t size_bak;
	char *head;
	const char *end;

	size_bak = size = size <= cache->free_size ? size : cache->free_size;

	for (head = cache->head, end = cache->end; size && head < end; size--)
	{
		*head++ = *buff++;
	}

	if (size == 0)
	{
		goto out_return;
	}

	for (head = cache->buff, end = cache->tail; size && head < end; size--)
	{
		*head++ = *buff++;
	}

out_return:
	cache->head = head;
	cache->free_size -= size_bak;

	return size_bak;
}

size_t mem_cache_read(struct mem_cache *cache, char *buff, size_t size)
{
	size_t size_bak;
	const char *tail, *end;

	size_bak = cache->size - cache->free_size;
	size_bak = size = size <= size_bak ? size : size_bak;

	for (tail = cache->tail, end = cache->end; size && tail < end; size--)
	{
		*buff++ = *tail++;
	}

	if (size == 0)
	{
		goto out_return;
	}

	for (tail = cache->buff, end = cache->head; size && tail < end; size--)
	{
		*buff++ = *tail++;
	}

out_return:
	cache->tail = tail;
	cache->free_size += size_bak;

	return size_bak;
}

void mem_cache_reinit(struct mem_cache *cache)
{
	cache->tail = cache->head = cache->buff;
	cache->free_size = cache->size;
}

size_t mem_cache_discard(struct mem_cache *cache, size_t size)
{
	size_t used_size;

	used_size = cache->size - cache->free_size;
	if (size >= used_size)
	{
		mem_cache_reinit(cache);
		return used_size;
	}

	if (cache->head > cache->tail)
	{
		cache->tail += size;
	}
	else
	{
		cache->tail += size;
		if (cache->tail >= cache->end)
		{
			cache->tail = cache->buff + (size - (cache->end - cache->tail));
		}
	}

	cache->free_size += size;

	return size;
}

size_t mem_cache_clean(struct mem_cache *cache, char *buff)
{
	size_t size, size_bak;
	const char *tail, *end;

	size = size_bak = cache->size - cache->free_size;

	for (tail = cache->tail, end = cache->end; size && tail < end; size--)
	{
		*buff++ = *tail++;
	}

	if (size == 0)
	{
		goto out_return;
	}

	for (tail = cache->buff, end = cache->head; size && tail < end; size--)
	{
		*buff++ = *tail++;
	}

out_return:
	mem_cache_reinit(cache);

	return size_bak;
}

// ============================================================

int ffile_cache_init(struct file_cache *cache, size_t read_size, size_t write_size)
{
	int ret;

	if (cache == NULL)
	{
		return -EINVAL;
	}

	ret = mem_cache_init(&cache->read_cache, read_size);
	if (ret < 0)
	{
		return ret;
	}

	ret = mem_cache_init(&cache->write_cache, write_size);
	if (ret < 0)
	{
		goto out_read_cache_deinit;
	}

	return 0;

out_read_cache_deinit:
	mem_cache_deinit(&cache->read_cache);

	return ret;
}

int file_cache_init(struct file_cache *cache, const char *pathname, int flags, mode_t mode)
{
	int ret;
	int fd;

	if (cache == NULL)
	{
		return -EINVAL;
	}

	fd = open(pathname, flags, mode);
	if (fd < 0)
	{
		return fd;
	}

	if (cache->cache_blocks == 0)
	{
		cache->cache_blocks = DEFAULT_CACHE_BLOCKS;
	}

	if (cache->block_size == 0)
	{
		cache->block_size = DEFAULT_BLOCK_SIZE;
	}

	ret = ffile_cache_init(cache, cache->cache_blocks * cache->block_size, cache->cache_blocks * cache->block_size);
	if (ret < 0)
	{
		goto out_close_fd;
	}

	cache->fd = fd;
	cache->start_block = 0;
	cache->read_pointer = cache->write_pointer = cache->start_block;
	text_copy(cache->pathname, pathname);

	return 0;

out_close_fd:
	close(fd);

	return ret;
}

void file_cache_deinit(struct file_cache *cache)
{
	if (cache == NULL)
	{
		return;
	}

	file_cache_fflush(cache);
	mem_cache_deinit(&cache->read_cache);
	mem_cache_deinit(&cache->write_cache);
	close(cache->fd);
}

static ssize_t file_cache_fill(struct file_cache *cache)
{
	struct mem_cache *read_cache;
	ssize_t readblk;

	if (cache->read_blocks == NULL)
	{
		return -EINVAL;
	}

	read_cache = &cache->read_cache;
	readblk = cache->read_blocks(cache, read_cache->buff, cache->cache_blocks);
	if (readblk < 0)
	{
		return readblk;
	}

	read_cache->tail = read_cache->head = read_cache->buff;
	read_cache->free_size = read_cache->size - readblk * cache->block_size;

	cache->read_pointer += readblk;

	return readblk;
}

ssize_t file_cache_read(struct file_cache *cache, void *buff, size_t size)
{
	struct mem_cache *read_cache;
	size_t mem_cache_readlen;
	ssize_t readlen, total_read;

	total_read = 0;

	for (read_cache = &cache->read_cache; size; size -= mem_cache_readlen)
	{
		mem_cache_readlen = mem_cache_read(read_cache, buff, size);
		total_read += mem_cache_readlen;
		if (mem_cache_readlen == size)
		{
			break;
		}

		readlen = file_cache_fill(cache);
		if (readlen < 0)
		{
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}
	}

	return total_read;
}

ssize_t file_cache_fflush(struct file_cache *cache)
{
	struct mem_cache *write_cache = &cache->write_cache;;
	char buff[write_cache->size];
	ssize_t writelen;

	if (cache->write_blocks == NULL)
	{
		return -EINVAL;
	}

	writelen = mem_cache_clean(write_cache, buff);
	if (writelen == 0)
	{
		return 0;
	}

	writelen = cache->write_blocks(cache, buff, (writelen + (cache->block_size - 1)) / cache->block_size);
	if (writelen < 0)
	{
		return writelen;
	}

	cache->write_pointer += writelen;

	return writelen;
}

ssize_t file_cache_clean(struct file_cache *cache)
{
	struct mem_cache *write_cache = &cache->write_cache;
	ssize_t writelen = write_cache->size;

	if (cache->write_blocks == NULL)
	{
		return -EINVAL;
	}

	write_cache = &cache->write_cache;
	writelen = cache->write_blocks(cache, write_cache->buff, cache->cache_blocks);
	if (writelen < 0)
	{
		return writelen;
	}

	write_cache->free_size = write_cache->size;
	write_cache->tail = write_cache->head = write_cache->buff;
	cache->write_pointer += writelen;

	return writelen;
}

ssize_t file_cache_write(struct file_cache *cache, const void *buff, size_t size)
{
	struct mem_cache *write_cache;
	size_t mem_cache_writelen;
	ssize_t writelen, size_bak;

	size_bak = size;

	for (write_cache = &cache->write_cache; size; size -= mem_cache_writelen)
	{
		mem_cache_writelen = mem_cache_write(write_cache, buff, size);
		if (mem_cache_writelen == size)
		{
			break;
		}

		writelen = file_cache_clean(cache);
		if (writelen < 0)
		{
			return writelen;
		}
	}

	return size_bak;
}

off_t file_cache_seek(struct file_cache *cache, off_t offset)
{
	ssize_t readblk;
	struct mem_cache *read_cache, *write_cache;

	cache->read_pointer = cache->write_pointer = offset / cache->block_size;

	read_cache = &cache->read_cache;
	write_cache = &cache->write_cache;

	readblk = cache->read_blocks(cache, read_cache->buff, cache->cache_blocks);
	if (readblk < 0)
	{
		return readblk;
	}

	offset %= cache->block_size;

	read_cache->head = read_cache->buff;
	read_cache->tail = read_cache->buff + offset;
	read_cache->free_size = read_cache->size - readblk * cache->block_size + offset;
	cache->read_pointer += readblk;

	mem_copy(write_cache->buff, read_cache->buff, write_cache->size);
	write_cache->tail = write_cache->buff;
	write_cache->head = write_cache->buff + offset;
	write_cache->free_size = write_cache->size - offset;

	return 0;
}

// ============================================================

int cavan_cache_init(struct cavan_cache *cache, void *mem, size_t size)
{
	int ret;

	ret = pthread_mutex_init(&cache->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	ret = pthread_cond_init(&cache->rdcond, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_cond_init");
		goto out_pthread_mutex_destroy_lock;
	}

	ret = pthread_cond_init(&cache->wrcond, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_cond_init");
		goto out_pthread_cond_destroy_rdcond;
	}

	cache->size = size;
	cache->mem = mem;
	cache->mem_end = cache->mem + size;

	return 0;

out_pthread_cond_destroy_wrcond:
	pthread_cond_destroy(&cache->wrcond);
out_pthread_cond_destroy_rdcond:
	pthread_cond_destroy(&cache->rdcond);
out_pthread_mutex_destroy_lock:
	pthread_mutex_destroy(&cache->lock);
	return ret;
}

void cavan_cache_deinit(struct cavan_cache *cache)
{
	pthread_cond_destroy(&cache->wrcond);
	pthread_cond_destroy(&cache->rdcond);
	pthread_mutex_destroy(&cache->lock);
}

struct cavan_cache *cavan_cache_create(size_t size)
{
	struct cavan_cache *cache;

	cache = malloc(sizeof(struct cavan_cache) + size);
	if (cache == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	if (cavan_cache_init(cache, (void *)(cache + 1), size) < 0)
	{
		pr_red_info("cavan_cache_init");
		free(cache);
		return NULL;
	}

	return cache;
}

void cavan_cache_destroy(struct cavan_cache *cache)
{
	cavan_cache_deinit(cache);
	free(cache);
}

void cavan_cache_open(struct cavan_cache *cache)
{
	pthread_mutex_lock(&cache->lock);
	cache->closed = false;
	cache->head = cache->tail = cache->mem;
	pthread_mutex_unlock(&cache->lock);
}

void cavan_cache_close(struct cavan_cache *cache)
{
	pthread_mutex_lock(&cache->lock);
	cache->closed = true;
	pthread_mutex_unlock(&cache->lock);

	pthread_cond_broadcast(&cache->rdcond);
	pthread_cond_broadcast(&cache->wrcond);
}

ssize_t cavan_cache_free_space(struct cavan_cache *cache)
{
	if (cache->tail < cache->head)
	{
		return cache->head - cache->tail - 1;
	}
	else
	{
		return (cache->mem_end - cache->tail) + (cache->head - cache->mem) - 1;
	}
}

ssize_t cavan_cache_used_space(struct cavan_cache *cache)
{
	if (cache->head > cache->tail)
	{
		return (cache->mem_end - cache->head) + (cache->tail - cache->mem);
	}
	else
	{
		return cache->tail - cache->head;
	}
}

char *cavan_cache_pointer_add(struct cavan_cache *cache, char *pointer, off_t offset)
{
	pointer += offset;

	while (pointer >= cache->mem_end)
	{
		pointer -= cache->size;
	}

	return pointer;
}

ssize_t cavan_cache_write(struct cavan_cache *cache, const char *buff, size_t size)
{
	const char *buff_bak = buff;
	const char *buff_end = buff + size;

	pthread_mutex_lock(&cache->lock);

	while (1)
	{
		size_t length, rcount;
		size_t remain;

		remain = buff_end - buff;
		if (remain == 0)
		{
			break;
		}

		while (1)
		{
			if (cache->tail < cache->head)
			{
				length = rcount = cache->head - cache->tail - 1;
			}
			else
			{
				rcount = cache->mem_end - cache->tail;
				length = rcount + (cache->head - cache->mem) - 1;
			}

#if CAVAN_CACHE_DEBUG
			println("Write: head = %p, tail = %p, rcount = %d, length = %d, free_space = %d",
				cache->head, cache->tail, rcount, length, cavan_cache_free_space(cache));
#endif

			if (length > 0)
			{
				break;
			}

			if (cache->closed)
			{
				size = buff - buff_bak;
				goto out_pthread_mutex_unlock;
			}

			pthread_cond_wait(&cache->wrcond, &cache->lock);
		}

		if (length > remain)
		{
			length = remain;
		}

		if (length > rcount)
		{
			size_t lcount = length - rcount;

			memcpy(cache->tail, buff, rcount);
			memcpy(cache->mem, buff + rcount, lcount);
			cache->tail = cache->mem + lcount;
		}
		else
		{
			memcpy(cache->tail, buff, length);

			cache->tail += length;
			if (cache->tail >= cache->mem_end)
			{
				cache->tail = cache->mem;
			}
		}

		buff += length;

		pthread_cond_signal(&cache->rdcond);
	}

out_pthread_mutex_unlock:
	pthread_mutex_unlock(&cache->lock);
	return size;
}

ssize_t cavan_cache_read(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout)
{
	ssize_t length, rcount;

	pthread_mutex_lock(&cache->lock);

	while (1)
	{
		if (cache->head > cache->tail)
		{
			rcount = cache->mem_end - cache->head;
			length = rcount + (cache->tail - cache->mem);
		}
		else
		{
			length = rcount = cache->tail - cache->head;
		}

#if CAVAN_CACHE_DEBUG
		println("Read: head = %p, tail = %p, rcount = %d, length = %d, used_space = %d",
			cache->head, cache->tail, rcount, length, cavan_cache_used_space(cache));
#endif

		if ((size_t) length > reserved)
		{
			length -= reserved;
			break;
		}

		if (cache->closed)
		{
			length = 0;
			goto out_pthread_mutex_unlock;
		}

		if (timeout > 0)
		{
			int ret;
			struct timespec abstime;

			cavan_timer_set_timespec(&abstime, timeout);

			ret = pthread_cond_timedwait(&cache->rdcond, &cache->lock, &abstime);
			if (ret != 0)
			{
				length = -ETIMEDOUT;
				pr_red_info("pthread_cond_timedwait");
				goto out_pthread_mutex_unlock;
			}
		}
		else
		{
			pthread_cond_wait(&cache->rdcond, &cache->lock);
		}
	}

	if ((size_t) length > size)
	{
		length = size;
	}

	if (length > rcount)
	{
		size_t lcount = length - rcount;

		memcpy(buff, cache->head, rcount);
		memcpy(buff + rcount, cache->mem, lcount);
		cache->head = cache->mem + lcount;
	}
	else
	{
		memcpy(buff, cache->head, length);

		cache->head += length;
		if (cache->head >= cache->mem_end)
		{
			cache->head = cache->mem;
		}
	}

	pthread_cond_signal(&cache->wrcond);

out_pthread_mutex_unlock:
	pthread_mutex_unlock(&cache->lock);
	return length;
}

ssize_t cavan_cache_fill(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout)
{
	char *buff_bak = buff;
	char *buff_end = buff + size;

	while (buff < buff_end)
	{
		ssize_t rdlen = cavan_cache_read(cache, buff, buff_end - buff, reserved, timeout);
		if (rdlen < 0)
		{
			pr_red_info("cavan_cache_read");
			return rdlen;
		}

		if (rdlen == 0)
		{
			size = buff - buff_bak;
			break;
		}

		buff += rdlen;
	}

	return size;
}

ssize_t cavan_cache_read_line(struct cavan_cache *cache, char *buff, size_t size, size_t reserved, u32 timeout)
{
	char *buff_bak = buff;
	char *buff_end = buff + size - 1;

	while (buff < buff_end)
	{
		char c;
		ssize_t rdlen;

		rdlen = cavan_cache_read(cache, &c, 1, reserved, timeout);
		if (rdlen < 0)
		{
			pr_red_info("cavan_cache_read");
			return rdlen;
		}

		if (rdlen == 0)
		{
			break;
		}

		switch (c)
		{
		case '\n':
			if (buff > buff_bak)
			{
				goto out_return;
			}
		case '\r':
			break;

		default:
			*buff++ = c;
		}
	}

out_return:
	*buff = 0;
	return buff - buff_bak;
}
