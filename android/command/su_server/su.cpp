/*
 * File:		su.cpp
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

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

#if 1
	int ret;
	pid_t pid;
	int ttyfds[3];
	int flags = 1 << 1;

	ret = su->popen(argv[1], &pid, flags);
	if (ret < 0) {
		fprintf(stderr, "Failed to popen: %d\n", ret);
		return ret;
	}

	ret = cavan_exec_open_temp_pipe_client(ttyfds, pid, flags);
	if (ret < 0) {
		pr_red_info("cavan_exec_open_temp_pipe_client: %d", ret);
		return ret;
	}

	while (1) {
		ssize_t rdlen;
		char buff[1024];

		rdlen = read(ttyfds[1], buff, sizeof(buff));
		if (rdlen <= 0) {
			break;
		}

		if (write(1, buff, rdlen) != rdlen) {
			break;
		}
	}
#else
	int ret = su->system(argv[1]);
	if (ret < 0) {
		fprintf(stderr, "Failed to system: %d\n", ret);
		return ret;
	}
#endif

	return 0;
}
