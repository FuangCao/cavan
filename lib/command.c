/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/socket.h>
#include <cavan/command.h>

const char *cavan_help_message_help = "display this information";
const char *cavan_help_message_version = "display command version information";
const char *cavan_help_message_ip = "set service ip address to IP";
const char *cavan_help_message_hostname = "set service hostname to HOSTNAME, this option same as --ip";
const char *cavan_help_message_local = "set ip address to 127.0.0.1";
const char *cavan_help_message_adb = "set service protocol to adb";
const char *cavan_help_message_tcp = "set service protocol to tcp";
const char *cavan_help_message_udp = "set service protocol to udp";
const char *cavan_help_message_unix_tcp = "set service protocol to unix tcp";
const char *cavan_help_message_unix_udp = "set service protocol to unix udp";
const char *cavan_help_message_port = "set service port to PORT";
const char *cavan_help_message_url = "set service url to URL";
const char *cavan_help_message_send_file = "send file to server";
const char *cavan_help_message_recv_file = "receive file from server";
const char *cavan_help_message_command = "pass COMMAND to the invoked shell";
const char *cavan_help_message_login = "make the shell a login shell";
const char *cavan_help_message_preserve_environment = "do not reset environment variables, and keep the same shell";
const char *cavan_help_message_shell = "use SHELL instead of the default in passwd";
const char *cavan_help_message_input_file = "input file pathname";
const char *cavan_help_message_output_file = "output file pathname";
const char *cavan_help_message_bs = "block size";
const char *cavan_help_message_seek = "offset of input file";
const char *cavan_help_message_skip = "offset of output file";
const char *cavan_help_message_super = "need super permission";
const char *cavan_help_message_daemon = "run as a daemon";
const char *cavan_help_message_daemon_min = "min daemon count";
const char *cavan_help_message_daemon_max = "max daemon count";
const char *cavan_help_message_verbose = "explain what is being done";
const char *cavan_help_message_logfile = "save log to file";
const char *cavan_help_message_proxy_ip = "set proxy ip address to IP";
const char *cavan_help_message_proxy_hostname = "set proxy hostname to HOSTNAME, this option same as --pip";
const char *cavan_help_message_proxy_port = "set proxy port to PORT";
const char *cavan_help_message_proxy_url = "set proxy url to URL";
const char *cavan_help_message_protocol = "set service protocol to PROTOCOL";
const char *cavan_help_message_proxy_protocol = "set proxy protocol to PROTOCOL";
const char *cavan_help_message_system = "R/W system partition";
const char *cavan_help_message_userdata = "R/W userdata partition";
const char *cavan_help_message_recovery = "R/W recovery partition";
const char *cavan_help_message_misc = "R/W misc partition";
const char *cavan_help_message_boot = "R/W boot partition";
const char *cavan_help_message_kernel = "R/W kernel partition";
const char *cavan_help_message_uboot = "R/W uboot partition";
const char *cavan_help_message_resource = "R/W resource partition";
const char *cavan_help_message_rw_image = "R/W partition by image short name";
const char *cavan_help_message_rw_image_auto = "R/W partition auto";
const char *cavan_help_message_driver = "driver module path";

int print_command_table(const struct cavan_command_map *p, const struct cavan_command_map *p_end)
{
	if (p >= p_end)
	{
		return 0;
	}

	print("Available command is:\n%s", p++->name);

	while (p < p_end)
	{
		print(", %s", p++->name);
	}

	print_char('\n');

	return 0;
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
		print_command_table(p, p_end);
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
		const char *q = pcmd = argv[0];

		while (1)
		{
			switch (*q)
			{
			case '/':
				while (*++q == '/');

				if (*q)
				{
					pcmd = q;
					break;
				}

			case 0:
				goto label_start_match;
			}

			q++;
		}
	}

label_start_match:
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

static int cavan_exec_command(const char *command)
{
	const char *shell_command = "sh";

	if (command && command[0] && text_cmp("shell", command))
	{
		return execlp(shell_command, shell_command, "-c", command, NULL);
	}
	else
	{
		return execlp(shell_command, shell_command, "-", NULL);
	}
}

int cavan_exec_redirect_stdio_base(int ttyfd, const char *command, int flags)
{
	int ret;

	ret = cavan_redirect_stdio_base(ttyfd, flags);
	if (ret < 0)
	{
		pr_error_info("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command);
}

int cavan_exec_redirect_stdio(const char *ttypath, int lines, int columns, const char *command, int flags)
{
	int ret;
	int ttyfd;

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
		ret = tty_set_win_size(ttyfd, lines, columns);
		if (ret < 0)
		{
			pr_red_info("tty_set_win_size");
			return ret;
		}
	}

	ret = cavan_exec_redirect_stdio_base(ttyfd, command, flags);
	close(ttyfd);

	return ret;
}

int cavan_tty_redirect_loop(int ttyfd, int ttyin, int ttyout)
{
	int ret;
	ssize_t rdlen;
	char buff[1024];
	struct pollfd pfds[2];

	pfds[0].events = POLLIN;
	pfds[0].fd = ttyin;

	pfds[1].events = POLLIN;
	pfds[1].fd = ttyfd;

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			return -ETIMEDOUT;
		}

		if (pfds[0].revents)
		{
			rdlen = read(ttyin, buff, sizeof(buff));
			if (rdlen <= 0 || write(ttyfd, buff, rdlen) < rdlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rdlen = read(ttyfd, buff, sizeof(buff));
			if (rdlen <= 0 || write(ttyout, buff, rdlen) < rdlen)
			{
				break;
			}

			fsync(ttyout);
		}
	}

	return 0;
}

int cavan_exec_redirect_stdio_popen(const char *command, int lines, int columns, pid_t *ppid, int flags)
{
	int ret;
	pid_t pid;
	int ttyfd;
	char pathname[32];

	if (lines == 0xFFFF && columns == 0xFFFF)
	{
		int pair[2];

		ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
		if (ret < 0)
		{
			pr_error_info("socketpair");
			return ret;
		}

		pid = fork();
		if (pid < 0)
		{
			pr_error_info("fork");

			close(pair[0]);
			close(pair[1]);

			return pid;
		}

		if (pid == 0)
		{
			close(pair[1]);

			return cavan_exec_redirect_stdio_base(pair[0], command, flags);
		}

		close(pair[0]);
		ttyfd = pair[1];
	}
	else
	{
		const char *ptspath;
		const char *ptmpath = "/dev/ptmx";

		ttyfd = open(ptmpath, O_RDWR);
		if (ttyfd < 0)
		{
			pr_error_info("open %s", ptmpath);
			return ttyfd;
		}

		ret = fcntl(ttyfd, F_SETFD, FD_CLOEXEC);
		if (ret < 0)
		{
			pr_error_info("fcntl");
			goto out_close_ttyfd;
		}

		ret = grantpt(ttyfd);
		if (ret < 0)
		{
			pr_error_info("grantpt");
			goto out_close_ttyfd;
		}

		ret = unlockpt(ttyfd);
		if (ret < 0)
		{
			pr_error_info("unlockpt");
			goto out_close_ttyfd;
		}

		ptspath = ptsname(ttyfd);
		if (ptspath == NULL)
		{
			pr_error_info("ptsname");
			ret = -EFAULT;
			goto out_close_ttyfd;
		}

		pid = fork();
		if (pid < 0)
		{
			pr_error_info("fork");
			ret = pid;
			goto out_close_ttyfd;
		}

		if (pid == 0)
		{
			close(ttyfd);

			return cavan_exec_redirect_stdio(ptspath, lines, columns, command, flags);
		}
	}

	snprintf(pathname, sizeof(pathname), "/proc/%d/oom_adj", pid);
	file_write(pathname, "0", 1);

	if (ppid)
	{
		*ppid = pid;
	}

	return ttyfd;

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int ttyin, int ttyout)
{
	int ret;
	int ttyfd;
	pid_t pid;

	ttyfd = cavan_exec_redirect_stdio_popen(command, lines, columns, &pid, 0x07);
	if (ttyfd < 0)
	{
		pr_red_info("cavan_exec_redirect_stdio_popen");
		return ttyfd;
	}

	if (ttyin < 0)
	{
		ttyin = fileno(stdin);
	}

	if (ttyout < 0)
	{
		ttyout = fileno(stdout);
	}

	ret = cavan_tty_redirect_loop(ttyfd, ttyin, ttyout);
	if (ret < 0)
	{
		pr_red_info("cavan_tty_redirect_loop");
		goto out_close_ttyfd;
	}

	ret = cavan_exec_waitpid(pid);

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_tty_redirect_base(int ttyfd)
{
	int ret;
	struct termios tty_attr;

	ret = set_tty_mode(fileno(stdin), 5, &tty_attr);
	if (ret < 0)
	{
		pr_red_info("set_tty_mode");
		return ret;
	}

	ret = cavan_tty_redirect_loop(ttyfd, fileno(stdin), fileno(stdout));
	restore_tty_attr(fileno(stdin), &tty_attr);

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

const char *cavan_getenv(const char *name, const char *default_value)
{
	const char *value;

	value = getenv(name);
	if (value)
	{
		return value;
	}

	return default_value;
}

u32 cavan_getenv_u32(const char *name, u32 default_value)
{
	const char *value;

	value = cavan_getenv(name, NULL);
	if (value == NULL)
	{
		return default_value;
	}

	return text2value_unsigned(value, NULL, 10);
}

int tty_get_win_size(int tty, u16 *lines, u16 *columns)
{
	int ret;
	struct winsize wsize;

	ret = ioctl(tty, TIOCGWINSZ, &wsize);
	if (ret < 0)
	{
		// pr_error_info("ioctl TIOCGWINSZ");
		return ret;
	}

	if (lines)
	{
		*lines = wsize.ws_row;
	}

	if (columns)
	{
		*columns = wsize.ws_col;
	}

	return 0;
}

int tty_set_win_size(int tty, u16 lines, u16 columns)
{
	int ret;
	char buff[64];
	struct winsize wsize =
	{
		.ws_row = lines,
		.ws_col = columns,
		.ws_xpixel = 0,
		.ws_ypixel = 0
	};

	ret = ioctl(tty, TIOCSWINSZ, &wsize);
	if (ret < 0)
	{
		pr_error_info("ioctl TIOCSWINSZ");
		return ret;
	}

	sprintf(buff, "LINES=%d", lines);
	putenv(buff);
	sprintf(buff, "COLUMNS=%d", columns);
	putenv(buff);

	return 0;
}

int cavan_system(const char *command)
{
	pid_t pid = fork();

	return pid == 0 ? cavan_exec_command(command) : cavan_exec_waitpid(pid);
}

int cavan_system2(const char *command, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, command);
	vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

	return cavan_system(buff);
}
