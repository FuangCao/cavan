/*
 * File:		testCommand.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-23 16:55:59
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
#include <cavan/command.h>

int main(int argc, char *argv[])
{
	int ret;
	pid_t pid;
	int size[2];
	int ttyfds[3];
	int flags = CAVAN_EXECF_STDIN | CAVAN_EXECF_STDOUT | CAVAN_EXECF_STDERR;

	assert(argc > 1);

	tty_get_win_size(0, size);

	ret = cavan_exec_redirect_stdio_popen2(argv[1], size[0], size[1], &pid, flags);
	if (ret < 0)
	{
		pr_red_info("cavan_exec_redirect_stdio_popen2");
		return ret;
	}

	ret = cavan_exec_open_temp_pipe_slave(ttyfds, pid, flags);
	if (ret < 0)
	{
		pr_red_info("cavan_exec_open_temp_pipe_client: %d", ret);
		return ret;
	}

	cavan_tty_redirect(ttyfds[0], ttyfds[1], ttyfds[2]);

	cavan_exec_close_temp_pipe(ttyfds);

	return cavan_exec_waitpid(pid);
}
