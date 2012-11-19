// Fuang.Cao <cavan.cfa@gmail.com> Mon Jun 27 09:56:25 CST 2011

#include <cavan.h>
#include <cavan/cache.h>

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

void mem_cache_uninit(struct mem_cache *cache)
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
		goto out_read_cache_uninit;
	}

	return 0;

out_read_cache_uninit:
	mem_cache_uninit(&cache->read_cache);

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

void file_cache_uninit(struct file_cache *cache)
{
	if (cache == NULL)
	{
		return;
	}

	file_cache_fflush(cache);
	mem_cache_uninit(&cache->read_cache);
	mem_cache_uninit(&cache->write_cache);
	close(cache->fd);
}

ssize_t file_cache_fill(struct file_cache *cache)
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

