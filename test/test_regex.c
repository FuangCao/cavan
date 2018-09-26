/*
 * File:		test_regex.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-09-26 11:25:18
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/regex.h>

int main(int argc, char *argv[])
{
	cavan_regex_t regex;
	int ret;

	assert(argc > 2);

	ret = cavan_regex_compile(&regex, argv[1]);
	if (ret < 0) {
		pr_red_info("cavan_regex_compile: %d", ret);
		return ret;
	}

	cavan_regex_dump(&regex);

	println("matched: %d", cavan_regex_match2(&regex, argv[2]));

	return 0;
}
