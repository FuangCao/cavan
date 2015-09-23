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
#include <cavan.h>

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
	char pathname[1024];

	int ret = su->popen(argv[1], pathname, sizeof(pathname));
	if (ret < 0) {
		fprintf(stderr, "Failed to popen: %d\n", ret);
		return ret;
	}

	int fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open file %s: %s\n", pathname, strerror(errno));
		return fd;
	}

	while (1) {
		ssize_t rdlen;
		char buff[1024];

		rdlen = read(fd, buff, sizeof(buff));
		if (rdlen <= 0) {
			break;
		}

		if (fwrite(buff, 1, rdlen, stdout) < (size_t) rdlen) {
			break;
		}
	}

	close(fd);
#else
	int ret = su->system(argv[1]);
	if (ret < 0) {
		fprintf(stderr, "Failed to system: %d\n", ret);
		return ret;
	}
#endif

	return 0;
}
