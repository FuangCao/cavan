/*
 * File:		SuService.cpp
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

#include "SuService.h"

namespace android {

static int cavan_redirect_stdio_base(int fds[3], int flags)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (flags & (1 << i)) {
			int ret;

			ret = dup2(fds[i], i);
			if (ret < 0) {
				ALOGE("dup2 stdio %d", i);
				return ret;
			}
		}
	}

	for (i = 0; i < 3; i++) {
		if (fds[i] >= 0 && flags & (1 << i)) {
			close(fds[i]);
		}
	}

	return 0;
}

static int cavan_exec_command(const char *command)
{
	const char *shell_command = "sh";

	if (command && command[0]) {
		return execlp(shell_command, shell_command, "-c", command, NULL);
	} else {
		return execlp(shell_command, shell_command, "-", NULL);
	}
}

static int cavan_exec_redirect_stdio_base(const char *command, int ttyfds[3], int flags)
{
	int ret;

	ret = cavan_redirect_stdio_base(ttyfds, flags);
	if (ret < 0) {
		ALOGE("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command);
}

static int cavan_exec_redirect_stdio_popen(const char *command, char *pathname, size_t size)
{
	int ret;
	pid_t pid;

	strncpy(pathname, "/dev/stdout-XXXXXX", size);

	ret = mkstemp(pathname);
	if (ret < 0) {
		ALOGE("Failed to mkstemp stdout: %s", strerror(errno));
		return ret;
	}

	ALOGE("pathname = %s", pathname);

	unlink(pathname);

	ret = mkfifo(pathname, 0777 | S_IFIFO);
	if (ret < 0) {
		ALOGE("Failed to create stdout pipe: %s", strerror(errno));
		return ret;
	}

	pid = fork();
	if (pid < 0) {
		ALOGE("fork");
		unlink(pathname);
		return pid;
	}

	if (pid == 0) {
		int fd;
		int ttyfds[3];

		fd = open(pathname, O_WRONLY);
		if (fd < 0) {
			ALOGE("Failed to open %s: %s", pathname, strerror(errno));
			return ret;
		}

		unlink(pathname);

		ttyfds[0] = -1;
		ttyfds[1] = ttyfds[2] = fd;

		return cavan_exec_redirect_stdio_base(command, ttyfds, 0x06);
	}

	return 0;
}

int SuService::system(const char *command)
{
	ALOGE("%s[%d]: command = %s", __FUNCTION__, __LINE__, command);

	if (::system(command)) {
		return -EFAULT;
	}

	return 0;
}

int SuService::popen(const char *command, char *pathname, size_t size)
{
	int ret;

	ALOGE("%s[%d]: command = %s", __FUNCTION__, __LINE__, command);

	ret = cavan_exec_redirect_stdio_popen(command, pathname, size);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

};
