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
	int lines, columns;

	if (isatty(0))
	{
		u16 size[2];

		ret = tty_get_win_size(0, size);
		if (ret < 0)
		{
			lines = columns = 0;
		}
		else
		{
			lines = size[0];
			columns = size[1];
		}
	}
	else
	{
		lines = columns = -1;
	}

	ret = su->popen(argv[1], lines, columns, &pid, flags);
	if (ret < 0) {
		fprintf(stderr, "Failed to popen: %d\n", ret);
		return ret;
	}

	ret = cavan_exec_open_temp_pipe_slave(ttyfds, pid, flags);
	if (ret < 0) {
		pr_red_info("cavan_exec_open_temp_pipe_client: %d", ret);
		return ret;
	}

	cavan_tty_redirect(ttyfds[0], ttyfds[1], ttyfds[2]);

	return cavan_exec_waitpid(pid);
#else
	int ret = su->system(argv[1]);
	if (ret < 0) {
		fprintf(stderr, "Failed to system: %d\n", ret);
		return ret;
	}
#endif

	return 0;
}
