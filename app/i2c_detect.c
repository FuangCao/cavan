/*
 * File:		i2c_detect.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-28 17:48:30
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
#include <cavan/i2c.h>

int main(int argc, char *argv[])
{
	int ret;
	int index;
	struct cavan_i2c_client client;

	assert(argc > 1);

	index = text2value_unsigned(argv[1], NULL, 10);
	println("index = %d", index);

	ret = cavan_i2c_init(&client, index, NULL);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	cavan_i2c_detect(&client);
	cavan_i2c_deinit(&client);

	return 0;
}
