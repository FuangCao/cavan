/*
 * File:		file.c
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-14
 * Description:	HUAMOBILE LIBRARY
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

#include <huamobile/file.h>

int huamobile_file_mmap(const char *pathname, void **addr, size_t *size, int flags)
{
	int ret;
	int fd;
	struct stat st;
	void *mem;

	pr_bold_info("pathname = %s", pathname);

	fd = open(pathname, flags, 0777);
	if (fd < 0)
	{
		pr_error_info("open file `%s'", pathname);
		return fd;
	}

	ret = flock(fd, LOCK_SH);
	if (ret < 0)
	{
		pr_error_info("flock");
		goto out_close_fd;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		pr_error_info("fstat");
		goto out_unlock_fd;
	}

	if (flags & O_RDWR)
	{
		flags = PROT_READ | PROT_WRITE;
	}
	else if (flags & O_WRONLY)
	{
		flags = PROT_WRITE;
	}
	else
	{
		flags = PROT_READ;
	}

	mem = mmap(NULL, st.st_size, flags, MAP_SHARED, fd, 0);
	if (mem == NULL || mem == MAP_FAILED)
	{
		ret = -EFAULT;
		pr_error_info("mmap");
		goto out_unlock_fd;
	};

	*addr = mem;
	*size = st.st_size;
	return fd;

out_unlock_fd:
	flock(fd, LOCK_UN);
out_close_fd:
	close(fd);
	return ret;
}

void huamobile_file_unmap(int fd, void *map, size_t size)
{
	munmap(map, size);
	flock(fd, LOCK_UN);
	close(fd);
}

void *huamobule_file_read_all(const char *pathname, size_t *size)
{
	int ret;
	int fd;
	struct stat st;
	void *mem;
	ssize_t rdlen;

	pr_bold_info("pathname = %s", pathname);

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file %s", pathname);
		return NULL;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		pr_error_info("get file `%s' stat", pathname);
		close(fd);
		return NULL;
	}

	mem = malloc(st.st_size);
	if (mem == NULL)
	{
		pr_error_info("malloc");
		close(fd);
		return NULL;
	}

	rdlen = read(fd, mem, st.st_size);
	if (rdlen < 0)
	{
		pr_error_info("read");
		free(mem);
		mem = NULL;
	}
	else
	{
		*size = rdlen;
	}

	close(fd);

	return mem;
}
