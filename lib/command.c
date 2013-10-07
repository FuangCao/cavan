/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <cavan/command.h>

void print_command_table(const struct cavan_command_map *p, size_t size)
{
	const struct cavan_command_map *p_end;

	if (size == 0)
	{
		return;
	}

	pr_bold_info("Available command is:");

	for (p_end = p + size - 1; p < p_end; p++)
	{
		print("%s, ", p->name);
	}

	print_string(p->name);
}

const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size)
{
	while (p < p_end)
	{
		if (text_ncmp(cmdname, p->name, size) == 0)
		{
			return p;
		}

		p++;
	}

	return NULL;
}

void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	int size;
	const struct cavan_command_map *q;

	for (size = text_len(cmdname); size && ((q = find_command_by_name(p, p_end, cmdname, size))) == NULL; size--);

	if (size)
	{
		int i;
		int count;
		const struct cavan_command_map *matchs[100];

		pr_bold_info("This command maybe:");

		for (count = 0, p = q; count < NELEM(matchs); count++)
		{
			matchs[count] = p;

			p = find_command_by_name(p + 1, p_end, cmdname, size);
			if (p == NULL)
			{
				break;
			}
		}

		for (i = 0; i < count; i++)
		{
			print("%s, ", matchs[i]->name);
		}

		print_string(matchs[i]->name);
	}
	else
	{
		pr_red_info("`%s' No such command", cmdname);
	}
}

const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	while (p < p_end)
	{
		if (text_cmp(cmdname, p->name) == 0)
		{
			return p;
		}

		p++;
	}

	return NULL;
}

int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[])
{
	const struct cavan_command_map *p;
	const char *pcmd;

	if (argc > 1 && argv[1][0] == '@')
	{
		pcmd = argv[1] + 1;
		argc--;
		argv++;
	}
	else
	{
		const char *q;

		for (pcmd = argv[0], q = pcmd; *q; q++)
		{
			if (*q == '/')
			{
				pcmd = q + 1;
			}
		}
	}

	p = match_command_by_name(map, map + count, pcmd);
	if (p == NULL && argc > 1 && file_test(argv[0], "l") < 0)
	{
		pcmd = argv[1];
		argc--;
		argv++;

		p = match_command_by_name(map, map + count, pcmd);
	}

	if (p)
	{
		return p->main_func(argc, argv);
	}

	print_maybe_command(map, map + count, pcmd);

	return -1;
}

int cavan_redirect_stdio_base(int ttyfd, int flags)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (flags & (1 << i))
		{
			int ret;

			ret = dup2(ttyfd, i);
			if (ret < 0)
			{
				pr_error_info("dup2 stdio %d", i);
				return ret;
			}
		}
	}

	return 0;
}

int cavan_redirect_stdio(const char *pathname, int flags)
{
	int fd;
	int ret;
	int open_flags;

	pr_bold_info("pathname = %s, flags = 0x%02x", pathname, flags);

	if ((flags & 0x01))
	{
		if ((flags & 0x06))
		{
			open_flags = O_RDWR;
		}
		else
		{
			open_flags = O_RDONLY;
		}
	}
	else
	{
		open_flags = O_WRONLY;
	}

	fd = open(pathname, open_flags | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		pr_error_info("open file `%s' failed", pathname);
		return fd;
	}

	ret = cavan_redirect_stdio_base(fd, flags);

	close(fd);

	return ret;
}

int cavan_exec_redirect_stdio_base(const char *ttypath, int lines, int columns, const char *command)
{
	int ret;
	int ttyfd;
	const char *shell_command = "sh";

	ret = setsid();
	if (ret < 0)
	{
		pr_error_info("setsid");
		return ret;
	}

	ttyfd = open(ttypath, O_RDWR);
	if (ttyfd < 0)
	{
		pr_error_info("open file %s", ttypath);
		return ttyfd;
	}

	if (isatty(ttyfd) && lines > 0 && columns > 0)
	{
		struct winsize wsize =
		{
			.ws_row = lines,
			.ws_col = columns,
			.ws_xpixel = 0,
			.ws_ypixel = 0
		};
		char buff[64];

		ret = ioctl(ttyfd, TIOCSWINSZ, &wsize);
		if (ret < 0)
		{
			pr_error_info("ioctl TIOCSWINSZ");
			return ret;
		}

		sprintf(buff, "LINES=%d", lines);
		putenv(buff);
		sprintf(buff, "COLUMNS=%d", columns);
		putenv(buff);
	}

	ret = cavan_redirect_stdio_base(ttyfd, 0x07);
	if (ret < 0)
	{
		pr_error_info("cavan_redirect_stdio_base");
		goto out_close_ttyfd;
	}

	close(ttyfd);

	if (command && command[0] && text_cmp("shell", command))
	{
		return execlp(shell_command, shell_command, "-c", command, NULL);
	}
	else
	{
		return execlp(shell_command, shell_command, "-", NULL);
	}

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int in_fd, int out_fd)
{
	int ret;
	pid_t pid;
	int ptm_fd;
	ssize_t rwlen;
	char buff[1024];
	const char *ptspath;
	const char *ptmpath = "/dev/ptmx";
	struct pollfd pfds[2];

	ptm_fd = open(ptmpath, O_RDWR);
	if (ptm_fd < 0)
	{
		pr_error_info("open %s", ptmpath);
		return ptm_fd;
	}

	ret = fcntl(ptm_fd, F_SETFD, FD_CLOEXEC);
	if (ret < 0)
	{
		pr_error_info("fcntl");
		goto out_close_ptm;
	}

	ret = grantpt(ptm_fd);
	if (ret < 0)
	{
		pr_error_info("grantpt");
		goto out_close_ptm;
	}

	ret = unlockpt(ptm_fd);
	if (ret < 0)
	{
		pr_error_info("unlockpt");
		goto out_close_ptm;
	}

	ptspath = ptsname(ptm_fd);
	if (ptspath == NULL)
	{
		pr_error_info("ptsname");
		ret = -EFAULT;
		goto out_close_ptm;
	}

	pid = fork();
	if (pid < 0)
	{
		pr_error_info("fork");
		ret = pid;
		goto out_close_ptm;
	}

	if (pid == 0)
	{
		close(ptm_fd);

		return cavan_exec_redirect_stdio_base(ptspath, lines, columns, command);
	}
	else
	{
		char oompath[64];

		sprintf(oompath, "/proc/%d/oom_adj", pid);
		file_write(oompath, "0", 1);
	}

	if (in_fd < 0)
	{
		in_fd = fileno(stdin);
	}

	if (out_fd < 0)
	{
		out_fd = fileno(stdout);
	}

	pfds[0].events = POLLIN;
	pfds[0].fd = ptm_fd;

	pfds[1].events = POLLIN;
	pfds[1].fd = in_fd;

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			goto out_close_ptm;
		}

		if (pfds[0].revents)
		{
			rwlen = read(ptm_fd, buff, sizeof(buff));
			if (rwlen <= 0 || write(out_fd, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = read(in_fd, buff, sizeof(buff));
			if (rwlen <= 0 || write(ptm_fd, buff, rwlen) < rwlen)
			{
				break;
			}
		}
	}

	waitpid(pid, &ret, WNOHANG);
	ret = (char)WEXITSTATUS(ret);

out_close_ptm:
	close(ptm_fd);
	return ret;
}

int cavan_tty_redirect_base(int ttyfd)
{
	int ret;
	ssize_t rwlen;
	char buff[1024];
	struct pollfd pfds[2];
	int tty_in, tty_out;
	struct termios tty_attr;

	tty_in = fileno(stdin);
	tty_out = fileno(stdout);

	ret = set_tty_mode(tty_in, 5, &tty_attr);
	if (ret < 0)
	{
		pr_red_info("set_tty_mode");
		return ret;
	}

	pfds[0].events = POLLIN;
	pfds[0].fd = tty_in;

	pfds[1].events = POLLIN;
	pfds[1].fd = ttyfd;

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			goto out_restore_tty_attr;
		}

		if (pfds[0].revents)
		{
			rwlen = read(tty_in, buff, sizeof(buff));
			if (rwlen <= 0 || write(ttyfd, buff, rwlen) < rwlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = read(ttyfd, buff, sizeof(buff));
			if (rwlen <= 0 || write(tty_out, buff, rwlen) < rwlen)
			{
				break;
			}

			fsync(tty_out);
		}
	}

	ret = 0;
out_restore_tty_attr:
	restore_tty_attr(tty_in, &tty_attr);
	return ret;
}

int cavan_tty_redirect(const char *ttypath)
{
	int ret;
	int fd;

	fd = open(ttypath, O_RDWR);
	if (fd < 0)
	{
		pr_error_info("open tty device %s", ttypath);
		return fd;
	}

	ret = cavan_tty_redirect_base(fd);

	close(fd);

	return ret;
}
