/*
 * File:		cpp_http_service.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-04-09 17:13:07
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
#include <cavan++/HttpFileManager.h>

int main(int argc, char *argv[])
{
	HttpFileService service;
	int ret;

	assert(argc > 1);

	ret = service.open(argv[1]);
	if (ret < 0) {
		pr_red_info("service.open");
		return ret;
	}

	service.join();

	return 0;
}
