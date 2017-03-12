/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <cavan/timer.h>
#include <cavan/ctype.h>
#include <cavan/thread.h>
#include <cavan/device.h>
#include <cavan/command.h>
#include <cavan/android.h>
#include <sys/socket.h>
#include <sys/reboot.h>
#include <sys/klog.h>
#include <linux/reboot.h>

#ifndef SYS_reboot
#define SYS_reboot __NR_reboot
#endif

#define CAVAN_COMMAND_DEBUG				0
#define CAVAN_TEE_USE_SYSTEM_POPEN		0

static struct cavan_async_command_service s_async_command_service_default = {
	.cond = PTHREAD_COND_INITIALIZER,
	.lock = PTHREAD_MUTEX_INITIALIZER,
};

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
const char *cavan_help_message_count = "block count";
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
const char *cavan_help_message_service = "run as service";
const char *cavan_help_message_server = "run as server";
const char *cavan_help_message_addr_bytes = "register address bytes";
const char *cavan_help_message_value_bytes = "register value bytes";
const char *cavan_help_message_big_endian = "register big-endian";
const char *cavan_help_message_little_endian = "register little-endian";
const char *cavan_help_message_cmdline = "run as cmdline";
const char *cavan_help_message_user = "set user";
const char *cavan_help_message_group = "set group";
const char *cavan_help_message_discovery = "enable discovery service and set port to PORT";
const char *cavan_help_message_discovery_delay = "discovery delay msec";

static const char *const cavan_exec_tty_prefix[3] = { "stdin", "stdout", "stderr" };
static const int cavan_exec_tty_master_open_flags[3] = { O_RDONLY, O_WRONLY, O_WRONLY };
static const int cavan_exec_tty_slave_open_flags[3] = { O_WRONLY, O_RDONLY, O_RDONLY };
static const char *const cavan_exec_shell_list[] = {
	"/bin/bash",
	"/bin/sh",
	"/system/bin/sh",
};

int print_command_table(const struct cavan_command_entry *p, const struct cavan_command_entry *p_end)
{
	if (p >= p_end) {
		return 0;
	}

	print("Available command is:\n%s", p++->name);

	while (p < p_end) {
		print(", %s", p++->name);
	}

	print_char('\n');

	return 0;
}

const struct cavan_command_entry *find_command_by_name(const struct cavan_command_entry *p, const struct cavan_command_entry *p_end, const char *cmdname, size_t size)
{
	while (p < p_end) {
		if (text_ncmp(cmdname, p->name, size) == 0) {
			return p;
		}

		p++;
	}

	return NULL;
}

void print_maybe_command(const struct cavan_command_entry *p, const struct cavan_command_entry *p_end, const char *cmdname)
{
	int size;
	const struct cavan_command_entry *q;

	for (size = text_len(cmdname); size && ((q = find_command_by_name(p, p_end, cmdname, size))) == NULL; size--);

	if (size) {
		int i;
		int count;
		const struct cavan_command_entry *matchs[100];

		pr_bold_info("This command maybe:");

		for (count = 0, p = q; count < NELEM(matchs); count++) {
			matchs[count] = p;

			p = find_command_by_name(p + 1, p_end, cmdname, size);
			if (p == NULL) {
				break;
			}
		}

		for (i = 0; i < count; i++) {
			print("%s, ", matchs[i]->name);
		}

		print_string(matchs[i]->name);
	} else {
		print_command_table(p, p_end);
	}
}

const struct cavan_command_entry *match_command_by_name(const struct cavan_command_entry *p, const struct cavan_command_entry *p_end, const char *cmdname)
{
	while (p < p_end) {
		if (text_cmp(cmdname, p->name) == 0) {
			return p;
		}

		p++;
	}

	return NULL;
}

int find_and_exec_command(const struct cavan_command_entry *map, size_t count, int argc, char *argv[])
{
	const struct cavan_command_entry *p;
	const char *pcmd;

	if (argc > 1 && argv[1][0] == '@') {
		pcmd = argv[1] + 1;
		argc--;
		argv++;
	} else {
		const char *q = pcmd = argv[0];

		while (1) {
			switch (*q) {
			case '/':
				while (*++q == '/');

				if (*q) {
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
	if (p == NULL && argc > 1 && file_test(argv[0], "l") < 0) {
		pcmd = argv[1];
		argc--;
		argv++;

		p = match_command_by_name(map, map + count, pcmd);
	}

	if (p) {
		return p->handler(argc, argv);
	}

	print_maybe_command(map, map + count, pcmd);

	return -1;
}

pid_t cavan_exec_fork(void)
{
	cavan_stdio_fflush();

	return fork();
}

int cavan_exec_waitpid(pid_t pid)
{
	int status;

#if 0
	while (1) {
		if (waitpid(pid, &status, WUNTRACED | WCONTINUED) < 0) {
			return 0;
		}

		if (WIFEXITED(status)) {
			break;
		}

		if (WIFSIGNALED(status)) {
			pr_info("killed by signal %d", WTERMSIG(status));
			return -EFAULT;
		}

		if (WIFSTOPPED(status)) {
			pr_info("stopped by signal %d", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			pr_info("continued");
		}
	}
#else
	if (waitpid(pid, &status, 0) != pid) {
		return 0;
	}
#endif

	return WEXITSTATUS(status);
}

static int cavan_builtin_command_shell(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
#ifdef CONFIG_ANDROID
	const char *username;
	const char *hostname;

	hostname = getenv("HOSTNAME");
	if (hostname == NULL) {
		hostname = "ANDROID";
	}

	username = getenv("USER");
	if (username == NULL) {
		username = "cavan";
	}

	CAVAN_TTY_SET_TITLE("%s@%s", username, hostname);
#endif

	return execlp(shell, desc->name, "-", NULL);
}

static int cavan_builtin_command_reboot_base(const struct cavan_builtin_command *desc, int argc, char *argv[], bool shutdown)
{
	if ((desc->flags & CAVAN_BUILTIN_CMDF_FORCE) == 0 && cavan_is_android() == false) {
		if (!cavan_get_choose_yesno_format(false, 5000, "Do you want to exec command: %s", argv[0])) {
			return -EPERM;
		}
	}

	println("command = %s", argv[1]);

	return cavan_reboot(shutdown, argv[1]);
}

static int cavan_builtin_command_reboot(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
	return cavan_builtin_command_reboot_base(desc, argc, argv, false);
}

static int cavan_builtin_command_shutdown(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
	return cavan_builtin_command_reboot_base(desc, argc, argv, true);
}

static int cavan_builtin_command_remount(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
	int ret = 0;

	if (cavan_is_android()) {
		if (argc > 1) {
			int i;

			for (i = 1; i < argc; i++) {
				ret |= bdev_remount(argv[i], NULL);
			}
		} else {
			ret |= bdev_remount("/", NULL);
			ret |= bdev_remount("/system", NULL);
		}

		if (ret < 0) {
			pr_red_info("Failed!");
		} else {
			pr_green_info("OK");
		}
	}

	return ret;
}

static int cavan_builtin_command_kmsg(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
#if 1
	int fd;
	const char *pathname = "/proc/kmsg";

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pr_err_info("open %s: %d", pathname, fd);
		return fd;
	}

	while (1) {
		ssize_t rdlen;
		char buff[1024];

		rdlen = ffile_read(fd, buff, sizeof(buff));
		if (rdlen <= 0) {
			break;
		}

		if (ffile_write(stdout_fd, buff, rdlen) != rdlen) {
			break;
		}
	}
#else
	while (1) {
		int length;
		char buff[1024];

		length = klogctl(SYSLOG_ACTION_READ, buff, sizeof(buff));
		if (length < 0) {
			break;
		}

		if (ffile_write(stdout_fd, buff, length) != length) {
			break;
		}
	}
#endif

	return 0;
}

static int cavan_builtin_command_logcat(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[])
{
	int i;
	char *argv_new[argc + 3];

	argv_new[0] = argv[0];
	argv_new[1] = "-v";
	argv_new[2] = "threadtime";

	for (i = 1; i <= argc; i++) {
		argv_new[i + 2] = argv[i];
	}

	return execvp("logcat", argv_new);
}

static const struct cavan_builtin_command cavan_builtin_command_list[] = {
	{ "shell", cavan_builtin_command_shell, 0 },
	{ "reboot", cavan_builtin_command_reboot, 0 },
	{ "reboot-force", cavan_builtin_command_reboot, CAVAN_BUILTIN_CMDF_FORCE },
	{ "halt", cavan_builtin_command_shutdown, 0 },
	{ "shutdown", cavan_builtin_command_shutdown, 0 },
	{ "halt-force", cavan_builtin_command_shutdown, CAVAN_BUILTIN_CMDF_FORCE },
	{ "shutdown-force", cavan_builtin_command_shutdown, CAVAN_BUILTIN_CMDF_FORCE },
	{ "remount", cavan_builtin_command_remount, 0 },
	{ "kmsg", cavan_builtin_command_kmsg, 0 },
	{ "logcat", cavan_builtin_command_logcat, 0 },
};

static const struct cavan_builtin_command *cavan_find_builtin_command(const char *command)
{
	int length;
	const char *p;
	const struct cavan_builtin_command *desc, *desc_end;

	for (p = command; cavan_notspace_zero(*p); p++);

	length = p - command;

	for (desc = cavan_builtin_command_list, desc_end = desc+ NELEM(cavan_builtin_command_list); desc < desc_end; desc++) {
		if (strncmp(command, desc->name, length) == 0) {
			return desc;
		}
	}

	return NULL;
}

static const char *cavan_get_shell_path(void)
{
	int i;

	for (i = 0; i < NELEM(cavan_exec_shell_list); i++) {
		if (file_access_e(cavan_exec_shell_list[i])) {
			return cavan_exec_shell_list[i];
		}
	}

	return "sh";
}

int cavan_exec_command(const char *command, int argc, char *argv[])
{
	int ret;
	const struct cavan_builtin_command *desc;
	const char *shell = cavan_get_shell_path();
	const char *name = cavan_path_basename_simple(shell);

	cavan_exec_set_oom_adj2(0, 0);

	if (command == NULL || command[0] == 0) {
		if (argv && argc > 0 && argv[0]) {
			command = argv[0];
		} else {
			command = "shell";
		}
	}

#if CAVAN_COMMAND_DEBUG
	pr_func_info("shell = %s, name = %s, command = `%s'", shell, name, command);
#endif

	desc = cavan_find_builtin_command(command);
	if (desc) {
		char *command_rw;
		char *argv_new[64];

		if (argv == NULL) {
			char *p, *p_end;

			command_rw = strdup(command);
			if (command_rw == NULL) {
				pr_err_info("strdup");
				return -ENOMEM;
			}

			argc = 0;
			argv = argv_new;

			p = command_rw;
			p_end = p + strlen(p);

			while (argc < NELEM(argv_new) - 1) {
				p = text_skip_space(p, p_end);
				if (p == NULL || p >= p_end) {
					break;
				}

				argv[argc++] = p;

				p = text_find_space(p, p_end);
				if (p == NULL) {
					break;
				}

				*p++ = 0;
			}

			argv[argc] = NULL;
		} else {
			command_rw = NULL;
		}

#if CAVAN_COMMAND_DEBUG
		pr_func_info("builtin_command = %s", desc->name);
#endif

		ret = desc->handler(desc, shell, argc, argv);

		if (command_rw) {
			free(command_rw);
		}
	} else if (argv) {
		ret = execvp(command, argv);
	} else {
		ret = execlp(shell, name, "-c", command, NULL);
	}

	if (ret < 0) {
		exit(errno ? errno : 1);
	}

	exit(0);
}

int cavan_exec_redirect_stdio_base(int ttyfds[3], const char *command)
{
	int ret;

	ret = cavan_stdio_redirect1(ttyfds);
	if (ret < 0) {
		pr_error_info("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command, 0, NULL);
}

int cavan_exec_redirect_stdio_base2(int ttyfd, const char *command, int flags)
{
	int ret;

	ret = cavan_stdio_redirect2(ttyfd, flags);
	if (ret < 0) {
		pr_error_info("cavan_redirect_stdio_base");
		return ret;
	}

	return cavan_exec_command(command, 0, NULL);
}

int cavan_exec_redirect_stdio(char *const ttypath[3], const char *command, int flags)
{
	int ret;
	int ttyfds[3];

	ret = cavan_exec_open_temp_pipe_master(ttyfds, ttypath, getpid(), flags);
	if (ret < 0) {
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
	if (ret < 0) {
		pr_error_info("setsid");
		// return ret;
	}

	ttyfd = open(ttypath, O_RDWR | O_CLOEXEC);
	if (ttyfd < 0) {
		pr_error_info("open file %s", ttypath);
		return ttyfd;
	}

	if (isatty(ttyfd) && lines > 0 && columns > 0) {
		ret = tty_set_win_size(ttyfd, lines, columns);
		if (ret < 0) {
			pr_red_info("tty_set_win_size");
			return ret;
		}
	}

	setenv("CAVAN_TTY", ttypath, 1);

	ret = cavan_exec_redirect_stdio_base2(ttyfd, command, flags);

	close(ttyfd);

	return ret;
}

int cavan_exec_set_oom_adj(const char *dirname, int value)
{
	ssize_t wrlen;
	size_t length;
	char buff[32];
	char *filename;
	char pathname[1024];

	length = value2text_simple(value, buff, sizeof(buff), 10) - buff;

	filename = cavan_path_copy(pathname, sizeof(pathname), dirname, true);
	strcpy(filename, "oom_score_adj");

	wrlen = file_write(pathname, buff, length);
	if (wrlen > 0) {
		return 0;
	}

	strcpy(filename, "oom_adj");

	return file_write(pathname, buff, length);
}

int cavan_exec_set_oom_adj2(int pid, int value)
{
	char buff[64];
	const char *dirname;

	if (pid == 0) {
		dirname = "/proc/self";
	} else {
		snprintf(buff, sizeof(buff), "/proc/%d", pid);
		dirname = buff;
	}

	return cavan_exec_set_oom_adj(dirname, value);
}

int cavan_exec_redirect_stdio_popen(const char *command, int lines, int columns, pid_t *ppid, int flags)
{
	int ret;
	pid_t pid;
	int ttyfd;
	char pathname[1024];

#if CAVAN_COMMAND_DEBUG
	pr_func_info("command = `%s', lines = %d, columns = %d, flags = 0x%08x", command, lines, columns, flags);
#endif

	if ((flags & CAVAN_EXECF_ERR_TO_OUT) && (flags & CAVAN_EXECF_STDOUT)) {
		flags |= CAVAN_EXECF_STDERR;
	}

	if (lines < 0 || columns < 0) {
		int pair[2];

		if ((flags & 0x01) == 0 || (flags & 0x06) == 0) {
			int pipefd[2];

#if CAVAN_COMMAND_DEBUG
			pr_func_info("create pipe");
#endif

			ret = pipe(pipefd);
			if (ret < 0) {
				pr_err_info("pipe");
				return ret;
			}

			if (flags & 0x01) {
				pair[0] = pipefd[0];
				pair[1] = pipefd[1];
			} else {
				pair[0] = pipefd[1];
				pair[1] = pipefd[0];
			}
		} else {
#if CAVAN_COMMAND_DEBUG
			pr_func_info("create socketpair");
#endif

			ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
			if (ret < 0) {
				pr_error_info("socketpair");
				return ret;
			}
		}

		pid = cavan_exec_fork();
		if (pid < 0) {
			pr_error_info("cavan_exec_fork");

			close(pair[0]);
			close(pair[1]);

			return pid;
		}

		if (pid == 0) {
			close(pair[1]);

			return cavan_exec_redirect_stdio_base2(pair[0], command, flags);
		}

		close(pair[0]);
		ttyfd = pair[1];
	} else {
		const char *ptmpath = "/dev/ptmx";

#if CAVAN_COMMAND_DEBUG
		pr_func_info("create pseudo terminal");
#endif

		ttyfd = open(ptmpath, O_RDWR | O_CLOEXEC);
		if (ttyfd < 0) {
			pr_error_info("open %s", ptmpath);
			return ttyfd;
		}

#if 0
		ret = fcntl(ttyfd, F_SETFD, FD_CLOEXEC);
		if (ret < 0) {
			pr_error_info("fcntl");
			goto out_close_ttyfd;
		}
#endif

		ret = grantpt(ttyfd);
		if (ret < 0) {
			pr_error_info("grantpt");
			goto out_close_ttyfd;
		}

		ret = unlockpt(ttyfd);
		if (ret < 0) {
			pr_error_info("unlockpt");
			goto out_close_ttyfd;
		}

		ret = ptsname_r(ttyfd, pathname, sizeof(pathname));
		if (ret < 0) {
			pr_error_info("ptsname");
			goto out_close_ttyfd;
		}

#if CAVAN_COMMAND_DEBUG
		pr_func_info("ptsname = `%s'", pathname);
#endif

		pid = cavan_exec_fork();
		if (pid < 0) {
			pr_error_info("cavan_exec_fork");
			ret = pid;
			goto out_close_ttyfd;
		}

		if (pid == 0) {
			close(ttyfd);

			return cavan_exec_redirect_stdio2(pathname, lines, columns, command, flags);
		}
	}

	if (ppid) {
		*ppid = pid;
	}

	return ttyfd;

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_exec_get_temp_pipe_pathname(char *pathname, size_t size, pid_t pid, int type)
{
	return snprintf(pathname, size, CAVAN_EXEC_PIPE_PATH "/%s-%d", cavan_exec_tty_prefix[type], pid);
}

int cavan_exec_make_temp_pipe(char *pathname, size_t size, pid_t pid, int type)
{
	int ret;
	bool retry = true;

	cavan_exec_get_temp_pipe_pathname(pathname, size, pid, type);

do_mkfifo:
	ret = mkfifo(pathname, 0777 | S_IFIFO);
	if (ret < 0) {
		if (retry) {
			retry = false;

			switch (errno) {
			case EEXIST:
				if (likely(file_is_fifo(pathname))) {
					return 0;
				}

				unlink(pathname);
				goto do_mkfifo;

			case ENOTDIR:
				unlink(CAVAN_EXEC_PIPE_PATH);
			case ENOENT:
				ret = mkdir_hierarchy(CAVAN_EXEC_PIPE_PATH, 0777);
				if (ret == 0) {
					goto do_mkfifo;
				}
			}
		}

		pr_err_info("mkfifo `%s'", pathname);
		return ret;
	}

#if CAVAN_COMMAND_DEBUG
	pr_func_info("create pipe `%s'", pathname);
#endif

	return 0;
}

int cavan_exec_make_temp_pipe2(pid_t pid, int flags)
{
	int i;

	for (i = 0; i < 3; i++) {
		int ret;
		char pathname[1024];

		if ((flags & (1 << i)) == 0) {
			continue;
		}

		ret = cavan_exec_make_temp_pipe(pathname, sizeof(pathname), pid, i);
		if (ret < 0) {
			pr_red_info("cavan_create_temp_pipe %s", pathname);

			while (--i >= 0) {
				if ((flags & (1 << i)) == 0) {
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

void cavan_exec_unlink_temp_pipe(char *const ttypath[3], pid_t pid, int count, int flags)
{
	int i;

	if (count < 0 || count > 3) {
		count = 3;
	}

	if (ttypath) {
		for (i = 0; i < count; i++) {
			if (ttypath[i]) {
				unlink(ttypath[i]);
			}
		}
	} else {
		for (i = 0; i < count; i++) {
			char pathname[1024];

			cavan_exec_get_temp_pipe_pathname(pathname, sizeof(pathname), pid, i);
			unlink(pathname);
		}
	}
}

int cavan_exec_open_temp_pipe_master(int ttyfds[3], char *const ttypath[3], pid_t pid, int flags)
{
	int i;
	int ret;

	for (i = 0; i < 3; i++) {
		char buff[1024];
		const char *pathname;

		if (ttypath) {
			pathname = ttypath[i];
		} else if (flags & (1 << i)) {
			ret = cavan_exec_make_temp_pipe(buff, sizeof(buff), pid, i);
			if (ret < 0) {
				pr_red_info("cavan_exec_make_temp_pipe: %d", ret);
				goto out_unlink_pipe;
			}

			pathname = buff;
		} else {
			pathname = NULL;
		}

		if (pathname) {
			int fd;

#if CAVAN_COMMAND_DEBUG
			pr_func_info("pathname = `%s'", pathname);
#endif

			fd = open(pathname, cavan_exec_tty_master_open_flags[i]);
			if (fd < 0) {
				pr_red_info("cavan_open_pipe_once %s", pathname);
				ret = fd;
				goto out_unlink_pipe;
			}

#if CAVAN_COMMAND_DEBUG
			pr_func_info("pathname = `%s', fd = %d", pathname, fd);
#endif

			ttyfds[i] = fd;

			if (flags & CAVAN_EXECF_DEL_TTY) {
				unlink(pathname);
			}
		} else {
			ttyfds[i] = -1;
		}
	}

	if (flags & (CAVAN_EXECF_ERR_TO_OUT) && ttyfds[2] < 0) {
		ttyfds[2] = ttyfds[1];
	}

#if CAVAN_COMMAND_DEBUG
	pr_func_info("ttyfds = [%d, %d, %d]", ttyfds[0], ttyfds[1], ttyfds[2]);
#endif

	return 0;

out_unlink_pipe:
	if (flags & CAVAN_EXECF_DEL_TTY) {
		cavan_exec_unlink_temp_pipe(ttypath, pid, i + 1, flags);
	}

	cavan_exec_close_temp_pipe(ttyfds, i);
	return ret;
}

int cavan_exec_open_temp_pipe_slave(int ttyfds[3], pid_t pid, int flags)
{
	int i;
	int auto_open;

	auto_open = flags & CAVAN_EXECF_AUTO_OPEN;
	if (auto_open) {
		flags |= 0x07;
	}

	for (i = 0; i < 3; i++) {
		int fd;

		if (flags & (1 << i)) {
			char pathname[1024];

			cavan_exec_get_temp_pipe_pathname(pathname, sizeof(pathname), pid, i);

#if CAVAN_COMMAND_DEBUG
			pr_func_info("pathname = `%s'", pathname);
#endif

do_open:
			fd = open(pathname, cavan_exec_tty_slave_open_flags[i]);
			if (fd < 0 && (auto_open == 0 || errno != ENOENT)) {
				if (errno == EINTR) {
					msleep(100);
					goto do_open;
				}

				pr_err_info("open %s", pathname);

				while (--i >= 0) {
					close(ttyfds[i]);
				}

				return fd;
			}

#if CAVAN_COMMAND_DEBUG
			pr_func_info("pathname = `%s', fd = %d", pathname, fd);
#endif
		} else {
			fd = -1;
		}

		ttyfds[i] = fd;
	}

#if CAVAN_COMMAND_DEBUG
	pr_func_info("ttyfds = [%d, %d, %d]", ttyfds[0], ttyfds[1], ttyfds[2]);
#endif

	return 0;
}

void cavan_exec_close_temp_pipe(int ttyfds[3], int count)
{
	int i;

	if (count < 0 || count > 3) {
		count = 3;
	}

	for (i = 0; i < count; i++) {
		if (ttyfds[i] >= 0) {
			close(ttyfds[i]);
		}
	}
}

static void *cavan_exec_redirect_thread_handler(void *_data)
{
	int ret;
	int ttyfds[3];
	struct cavan_exec_pipe_thread_data *data = _data;

#if CAVAN_COMMAND_DEBUG
	pr_func_info("fd = %d, pid = %d", data->fd, data->pid);
#endif

	ret = cavan_exec_open_temp_pipe_master(ttyfds, NULL, data->pid, data->flags);
	if (ret < 0) {
		pr_red_info("cavan_exec_open_temp_pipe: %d", ret);
		return NULL;
	}

	cavan_tty_redirect_loop3(data->fd, data->fd, -1, ttyfds[0], ttyfds[1], ttyfds[2]);

	close(data->fd);
	cavan_exec_close_temp_pipe(ttyfds, -1);

	free(data);

	return NULL;
}

int cavan_exec_redirect_stdio_popen2(const char *command, int lines, int columns, pid_t *ppid, int flags)
{
	int fd;
	int ret;
	pid_t pid;
	struct cavan_exec_pipe_thread_data *data;

#if CAVAN_COMMAND_DEBUG
	pr_func_info("command = `%s', lines = %d, columns = %d, flags = 0x%08x", command, lines, columns, flags);
#endif

	umask(0);
	signal(SIGCHLD, SIG_IGN);

	flags |= CAVAN_EXECF_DEL_TTY | CAVAN_EXECF_ERR_TO_OUT;

	if (lines < 0 || columns < 0) {
		pid = cavan_exec_fork();
		if (pid < 0) {
			pr_err_info("cavan_exec_fork");
			return pid;
		}

		if (pid > 0) {
			ret = cavan_exec_make_temp_pipe2(pid, flags);
			if (ret < 0) {
				pr_red_info("cavan_exec_make_temp_pipe2: %d", ret);
				return ret;
			}
		} else {
			return cavan_exec_redirect_stdio(NULL, command, flags);
		}
	} else {
		fd = cavan_exec_redirect_stdio_popen(command, lines, columns, &pid, flags);
		if (fd < 0) {
			pr_red_info("cavan_exec_redirect_stdio_popen: %d", fd);
			return fd;
		}

		ret = cavan_exec_make_temp_pipe2(pid, flags);
		if (ret < 0) {
			pr_red_info("cavan_exec_make_temp_pipe2: %d", ret);
			goto out_close_fd;
		}

		data = malloc(sizeof(struct cavan_exec_pipe_thread_data));
		if (data == NULL) {
			pr_err_info("malloc");
			ret = -ENOMEM;
			goto out_kill_pid;
		}

	#if CAVAN_COMMAND_DEBUG
		pr_func_info("pid = %d", pid);
	#endif

		data->fd = fd;
		data->pid = pid;
		data->flags = flags;

		ret = cavan_pthread_run(cavan_exec_redirect_thread_handler, data);
		if (ret < 0) {
			pr_red_info("cavan_pthread_create: %d", ret);
			goto out_free_data;
		}
	}

	if (ppid) {
		*ppid = pid;
	}

	return 0;

out_free_data:
	free(data);
out_kill_pid:
	kill(pid, SIGKILL);
	cavan_exec_unlink_temp_pipe(NULL, pid, -1, flags);
out_close_fd:
	if (fd >= 0) {
		close(fd);
	}
	return ret;
}

int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int ttyin, int ttyout)
{
	int ret;
	int ttyfd;
	pid_t pid;

	ttyfd = cavan_exec_redirect_stdio_popen(command, lines, columns, &pid, 0x07);
	if (ttyfd < 0) {
		pr_red_info("cavan_exec_redirect_stdio_popen");
		return ttyfd;
	}

	if (ttyin < 0) {
		ttyin = stdin_fd;
	}

	if (ttyout < 0) {
		ttyout = stdout_fd;
	}

	ret = cavan_tty_redirect_loop3(ttyfd, ttyfd, -1, ttyin, ttyout, stderr_fd);
	if (ret < 0) {
		pr_red_info("cavan_tty_redirect_loop");
		goto out_close_ttyfd;
	}

	ret = cavan_exec_waitpid(pid);

out_close_ttyfd:
	close(ttyfd);
	return ret;
}

int cavan_tty_redirect(int ttyin, int ttyout, int ttyerr)
{
	int ret;
	struct termios tty_attr;

	ret = cavan_tty_set_mode(stdin_fd, CAVAN_TTY_MODE_SSH, &tty_attr);
	if (ret < 0) {
		pr_red_info("cavan_tty_set_mode");
		return ret;
	}

	ret = cavan_tty_redirect_loop4(ttyin, ttyout, ttyerr);

	cavan_tty_attr_restore(stdin_fd, &tty_attr);

	return ret;
}

int cavan_tty_redirect3(const char *ttypath)
{
	int ret;
	int fd;

	fd = open(ttypath, O_RDWR);
	if (fd < 0) {
		pr_error_info("open tty device %s", ttypath);
		return fd;
	}

	ret = cavan_tty_redirect2(fd);

	close(fd);

	return ret;
}

const char *cavan_getenv(const char *name, const char *default_value)
{
	const char *value;

	value = getenv(name);
	if (value) {
		return value;
	}

	return default_value;
}

u32 cavan_getenv_u32(const char *name, u32 default_value)
{
	const char *value;

	value = cavan_getenv(name, NULL);
	if (value == NULL) {
		return default_value;
	}

	return text2value_unsigned(value, NULL, 10);
}

int tty_get_win_size(int tty, int size[2])
{
	int ret;
	struct winsize wsize;

	ret = ioctl(tty, TIOCGWINSZ, &wsize);
	if (ret < 0) {
		if (isatty(tty)) {
			size[0] = cavan_getenv_u32("LINES", 0);
			size[1] = cavan_getenv_u32("COLUMNS", 0);
		} else {
			size[0] = size[1] = -1;
		}
	} else {
		size[0] = wsize.ws_row;
		size[1] = wsize.ws_col;
	}

	return ret;
}

int tty_get_win_size2(int tty, int *lines, int *columns)
{
	int ret;
	int size[2];

	ret = tty_get_win_size(tty, size);

	if (lines) {
		*lines = size[0];
	}

	if (columns) {
		*columns = size[1];
	}

	return ret;
}

int tty_get_win_size3(int tty, u16 *lines, u16 *columns)
{
	int ret;
	int size[2];

	ret = tty_get_win_size(tty, size);

	if (lines) {
		*lines = size[0] < 0 ? 0xFFFF : size[0];
	}

	if (columns) {
		*columns = size[1] < 0 ? 0xFFFF : size[1];
	}

	return ret;
}

int tty_set_win_size(int tty, u16 lines, u16 columns)
{
	int ret;
	char buff[64];
	struct winsize wsize = {
		.ws_row = lines,
		.ws_col = columns,
		.ws_xpixel = 0,
		.ws_ypixel = 0
	};

	ret = ioctl(tty, TIOCSWINSZ, &wsize);
	if (ret < 0) {
		pr_error_info("ioctl TIOCSWINSZ");
		return ret;
	}

	sprintf(buff, "LINES=%d", lines);
	putenv(buff);
	sprintf(buff, "COLUMNS=%d", columns);
	putenv(buff);

	return 0;
}

int cavan_system(const char *command, int argc, char *argv[])
{
	pid_t pid = cavan_exec_fork();

	return pid == 0 ? cavan_exec_command(command, argc, argv) : cavan_exec_waitpid(pid);
}

int cavan_system2(const char *command, ...)
{
	va_list ap;
	char buff[2048];

	va_start(ap, command);
	vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

	return cavan_system(buff, 0, NULL);
}

int cavan_popen(const char *command, char *buff, size_t size, char **buff_ret)
{
	int fd;
	pid_t pid;

	fd = cavan_exec_redirect_stdio_popen(command, -1, -1, &pid, CAVAN_EXECF_STDOUT);
	if (fd < 0) {
		pr_red_info("cavan_exec_redirect_stdio_popen, command = %s", command);
		return fd;
	}

	while (size > 0) {
		ssize_t rdlen;

		rdlen = ffile_read(fd, buff, size);
		if (rdlen <= 0) {
			break;
		}

		buff += rdlen;
		size -= rdlen;
	}

	close(fd);

	if (buff_ret) {
		*buff_ret = buff;
	}

	return cavan_exec_waitpid(pid);
}

int cavan_popen2(const char *command, char *buff, size_t size)
{
	char *p;
	int ret;

	ret = cavan_popen(command, buff, size - 1, &p);
	if (ret < 0) {
		return ret;
	}

	*p = 0;

	while (--p > buff && byte_is_space_or_lf(*p)) {
		*p = 0;
	}

	return ret;
}

int cavan_popen3(char *buff, size_t size, char **buff_ret, const char *command, ...)
{
	va_list ap;
	char command_buff[1024];

	va_start(ap, command);
	vsnprintf(command_buff, sizeof(command_buff), command, ap);
	va_end(ap);

	return cavan_popen(command_buff, buff, size, buff_ret);
}

int cavan_popen4(char *buff, size_t size, const char *command, ...)
{
	va_list ap;
	char command_buff[1024];

	va_start(ap, command);
	vsnprintf(command_buff, sizeof(command_buff), command, ap);
	va_end(ap);

	return cavan_popen2(command_buff, buff, size);
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

	if (filename == NULL) {
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = stderr;
#else
		fd = 2;
#endif
	} else if (strcmp(filename, "-") == 0) {
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = stdout;
#else
		fd = 1;
#endif
	} else if (command) {
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = popen(filename, "w");
		if (fp == NULL) {
			pr_err_info("popen %s", filename);
			return -EFAULT;
		}

		setvbuf(fp, NULL, _IONBF, 0);
#else
		fd = cavan_exec_redirect_stdio_popen(filename, -1, -1, NULL, 0x01);
		if (fd < 0) {
			pr_red_info("cavan_exec_redirect_stdio_popen: %d", fd);
			return fd;
		}
#endif
	} else {
#if CAVAN_TEE_USE_SYSTEM_POPEN
		fp = fopen(filename, append ? "a+" : "w+");
		if (fp == NULL) {
			pr_err_info("fopen %s", filename);
			return -EFAULT;
		}
#else
		fd = open(filename, (append ? O_APPEND : O_TRUNC) | O_WRONLY | O_CREAT, 0777);
		if (fd < 0) {
			pr_err_info("open %s", filename);
			return fd;
		}
#endif
	}

	while (1) {
		ssize_t rdlen;
		ssize_t wrlen;
		char buff[1024];

		rdlen = ffile_read(0, buff, sizeof(buff));
		if (rdlen <= 0) {
			break;
		}

#if CAVAN_TEE_USE_SYSTEM_POPEN
		wrlen = fwrite(buff, rdlen, 1, fp) | fwrite(buff, rdlen, 1, stdout);
#else
		wrlen = ffile_write(1, buff, rdlen) | ffile_write(fd, buff, rdlen);
#endif
		if (wrlen != rdlen) {
			// pr_err_info("ffile_write");
			break;
		}

		total += rdlen;
	}

#if CAVAN_TEE_USE_SYSTEM_POPEN
	if (fp != stderr && fp != stdout) {
		if (command) {
			pclose(fp);
		} else {
			fclose(fp);
		}
	}
#else
	if (fd != 1 && fd != 2) {
		close(fd);
	}
#endif

	if (ret < 0) {
		return ret;
	}

	if (total > 0) {
		return 0;
	}

	return -ENODATA;
}

// ================================================================================

int cavan_tty_redirect_loop(int ttyfds[][2], int count)
{
	int ret;
	struct cavan_file_proxy_desc desc;

	ret = cavan_file_proxy_init(&desc);
	if (ret < 0) {
		pr_red_info("cavan_file_proxy_init: %d", ret);
		return ret;
	}

	ret = cavan_file_proxy_add_array(&desc, ttyfds, count);
	if (ret < 0) {
		pr_red_info("cavan_file_proxy_add_array: %d", ret);
		goto out_cavan_file_proxy_deinit;
	}

	ret = cavan_file_proxy_main_loop(&desc);

out_cavan_file_proxy_deinit:
	cavan_file_proxy_deinit(&desc);
	return ret;
}

int cavan_tty_redirect_loop2(const int *ttyin, const int *ttyout, int count)
{
	int i;
	int ttyfds[count][2];

	for (i = 0; i < count; i++) {
		ttyfds[i][0] = ttyin[i];
		ttyfds[i][1] = ttyout[i];
	}

	return cavan_tty_redirect_loop(ttyfds, count);
}

int cavan_tty_redirect_loop3(int in, int out, int err, int ttyin, int ttyout, int ttyerr)
{
	int ttyfds[][2] = {
		{ ttyin, in },
		{ out, ttyout },
		{ err, ttyerr },
	};

	return cavan_tty_redirect_loop(ttyfds, NELEM(ttyfds));
}

static void cavan_exit_ask_handler(int signum)
{
	static int count;

	// println("signum = %d, count = %d", signum, count);

	if (cavan_atomic_inc(&count) > 1) {
		exit(0);
	}

	if (cavan_get_choose_yesno("Do you want to exit?", false, -1)) {
		exit(0);
	}

	cavan_atomic_dec(&count);
}

void cavan_set_exit_ask(void)
{
	signal(SIGINT, cavan_exit_ask_handler);
}

int cavan_reboot(bool shutdown, const char *command)
{
	int ret;

	sync();

	ret = cavan_remount_ro_done();
	if (ret < 0) {
		pr_err_info("cavan_remount_ro_done");
	}

	if (shutdown) {
		ret = reboot(RB_POWER_OFF);
		if (ret < 0) {
			pr_err_info("reboot: %d", ret);
			return ret;
		}
	} else if (command && command[0]) {
		ret = syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, command);
		if (ret < 0) {
			pr_err_info("syscall-%d: %d", SYS_reboot, ret);
			return ret;
		}
	} else {
		ret = reboot(RB_AUTOBOOT);
		if (ret < 0) {
			pr_err_info("reboot: %d", ret);
			return ret;
		}
	}

	return 0;
}

// ================================================================================

int cavan_cmdline_parse(char *cmdline, char *argv[], int size)
{
	int argc = 0;
	char *p = cmdline;

	argv[argc] = p;

	while (1) {
		switch (*cmdline) {
		case ' ':
		case '\t':
			if (p != argv[argc]) {
				*p++ = 0;

				if (++argc >= size) {
					return argc;
				}

				argv[argc] = p;
			}
			break;

		case '\r':
		case '\n':
		case '\f':
		case '\0':
label_parse_complete:
			if (p != argv[argc]) {
				*p = 0;
				return argc + 1;
			} else {
				return argc;
			}

		case '\\':
			switch (*++cmdline) {
			case '\r':
			case '\n':
			case '\f':
			case '\0':
				goto label_parse_complete;
			}
		default:
			*p++ = *cmdline;
		}

		cmdline++;
	}
}

const struct cavan_command_entry2 *cavan_pipe_cmdline_find(const char *name, const struct cavan_command_entry2 cmd_list[], size_t cmd_count)
{
	const struct cavan_command_entry2 *cmd_end;

	for (cmd_end = cmd_list + cmd_count; cmd_list < cmd_end; cmd_list++) {
		if (strcmp(name, cmd_list->name) == 0) {
			return cmd_list;
		}
	}

	return NULL;
}

int cavan_pipe_cmdline_loop(FILE *fp, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	int i;
	int ret;
	int argc;
	char *argv[100];
	char command[1024];
	const struct cavan_command_entry2 *cmd;

	setlinebuf(fp);

	while (fgets(command, sizeof(command), fp)) {
		argc = cavan_cmdline_parse(command, argv, NELEM(argv));
		if (argc < 1) {
			pr_red_info("cavan_cmdline_parse: argc = %d", argc);
			continue;
		}

		for (i = 0; i < argc; i++) {
			pd_info("argv[%d] = %s", i, argv[i]);
		}

		cmd = cavan_pipe_cmdline_find(argv[0], cmd_list, cmd_count);
		if (cmd == NULL) {
			pr_red_info("cavan_pipe_cmdline_find");
			continue;
		}

		ret = cmd->handler(argc, argv, data);
		if (ret < 0) {
			pr_red_info("cmd->handler: %d", ret);
			continue;
		}
	}

	return 0;
}

int cavan_pipe_cmdline_loop2(int fd, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	int ret;
	FILE *fp;

	fp = fdopen(fd, "r");
	if (fp == NULL) {
		pr_err_info("fdopen %d", fd);
		return -EFAULT;
	}

	ret = cavan_pipe_cmdline_loop(fp, cmd_list, cmd_count, data);

	fclose(fp);

	return ret;
}

int cavan_pipe_cmdline_loop3(const char *pathname, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	int ret;
	FILE *fp;

	while (1) {
		fp = fopen(pathname, "r");
		if (fp == NULL) {
			if (errno != ENOENT) {
				pr_err_info("open file %s", pathname);
				return -EFAULT;
			}

			ret = mkfifo(pathname, 0777);
			if (ret < 0) {
				const char *p;
				const char *filename;

				if (errno != ENOENT) {
					pr_err_info("mkfifo %s: %d", pathname, ret);
					return ret;
				}

				for (p = pathname, filename = p; *p; p++) {
					if (*p == '/') {
						filename = p + 1;
					}
				}

				while (*filename == '/') {
					filename--;
				}

				ret = mkdir_hierarchy_length(pathname, filename - pathname, 0777);
				if (ret < 0) {
					pr_err_info("mkdir_hierarchy_length: %d", ret);
					return ret;
				}
			} else {
				chmod(pathname, 0777);
			}
		} else {
			cavan_pipe_cmdline_loop(fp, cmd_list, cmd_count, data);
			fclose(fp);
		}
	}

	return 0;
}

static void *cavan_pipe_cmdline_thread(void *data)
{
	struct cavan_pipe_cmdline *cmdline = data;

	cavan_pipe_cmdline_loop(cmdline->fp, cmdline->cmd_list, cmdline->cmd_count, data);

	if (cmdline->need_close) {
		fclose(cmdline->fp);
	}

	free(cmdline);

	return NULL;
}

static void *cavan_pipe_cmdline_thread2(void *data)
{
	struct cavan_pipe_cmdline *cmdline = data;
	bool need_close = cmdline->need_close;

	cmdline->fp = fdopen(cmdline->fd, "r");
	if (cmdline->fp == NULL) {
		pr_err_info("fdopen");
		return NULL;
	}

	cmdline->need_close = true;
	cavan_pipe_cmdline_thread(data);

	if (need_close) {
		close(cmdline->fd);
	}

	return NULL;
}

static void *cavan_pipe_cmdline_thread3(void *data)
{
	struct cavan_pipe_cmdline *cmdline = data;

	cavan_pipe_cmdline_loop3(cmdline->pathname, cmdline->cmd_list, cmdline->cmd_count, data);
	free(cmdline);

	return NULL;
}

int cavan_pipe_cmdline_run(FILE *fp, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	struct cavan_pipe_cmdline *cmdline;

	cmdline = malloc(sizeof(struct cavan_pipe_cmdline));
	if (cmdline == NULL) {
		pr_err_info("malloc");
		return -ENOMEM;
	}

	cmdline->fp = fp;
	cmdline->data = data;
	cmdline->need_close = false;
	cmdline->cmd_list = cmd_list;
	cmdline->cmd_count = cmd_count;

	return cavan_pthread_create(NULL, cavan_pipe_cmdline_thread, cmdline, false);
}

int cavan_pipe_cmdline_run2(int fd, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	struct cavan_pipe_cmdline *cmdline;

	cmdline = malloc(sizeof(struct cavan_pipe_cmdline));
	if (cmdline == NULL) {
		pr_err_info("malloc");
		return -ENOMEM;
	}

	cmdline->fd = fd;
	cmdline->data = data;
	cmdline->need_close = false;
	cmdline->cmd_list = cmd_list;
	cmdline->cmd_count = cmd_count;

	return cavan_pthread_create(NULL, cavan_pipe_cmdline_thread2, cmdline, false);
}

int cavan_pipe_cmdline_run3(const char *pathname, const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	struct cavan_pipe_cmdline *cmdline;

	cmdline = malloc(sizeof(struct cavan_pipe_cmdline));
	if (cmdline == NULL) {
		pr_err_info("malloc");
		return -ENOMEM;
	}

	cmdline->data = data;
	cmdline->pathname = pathname;
	cmdline->need_close = false;
	cmdline->cmd_list = cmd_list;
	cmdline->cmd_count = cmd_count;

	return cavan_pthread_create(NULL, cavan_pipe_cmdline_thread3, cmdline, false);
}

int cavan_pipe_cmdline_run4(const struct cavan_command_entry2 cmd_list[], size_t cmd_count, void *data)
{
	int ret;
	int pipefd[2];

	ret = pipe(pipefd);
	if (ret < 0) {
		pr_err_info("pipe: %d", ret);
		return ret;
	}

	ret = cavan_pipe_cmdline_run2(pipefd[0], cmd_list, cmd_count, data);
	if (ret < 0) {
		close(pipefd[0]);
		close(pipefd[1]);

		pr_red_info("cavan_pipe_cmdline_run2: %d", ret);
		return ret;
	}

	return pipefd[1];
}

// ================================================================================

int cavan_async_command_service_init(struct cavan_async_command_service *service)
{
	pthread_mutex_init(&service->lock, NULL);
	pthread_cond_init(&service->cond, NULL);

	return 0;
}

void cavan_async_command_service_deinit(struct cavan_async_command_service *service)
{
	pthread_mutex_destroy(&service->lock);
	pthread_cond_destroy(&service->cond);
}

static void *cavan_async_command_thread_handler(void *data)
{
	int ret;
	struct cavan_async_command_service *service = data;

	cavan_async_command_service_lock(service);

	while (service->head) {
		struct cavan_async_command *head = service->head;

		ret = pthread_cond_timedwait(&service->cond, &service->lock, &head->spec);
		if (ret < 0) {
			pd_err_info("pthread_cond_timedwait: %d", ret);
			msleep(2000);
		} else if (ret == ETIMEDOUT && head == service->head) {
			service->head = head->next;

			cavan_async_command_service_unlock(service);
			head->handler(head->data);
			free(head);
			cavan_async_command_service_lock(service);
		}
	}

	service->running = false;

	cavan_async_command_service_unlock(service);

	return NULL;
}

int cavan_async_command_execute(struct cavan_async_command_service *service, void (*handler)(void *data), void *data, long msec)
{
	int ret;
	struct cavan_async_command *desc;
	struct cavan_async_command **head;

	desc = malloc(sizeof(struct cavan_async_command));
	if (desc == NULL) {
		pd_err_info("malloc");
		return -ENOMEM;
	}

	ret = cavan_timer_set_timespec(&desc->spec, msec);
	if (ret < 0) {
		pd_err_info("cavan_timer_set_timespec: %d", ret);
		goto out_free;
	}

	if (service == NULL) {
		service = &s_async_command_service_default;
	}

	desc->data = data;
	desc->handler = handler;

	cavan_async_command_service_lock(service);

	for (head = &service->head; (*head) && cavan_timespec_cmp(&desc->spec, &(*head)->spec) >= 0; head = &(*head)->next);

	desc->next = *head;
	*head = desc;

	if (service->running) {
		if (desc == service->head) {
			pthread_cond_signal(&service->cond);
		}
	} else {
		service->running = true;
		cavan_pthread_run(cavan_async_command_thread_handler, service);
	}

	cavan_async_command_service_unlock(service);

	return 0;

out_free:
	free(desc);
	return ret;
}

int cavan_async_command_cancel(struct cavan_async_command_service *service, void (*handler)(void *data), int max)
{
	int count = 0;
	struct cavan_async_command **head;

	if (service == NULL) {
		service = &s_async_command_service_default;
	}

	cavan_async_command_service_lock(service);

	head = &service->head;

	while (*head)
	{
		if ((*head)->handler == handler) {
			struct cavan_async_command *desc;

			desc = *head;
			*head = desc->next;

			free(desc);

			if (++count >= max && max > 0) {
				break;
			}
		} else {
			head = &(*head)->next;
		}
	}

	cavan_async_command_service_unlock(service);

	return count;
}
