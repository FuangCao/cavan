/*
 * File:		testFifo.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-07-31 12:08:32
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

static ssize_t test_fifo_read(struct cavan_fifo *fifo, void *buff, size_t size)
{
	pr_pos_info();

	return text_ncopy(buff, "0123456789", size) - (char *) buff;
}

static ssize_t test_fifo_write(struct cavan_fifo *fifo, const void *buff, size_t size)
{
	println("buff = %p, size = %" PRINT_FORMAT_SIZE, buff, size);

	print_ntext(buff, size);
	print_char('\n');

	return size;
}

int main(int argc, char *argv[])
{
	char c;
	int ret;
	struct cavan_fifo fifo;

	assert(argc > 1);

	ret = cavan_fifo_init(&fifo, 20, NULL);
	if (ret < 0)
	{
		pr_red_info("cavan_fifo_init");
		return ret;
	}

	fifo.read = test_fifo_read;
	fifo.write = test_fifo_write;

	if (strcmp(argv[1], "write") == 0)
	{
		for (c = 'A'; c <= 'Z'; c++)
		{
			cavan_fifo_write(&fifo, &c, 1);
		}

		cavan_fifo_fflush(&fifo);
	}
	else if (strcmp(argv[1], "read") == 0)
	{
		int  i;
		char buff[100];

		for (i = 0; i < (int) sizeof(buff) - 1 && cavan_fifo_read(&fifo, buff + i, 1) == 1; i++);

		buff[i] = 0;

		println("buff[%d] = %s", i, buff);
	}
	else
	{
		pr_red_info("inlivad action %s", argv[1]);
	}

	cavan_fifo_deinit(&fifo);

	return 0;
}
