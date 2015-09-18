/*
 * File:		main_tcp_dd.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-18 11:55:09
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

#include "ISuService.h"

using namespace android;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Too a few argument!\n");
		return -EINVAL;
	}

	sp<ISuService> su = ISuService::getService();
	if (su == NULL) {
		fprintf(stderr, "Failed to SuService::getService()\n");
		return -EFAULT;
	}

	status_t status = su->runCommand(argv[1]);
	printf("status = %d\n", status);

	return 0;
}
