/*
 * File:		testQueue.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-30 11:24:29
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/queue.h>

int main(int argc, char *argv[])
{
	int i;
	int ret;
	char buff[1024];
	struct cavan_mem_queue queue;

	ret = cavan_mem_queue_init(&queue, 16);
	if (ret < 0)
	{
		pr_red_info("cavan_mem_queue_init: %d", ret);
		return ret;
	}

	println("%d. used_size = %ld", __LINE__, cavan_mem_queue_get_used_size(&queue));
	println("%d. free_size = %ld", __LINE__, cavan_mem_queue_get_free_size(&queue));

	for (i = 0; i < 10; i++)
	{
		char c = '0' + i;

		cavan_mem_queue_inqueue(&queue, &c, 1);
	}

	println("%d. used_size = %ld", __LINE__, cavan_mem_queue_get_used_size(&queue));
	println("%d. free_size = %ld", __LINE__, cavan_mem_queue_get_free_size(&queue));

	ret = cavan_mem_queue_dequeue(&queue, buff, sizeof(buff));
	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

	for (i = 0; i < 26; i++)
	{
		char c = 'A' + i;

		cavan_mem_queue_inqueue(&queue, &c, 1);
		ret = cavan_mem_queue_dequeue(&queue, buff, sizeof(buff));
		buff[ret] = 0;
		println("buff[%d] = %s", ret, buff);

		println("%d. used_size = %ld", __LINE__, cavan_mem_queue_get_used_size(&queue));
		println("%d. free_size = %ld", __LINE__, cavan_mem_queue_get_free_size(&queue));
	}

	for (i = 0; i < 26; i += 2)
	{
		char chars[2] = { 'A' + i, 'A' + i + 1 };

		cavan_mem_queue_inqueue(&queue, &chars, 2);
		ret = cavan_mem_queue_dequeue(&queue, buff, 1);
		buff[ret] = 0;
		println("buff[%d] = %s", ret, buff);

		println("%d. used_size = %ld", __LINE__, cavan_mem_queue_get_used_size(&queue));
		println("%d. free_size = %ld", __LINE__, cavan_mem_queue_get_free_size(&queue));
	}

	ret = cavan_mem_queue_dequeue(&queue, buff, sizeof(buff));
	buff[ret] = 0;
	println("buff[%d] = %s", ret, buff);

	println("%d. used_size = %ld", __LINE__, cavan_mem_queue_get_used_size(&queue));
	println("%d. free_size = %ld", __LINE__, cavan_mem_queue_get_free_size(&queue));

	cavan_mem_queue_deinit(&queue);

	return 0;
}
