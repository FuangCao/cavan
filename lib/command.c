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
	char buff[MB(1)], *buff_p;

	if (size == 0)
	{
		return;
	}

	pr_bold_info("Available command is:");

	for (p_end = p + size, buff_p = buff; p < p_end; p++)
	{
		buff_p += sprintf(buff_p, "%s, ", p->name);
	}

	*(buff_p - 2) = 0;

	print_string(buff);
}

const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size)
{
	while (p < p_end && text_ncmp(cmdname, p->name, size))
	{
		p++;
	}

	return p < p_end ? p : NULL;
}

void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	int size;
	const struct cavan_command_map *p_match;
	char buff[MB(1)], *buff_p;

	for (size = text_len(cmdname); size && ((p_match = find_command_by_name(p, p_end, cmdname, size))) == NULL; size--);

	if (size == 0)
	{
		pr_red_info("`%s' No such command", cmdname);
		return;
	}

	pr_bold_info("This command maybe:");

	buff_p = buff;

	do {
		buff_p += sprintf(buff_p, "%s, ", p_match->name);
	} while ((p_match = find_command_by_name(p_match + 1, p_end, cmdname, size)));

	*(buff_p - 2) = 0;

	print_string(buff);
}

const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	while (p < p_end && text_cmp(cmdname, p->name))
	{
		p++;
	}

	return p < p_end ? p : NULL;
}

int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[])
{
	const struct cavan_command_map *p;
	const char *pcmd;

	if (file_test(argv[0], "l") < 0)
	{
		if (argc < 2)
		{
			print_command_table(map, count);
			return -1;
		}

		pcmd = argv[1];
		argc--;
		argv++;
	}
	else
	{
		const char *pstart;

		for (pcmd = argv[0]; *pcmd; pcmd++);
		for (pstart = argv[0]; pcmd >= pstart && *pcmd != '/'; pcmd--);
		pcmd++;
	}

	p = match_command_by_name(map, map + count, pcmd);
	if (p)
	{
		return p->main_func(argc, argv);
	}

	print_maybe_command(map, map + count, pcmd);

	return -1;
}

int cavan_exec_redirect_stdio1(int ttyfd, const char *command, const char *args)
{
	int ret;

	ret = setsid();
	if (ret < 0)
	{
		pr_error_info("setsid");
		return ret;
	}

	ret = dup2(ttyfd, fileno(stdin));
	if (ret < 0)
	{
		pr_error_info("dup2 stdin");
		return ret;
	}

	ret = dup2(ttyfd, fileno(stdout));
	if (ret < 0)
	{
		pr_error_info("dup2 stdout");
		return ret;
	}

	ret = dup2(ttyfd, fileno(stderr));
	if (ret < 0)
	{
		pr_error_info("dup2 stderr");
		return ret;
	}

	return execl(command, command, args, NULL);
}

int cavan_exec_redirect_stdio2(const char *ttypath, const char *command, const char *args)
{
	int ret;
	int ttyfd;

	println("ttypath = %s", ttypath);

	ttyfd = open(ttypath, O_RDWR);
	if (ttyfd < 0)
	{
		pr_error_info("open file %s", ttypath);
		return ttyfd;
	}

	ret = cavan_exec_redirect_stdio1(ttyfd, command, args);
	if (ret < 0)
	{
		pr_red_info("cavan_exec_redirect_stdio1 %s %s", command, args);
	}

	close(ttyfd);

	return ret;
}

int cavan_exec_redirect_stdio_main(const char *command, const char *args, int in_fd, int out_fd)
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

	println("ptspath = %s", ptspath);

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

		return cavan_exec_redirect_stdio2(ptspath, command, args);
	}
	else
	{
		char oompath[64];

		sprintf(oompath, "/proc/%d/oom_adj", pid);
		println("oompath = %s", oompath);
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
		if (ret < 0)
		{
			pr_error_info("poll");
			goto out_close_ptm;
		}

		if (pfds[0].revents)
		{
			rwlen = read(ptm_fd, buff, sizeof(buff));
			if (rwlen <= 0 || write(out_fd, buff, rwlen) < 0)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rwlen = read(in_fd, buff, sizeof(buff));
			if (rwlen <= 0 || write(ptm_fd, buff, rwlen) < 0)
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
