/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/socket.h>
#include <cavan/device.h>
#include <cavan/command.h>

#define CAVAN_COMMAND_DEBUG				0
#define CAVAN_TEE_USE_SYSTEM_POPEN		0

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

static const char *const cavan_exec_tty_prefix[3] = { "stdin", "stdout", "stderr" };
static const int cavan_exec_tty_flags[3] = { O_RDONLY, O_WRONLY, O_WRONLY };
static const int cavan_exec_tty_flags_client[3] = { O_WRONLY, O_RDONLY, O_RDONLY };
static const char *const cavan_exec_shell_list[] =
{
	"/bin/bash",
	"/bin/sh",
	"/system/bin/sh",
};

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

int cavan_redirect_stdio_base(int ttyfds[3])
{
	int i;
	int ret;

	for (i = 0; i < 3; i++)
	{
		if (ttyfds[i] < 0)
		{
			continue;
		}

		ret = dup2(ttyfds[i], i);
		if (ret < 0)
		{
			pr_error_info("dup2 stdio %d", i);
			return ret;
		}
	}

	for (i = 0; i < 3; i++)
	{
		int j;

		if (ttyfds[i] < 0)
		{
			continue;
		}

		j = i;

		do {
			if (--j < 0)
			{
				close(ttyfds[i]);
				break;
			}
		} while (ttyfds[i] != ttyfds[j]);
	}

	return 0;
}

int cavan_redirect_stdio_base2(int fd, int flags)
{
	int i;
	int ttyfds[3];

	for (i = 0; i < 3; i++)
	{
		if (flags & (1 << i))
		{
			ttyfds[i] = fd;
		}
		else
		{
			ttyfds[i] = -1;
		}
	}

	return cavan_redirect_stdio_base(ttyfds);
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

	ret = cavan_redirect_stdio_base2(fd, flags);

	close(fd);

	return ret;
}

static const char *cavan_get_shell_path(void)
{
	int i;

	for (i = 0; i < NELEM(cavan_exec_shell_list); i++)
	{
		if (file_access_e(cavan_exec_shell_list[i]))
		{
			return cavan_exec_shell_list[i];
		}
	}

	return "sh";
}

static int cavan_exec_command(const char *command)
{
	const char *shell = cavan_get_shell_path();
	const char *name = text_basename_simple(shell);

#if CAVAN_COMMAND_DEBUG
	println("shell = %s, name = %s, command = %s", shell, name, command);
#endif

	if (command && command[0] && text_cmp("shell", command))
	{
		return execlp(shell, name, "-c", command, NULL);
	}
	else
	{
		return execlp(shell, name, "-", NULL);
	}
}

int cavan_exec_redirect_stdio_base(int ttyfds[3], const char *command)
{
	int ret;

	ret = cavan_redirect_stdio_base(ttyfds);
	if (ret < 0)
	{
		pr_error_info("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command);
}

int cavan_exec_redirect_stdio_base2(int ttyfd, const char *command, int flags)
{
	int ret;

	ret = cavan_redirect_stdio_base2(ttyfd, flags);
	if (ret < 0)
	{
		pr_error_info("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command);
}

int cavan_exec_redirect_stdio(char *const ttypath[3], const char *command, int flags)
{
	int ret;
	int ttyfds[3];

	ret = cavan_exec_open_temp_pipe(ttyfds, ttypath, flags);
	if (ret < 0)
	{
		pr_red_info("cavan_exec_open_temp_pipe: %d", ret);
		return ret;
	}

	return cavan_exec_redirect_stdio_base(ttyfds, command);
}

int cavan_exec_redirect_stdio2(const char *ttypath, int lines, int columns, const char *command, int flags)
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

	return cavan_exec_redirect_stdio_base2(ttyfd, command, flags);
}

int cavan_tty_redirect_loop(int tty1[2], int tty2[2])
{
	int ret;
	ssize_t rdlen;
	char buff[1024];
	struct pollfd pfds[2];

	pfds[0].events = POLLIN;
	pfds[0].fd = tty1[0];

	pfds[1].events = POLLIN;
	pfds[1].fd = tty2[0];

	while (1)
	{
		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0)
		{
			return -ETIMEDOUT;
		}

		if (pfds[0].revents)
		{
			rdlen = read(tty1[0], buff, sizeof(buff));
			if (rdlen <= 0 || write(tty2[1], buff, rdlen) < rdlen)
			{
				break;
			}
		}

		if (pfds[1].revents)
		{
			rdlen = read(tty2[0], buff, sizeof(buff));
			if (rdlen <= 0 || write(tty1[1], buff, rdlen) < rdlen)
			{
				break;
			}
		}
	}

	return 0;
}

int cavan_exec_set_oom_adj(int pid, int value)
{
	char buff[32], *p;
	char pathname[1024];

	snprintf(pathname, sizeof(pathname), "/proc/%d/oom_adj", pid);
	p = value2text_simple(value, buff, sizeof(buff), 10);

	return file_write(pathname, buff, p - buff);
}

int cavan_exec_redirect_stdio_popen(const char *command, int lines, int columns, pid_t *ppid, int flags)
{
	int ret;
	pid_t pid;
	int ttyfd;
	char pathname[1024];

	if (lines < 0 || columns < 0)
	{
		int pair[2];

		if ((flags & 0x01) == 0 || (flags & 0x06) == 0)
		{
			int pipefd[2];

#if CAVAN_COMMAND_DEBUG
			println("create pipe");
#endif

			ret = pipe(pipefd);
			if (ret < 0)
			{
				pr_err_info("pipe");
				return ret;
			}

			if (flags & 0x01)
			{
				pair[0] = pipefd[0];
				pair[1] = pipefd[1];
			}
			else
			{
				pair[0] = pipefd[1];
				pair[1] = pipefd[0];
			}
		}
		else
		{
#if CAVAN_COMMAND_DEBUG
			println("create socketpair");
#endif

			ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
			if (ret < 0)
			{
				pr_error_info("socketpair");
				return ret;
			}
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

			return cavan_exec_redirect_stdio_base2(pair[0], command, flags);
		}

		close(pair[0]);
		ttyfd = pair[1];
	}
	else
	{
		const char *ptmpath = "/dev/ptmx";

#if CAVAN_COMMAND_DEBUG
		println("create pseudo terminal");
#endif

		ttyfd = open(ptmpath, O_RDWR | O_CLOEXEC);
		if (ttyfd < 0)
		{
			pr_error_info("open %s", ptmpath);
			return ttyfd;
		}

#if 0
		ret = fcntl(ttyfd, F_SETFD, FD_CLOEXEC);
		if (ret < 0)
		{
			pr_error_info("fcntl");
			goto out_close_ttyfd;
		}
#endif

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

		ret = ptsname_r(ttyfd, pathname, sizeof(pathname));
		if (ret < 0)
		{
			pr_error_info("ptsname");
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

			return cavan_exec_redirect_stdio2(pathname, lines, columns, command, flags);
		}
	}

	cavan_exec_set_oom_adj(pid, 0);

	if (ppid)
	{
		*ppid = pid;
	}

	return ttyfd;

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_exec_get_temp_pipe_pathname(char *pathname, size_t size, pid_t pid, int type)
{
	return snprintf(pathname, size, "/dev/%s-%d", cavan_exec_tty_prefix[type], pid);
}

int cavan_exec_make_temp_pipe(char *pathname, size_t size, pid_t pid, int type)
{
	int ret;

	cavan_exec_get_temp_pipe_pathname(pathname, size, pid, type);

	ret = mkfifo(pathname, 0777 | S_IFIFO);
	if (ret < 0 && errno != EEXIST)
	{
		pr_err_info("mkfifo %s", pathname);
		return ret;
	}

	return 0;
}

int cavan_exec_make_temp_pipe2(pid_t pid, int flags)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		int ret;
		char pathname[1024];

		if ((flags & (1 << i)) == 0)
		{
			continue;
		}

		ret = cavan_exec_make_temp_pipe(pathname, sizeof(pathname), pid, i);
		if (ret < 0)
		{
			pr_red_info("cavan_create_temp_pipe %s", pathname);

			while (--i >= 0)
			{
				if ((flags & (1 << i)) == 0)
				{
					continue;
				}

				cavan_exec_get_temp_pipe_pathname(pathname, sizeof(pathname), pid, i);
				unlink(pathname);
			}

			return ret;
		}
	}

	return 0;
}

void cavan_exec_unlink_temp_pipe(char *ttypath[3])
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (ttypath[i])
		{
			unlink(ttypath[i]);
		}
	}
}

int cavan_exec_open_temp_pipe(int ttyfds[3], char *const ttypath[3], int flags)
{
	int i;
	pid_t pid = getpid();

	for (i = 0; i < 3; i++)
	{
		char buff[1024];
		const char *pathname;

		if (ttypath)
		{
			pathname = ttypath[i];
		}
		else if (flags & (1 << i))
		{
			cavan_exec_make_temp_pipe(buff, sizeof(buff), pid, 1);
			pathname = buff;
		}
		else
		{
			pathname = NULL;
		}

		if (pathname)
		{
			int fd = open(pathname, cavan_exec_tty_flags[i]);
			if (fd < 0)
			{
				pr_red_info("cavan_open_pipe_once %s", pathname);

				if (flags & CAVAN_EXECF_DEL_TTY)
				{
					int j = i + 1;

					unlink(pathname);

					if (ttypath)
					{
						while (j < 3)
						{
							if (ttypath[j])
							{
								unlink(ttypath[j]);
							}

							j++;
						}
					}
					else
					{
						while (j < 3)
						{
							if (flags & (1 << i))
							{
								cavan_exec_get_temp_pipe_pathname(buff, sizeof(buff), pid, j);
								unlink(buff);
							}

							j++;
						}
					}
				}

				while (--i >= 0)
				{
					if (ttyfds[i] >= 0)
					{
						close(ttyfds[i]);
					}
				}

				return fd;
			}

			ttyfds[i] = fd;

			if (flags & CAVAN_EXECF_DEL_TTY)
			{
				unlink(pathname);
			}
		}
		else
		{
			ttyfds[i] = -1;
		}
	}

	if (flags & (CAVAN_EXECF_ERR_TO_OUT) && ttyfds[2] < 0)
	{
		ttyfds[2] = ttyfds[1];
	}

	return 0;
}

int cavan_exec_open_temp_pipe_client(int ttyfds[3], pid_t pid, int flags)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		int fd;
		int ret;
		char pathname[1024];

		if ((flags & (1 << i)) == 0)
		{
			continue;
		}

		ret = cavan_exec_get_temp_pipe_pathname(pathname, sizeof(pathname), pid, i);
		if (ret < 0)
		{
			pr_red_info("cavan_exec_get_temp_pipe_pathname: %d", ret);
			return ret;
		}

		fd = open(pathname, cavan_exec_tty_flags_client[i]);
		if (fd < 0)
		{
			pr_err_info("open %s", pathname);

			while (--i >= 0)
			{
				close(ttyfds[i]);
			}

			return fd;
		}

		ttyfds[i] = fd;
	}

	return 0;
}

int cavan_exec_redirect_stdio_popen2(const char *command, pid_t *ppid, int flags)
{
	int ret;
	pid_t pid;

	pid = fork();
	if (pid < 0)
	{
		pr_err_info("fork");
		return pid;
	}

	if (pid > 0)
	{
		ret = cavan_exec_make_temp_pipe2(pid, flags);
		if (ret < 0)
		{
			pr_red_info("cavan_exec_make_temp_pipe2: %d", ret);
			return ret;
		}
	}
	else
	{
		return cavan_exec_redirect_stdio(NULL, command, CAVAN_EXECF_DEL_TTY | CAVAN_EXECF_ERR_TO_OUT | flags);
	}

	cavan_exec_set_oom_adj(pid, 0);

	if (ppid)
	{
		*ppid = pid;
	}

	return 0;
}

int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int ttyin, int ttyout)
{
	int ret;
	int ttyfd;
	pid_t pid;
	int ttyfds[2];
	int stdfds[2];

	ttyfd = cavan_exec_redirect_stdio_popen(command, lines, columns, &pid, 0x07);
	if (ttyfd < 0)
	{
		pr_red_info("cavan_exec_redirect_stdio_popen");
		return ttyfd;
	}

	ttyfds[0] = ttyfds[1] = ttyfd;
	stdfds[0] = ttyin < 0 ? fileno(stdin) : ttyin;
	stdfds[1] = ttyout < 0 ? fileno(stdout) : ttyout;

	ret = cavan_tty_redirect_loop(ttyfds, stdfds);
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
	int ttyfds[] = { ttyfd, ttyfd };
	int stdfds[] = { fileno(stdin), fileno(stdout) };

	ret = set_tty_mode(fileno(stdin), 5, &tty_attr);
	if (ret < 0)
	{
		pr_red_info("set_tty_mode");
		return ret;
	}

	ret = cavan_tty_redirect_loop(ttyfds, stdfds);
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

int cavan_tee_main(const char *filename, bool append, bool command)
{
#if CAVAN_TEE_USE_SYSTEM_POPEN
	FILE *fp;
#else
	int fd;
#endif
	int ret = 0;
	size_t total = 0;

	if (filename == NULL)
	{
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = stderr;
#else
		fd = 2;
#endif
	}
	else if (strcmp(filename, "-") == 0)
	{
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = stdout;
#else
		fd = 1;
#endif
	}
	else if (command)
	{
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = popen(filename, "w");
		if (fp == NULL)
		{
			pr_err_info("popen %s", filename);
			return -EFAULT;
		}

		setvbuf(fp, NULL, _IONBF, 0);
#else
		fd = cavan_exec_redirect_stdio_popen(filename, -1, -1, NULL, 0x01);
		if (fd < 0)
		{
			pr_red_info("cavan_exec_redirect_stdio_popen: %d", fd);
			return fd;
		}
#endif
	}
	else
	{
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = fopen(filename, append ? "a+" : "w+");
		if (fp == NULL)
		{
			pr_err_info("fopen %s", filename);
			return -EFAULT;
		}
#else
		fd = open(filename, (append ? O_APPEND : O_TRUNC) | O_WRONLY | O_CREAT, 0777);
		if (fd < 0)
		{
			pr_err_info("open %s", filename);
			return fd;
		}
#endif
	}

	while (1)
	{
		ssize_t rdlen;
		ssize_t wrlen;
		char buff[1024];

		rdlen = read(0, buff, sizeof(buff));
		if (rdlen <= 0)
		{
			break;
		}

#if CAVAN_TEE_USE_SYSTEM_POPEN
		wrlen = fwrite(buff, rdlen, 1, fp) | fwrite(buff, rdlen, 1, stdout);
#else
		wrlen = write(1, buff, rdlen) | write(fd, buff, rdlen);
#endif
		if (wrlen != rdlen)
		{
			// pr_err_info("write");
			break;
		}

		total += rdlen;
	}

#if CAVAN_TEE_USE_SYSTEM_POPEN
	if (fp != stderr && fp != stdout)
	{
		if (command)
		{
			pclose(fp);
		}
		else
		{
			fclose(fp);
		}
	}
#else
	if (fd != 1 && fd != 2)
	{
		close(fd);
	}
#endif

	if (ret < 0)
	{
		return ret;
	}

	if (total > 0)
	{
		return 0;
	}

	return -ENODATA;
}
