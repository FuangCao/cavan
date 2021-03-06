/*
 * File:		testCache.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-09-28 14:11:47
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/cache.h>
#include <cavan/thread.h>

#if 0
static void *read_thread_handler(void *data)
{
	ssize_t rdlen;
	char buff[1024];
	struct cavan_cache *cache = data;

	while (1) {
		rdlen = cavan_cache_read_line(cache, buff, sizeof(buff), 0, 20000);
		if (rdlen <= 0) {
			if (rdlen < 0) {
				pr_red_info("cavan_cache_read");
				break;
			} else {
				rdlen = cavan_cache_read(cache, buff, sizeof(buff) - 1, 0, 0);
				if (rdlen <= 0) {
					break;
				}
			}
		}

		buff[rdlen] = 0;

#if __WORDSIZE == 64
		pr_green_info("buff[%ld] = %s", rdlen, buff);
#else
		pr_green_info("buff[%d] = %s", rdlen, buff);
#endif

		// msleep(500);
	}

	pr_red_info("Read thread exit!");
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;
	struct cavan_cache *cache;
	pthread_t rdthread;

	cache = cavan_cache_create(10);
	if (cache == NULL) {
		pr_red_info("cavan_cache_create");
		return -1;
	}

	cavan_cache_open(cache);

	cavan_pthread_create(&rdthread, read_thread_handler, cache, true);

	while (1) {
		ssize_t wrlen;
		char buff[1024];

		ret = scanf("%s", buff);
		if (ret != 1 || strcmp(buff, "exit") == 0) {
			break;
		}

		wrlen = cavan_cache_write(cache, buff, strlen(buff));
		if (wrlen < 0) {
			pr_red_info("cavan_cache_write");
			break;
		}

		wrlen = cavan_cache_write(cache, "\n", 1);
		if (wrlen < 0) {
			pr_red_info("cavan_cache_write");
			break;
		}
	}

	cavan_cache_close(cache);

	pr_pos_info();
	pthread_join(rdthread, NULL);

	pr_pos_info();
	cavan_cache_destroy(cache);

	pr_pos_info();
	msleep(1000);
	pr_pos_info();

	return 0;
}
#else
int main(int argc, char *argv[])
{
	struct cavan_block_cache cache;
	int readed;
	int i;

	cavan_block_cache_init(&cache);

	for (i = 0; i < 100000; i++) {
		char value = '0' + (i % 10);
		cavan_block_cache_write(&cache, &value, 1);
	}

	readed = 0;

	while (1) {
		char buff[1024];
		int length = cavan_block_cache_read(&cache, buff, sizeof(buff) - 1);

		if (length == 0) {
			break;
		}

		readed += length;

		buff[length] = 0;
		println("length = %d, buff = %s", length, buff);

		cavan_block_cache_write(&cache, "ABCDEF", 6);
		msleep(10);
	}

	println("readed = %d", readed);

	cavan_block_cache_deinit(&cache);

	return 0;
}
#endif
