#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/wait.h>

#define CAVAN_EXEC_PIPE_PATH	"/dev/cavan/pipe"

#define FIND_EXEC_COMMAND_MAIN(map) \
	int main(int argc, char *argv[]) { \
		if (argc > 1) { \
			return find_and_exec_command(map, ARRAY_SIZE(map), argc - 1, argv + 1); \
		} \
		return print_command_table(map, map + ARRAY_SIZE(map)); \
	}

#define CAVAN_COMMAND_MAP_START \
	static struct cavan_command_map __local_cmd_map[] =

#define CAVAN_COMMAND_MAP_END \
	; \
	FIND_EXEC_COMMAND_MAIN(__local_cmd_map);

#define CAVAN_ALIAS_DECLARE(name, command, subcmd) \
	int cavan_alias_##name(int argc, char *argv[]) { \
		int i; \
		char *argv_new[argc + 1]; \
		argv_new[0] = #command; \
		argv_new[1] = #subcmd; \
		for (i = 1; i < argc; i++) { \
			argv_new[i + 1] = argv[i]; \
		} \
		return do_cavan_##command(NELEM(argv_new), argv_new); \
	}

#define CAVAN_EXECF_STDIN			(1 << 0)
#define CAVAN_EXECF_STDOUT			(1 << 1)
#define CAVAN_EXECF_STDERR			(1 << 2)
#define CAVAN_EXECF_DEL_TTY			(1 << 3)
#define CAVAN_EXECF_ERR_TO_OUT		(1 << 4)
#define CAVAN_EXECF_AUTO_OPEN		(1 << 5)

#define CAVAN_BUILTIN_CMDF_FORCE	(1 << 0)

__BEGIN_DECLS;

typedef enum {
	CAVAN_COMMAND_OPTION_ADB = 0x256,
	CAVAN_COMMAND_OPTION_ADDR_BYTES,
	CAVAN_COMMAND_OPTION_ADDR_BIG_ENDIAN,
	CAVAN_COMMAND_OPTION_ALOOP,
	CAVAN_COMMAND_OPTION_ALL,
	CAVAN_COMMAND_OPTION_VALUE_BYTES,
	CAVAN_COMMAND_OPTION_VALUE_BIG_ENDIAN,
	CAVAN_COMMAND_OPTION_BIG_ENDIAN,
	CAVAN_COMMAND_OPTION_LITTLE_ENDIAN,
	CAVAN_COMMAND_OPTION_ACK,
	CAVAN_COMMAND_OPTION_NOACK,
	CAVAN_COMMAND_OPTION_AUTO,
	CAVAN_COMMAND_OPTION_APPEND,
	CAVAN_COMMAND_OPTION_BASE,
	CAVAN_COMMAND_OPTION_BOARD,
	CAVAN_COMMAND_OPTION_BT_MAC,
	CAVAN_COMMAND_OPTION_COMMAND,
	CAVAN_COMMAND_OPTION_CONTEXT,
	CAVAN_COMMAND_OPTION_COUNT,
	CAVAN_COMMAND_OPTION_CONFIG,
	CAVAN_COMMAND_OPTION_CONTINUE,
	CAVAN_COMMAND_OPTION_CHIP,
	CAVAN_COMMAND_OPTION_CHECK_ALL,
	CAVAN_COMMAND_OPTION_DAEMON,
	CAVAN_COMMAND_OPTION_DAEMON_MAX,
	CAVAN_COMMAND_OPTION_DAEMON_MIN,
	CAVAN_COMMAND_OPTION_DATE,
	CAVAN_COMMAND_OPTION_DELAY,
	CAVAN_COMMAND_OPTION_DEVICE,
	CAVAN_COMMAND_OPTION_DRIVER,
	CAVAN_COMMAND_OPTION_ERASE,
	CAVAN_COMMAND_OPTION_EXEC,
	CAVAN_COMMAND_OPTION_FORCE,
	CAVAN_COMMAND_OPTION_FSTAB,
	CAVAN_COMMAND_OPTION_GROUP,
	CAVAN_COMMAND_OPTION_HAL,
	CAVAN_COMMAND_OPTION_HELP,
	CAVAN_COMMAND_OPTION_HOST,
	CAVAN_COMMAND_OPTION_IMAGE,
	CAVAN_COMMAND_OPTION_IMEI1,
	CAVAN_COMMAND_OPTION_IMEI2,
	CAVAN_COMMAND_OPTION_IMEI3,
	CAVAN_COMMAND_OPTION_IMEI4,
	CAVAN_COMMAND_OPTION_IP,
	CAVAN_COMMAND_OPTION_LENGTH,
	CAVAN_COMMAND_OPTION_LOGIN,
	CAVAN_COMMAND_OPTION_LIST,
	CAVAN_COMMAND_OPTION_LOCAL,
	CAVAN_COMMAND_OPTION_LOGFILE,
	CAVAN_COMMAND_OPTION_LONG,
	CAVAN_COMMAND_OPTION_LOOP,
	CAVAN_COMMAND_OPTION_MASK,
	CAVAN_COMMAND_OPTION_MODE,
	CAVAN_COMMAND_OPTION_NONE,
	CAVAN_COMMAND_OPTION_PARENTS,
	CAVAN_COMMAND_OPTION_PIDFILE,
	CAVAN_COMMAND_OPTION_PIPE,
	CAVAN_COMMAND_OPTION_PORT,
	CAVAN_COMMAND_OPTION_PREFIX,
	CAVAN_COMMAND_OPTION_PROTOCOL,
	CAVAN_COMMAND_OPTION_PROXY_HOST,
	CAVAN_COMMAND_OPTION_PROXY_PORT,
	CAVAN_COMMAND_OPTION_PROXY_PROTOCOL,
	CAVAN_COMMAND_OPTION_PROXY_URL,
	CAVAN_COMMAND_OPTION_READ,
	CAVAN_COMMAND_OPTION_REPEAT,
	CAVAN_COMMAND_OPTION_RESET,
	CAVAN_COMMAND_OPTION_RESUME,
	CAVAN_COMMAND_OPTION_ROOT,
	CAVAN_COMMAND_OPTION_SERVER,
	CAVAN_COMMAND_OPTION_SERVICE,
	CAVAN_COMMAND_OPTION_SHELL,
	CAVAN_COMMAND_OPTION_SIZE,
	CAVAN_COMMAND_OPTION_START,
	CAVAN_COMMAND_OPTION_STOP,
	CAVAN_COMMAND_OPTION_SUPER,
	CAVAN_COMMAND_OPTION_TCP,
	CAVAN_COMMAND_OPTION_TIME,
	CAVAN_COMMAND_OPTION_UDP,
	CAVAN_COMMAND_OPTION_UNKNOWN,
	CAVAN_COMMAND_OPTION_UNIX,
	CAVAN_COMMAND_OPTION_UNIX_TCP,
	CAVAN_COMMAND_OPTION_UNIX_UDP,
	CAVAN_COMMAND_OPTION_URL,
	CAVAN_COMMAND_OPTION_VERBOSE,
	CAVAN_COMMAND_OPTION_VERSION,
	CAVAN_COMMAND_OPTION_WIFI_MAC,
	CAVAN_COMMAND_OPTION_WRITE,
	CAVAN_COMMAND_OPTION_DISCOVERY,
	CAVAN_COMMAND_OPTION_DISCOVERY_DELAY,
	CAVAN_COMMAND_OPTION_SYSTEM,
	CAVAN_COMMAND_OPTION_USERDATA,
	CAVAN_COMMAND_OPTION_OVERRIDE,
	CAVAN_COMMAND_OPTION_RATE,
	CAVAN_COMMAND_OPTION_RECOVERY,
	CAVAN_COMMAND_OPTION_ROCKCHIP,
	CAVAN_COMMAND_OPTION_MISC,
	CAVAN_COMMAND_OPTION_BOOT,
	CAVAN_COMMAND_OPTION_KERNEL,
	CAVAN_COMMAND_OPTION_KERNEL_ADDR,
	CAVAN_COMMAND_OPTION_KERNEL_OFFSET,
	CAVAN_COMMAND_OPTION_RAMDISK,
	CAVAN_COMMAND_OPTION_RAMDISK_ADDR,
	CAVAN_COMMAND_OPTION_RAMDISK_OFFSET,
	CAVAN_COMMAND_OPTION_SAME,
	CAVAN_COMMAND_OPTION_SECOND,
	CAVAN_COMMAND_OPTION_SECOND_ADDR,
	CAVAN_COMMAND_OPTION_SECOND_OFFSET,
	CAVAN_COMMAND_OPTION_STEP,
	CAVAN_COMMAND_OPTION_TAGS_ADDR,
	CAVAN_COMMAND_OPTION_TAGS_OFFSET,
	CAVAN_COMMAND_OPTION_TYPE,
	CAVAN_COMMAND_OPTION_DT,
	CAVAN_COMMAND_OPTION_CMDLINE,
	CAVAN_COMMAND_OPTION_UBOOT,
	CAVAN_COMMAND_OPTION_NAME,
	CAVAN_COMMAND_OPTION_PAGE_SIZE,
	CAVAN_COMMAND_OPTION_UNUSED,
	CAVAN_COMMAND_OPTION_USER,
	CAVAN_COMMAND_OPTION_REMAIN,
	CAVAN_COMMAND_OPTION_RESOURCE,
	CAVAN_COMMAND_OPTION_LAST = CAVAN_COMMAND_OPTION_RESOURCE
} cavan_command_option_t;

struct cavan_command_map {
	const char *name;
	int (*main_func)(int argc, char *argv[]);
};

struct cavan_builtin_command {
	const char *name;
	int (*handler)(const struct cavan_builtin_command *desc, const char *shell, int argc, char *argv[]);
	int flags;
};

struct cavan_exec_pipe_thread_data {
	int fd;
	int flags;
	pid_t pid;
};

// ============================================================

extern const char *cavan_help_message_help;
extern const char *cavan_help_message_version;
extern const char *cavan_help_message_ip;
extern const char *cavan_help_message_hostname;
extern const char *cavan_help_message_local;
extern const char *cavan_help_message_adb;
extern const char *cavan_help_message_tcp;
extern const char *cavan_help_message_udp;
extern const char *cavan_help_message_unix_tcp;
extern const char *cavan_help_message_unix_udp;
extern const char *cavan_help_message_port;
extern const char *cavan_help_message_url;
extern const char *cavan_help_message_send_file;
extern const char *cavan_help_message_recv_file;
extern const char *cavan_help_message_command;
extern const char *cavan_help_message_login;
extern const char *cavan_help_message_preserve_environment;
extern const char *cavan_help_message_shell;
extern const char *cavan_help_message_input_file;
extern const char *cavan_help_message_output_file;
extern const char *cavan_help_message_bs;
extern const char *cavan_help_message_seek;
extern const char *cavan_help_message_skip;
extern const char *cavan_help_message_count;
extern const char *cavan_help_message_super;
extern const char *cavan_help_message_daemon;
extern const char *cavan_help_message_daemon_min;
extern const char *cavan_help_message_daemon_max;
extern const char *cavan_help_message_verbose;
extern const char *cavan_help_message_logfile;
extern const char *cavan_help_message_proxy_ip;
extern const char *cavan_help_message_proxy_hostname;
extern const char *cavan_help_message_proxy_port;
extern const char *cavan_help_message_proxy_url;
extern const char *cavan_help_message_protocol;
extern const char *cavan_help_message_proxy_protocol;
extern const char *cavan_help_message_system;
extern const char *cavan_help_message_userdata;
extern const char *cavan_help_message_recovery;
extern const char *cavan_help_message_misc;
extern const char *cavan_help_message_boot;
extern const char *cavan_help_message_kernel;
extern const char *cavan_help_message_uboot;
extern const char *cavan_help_message_resource;
extern const char *cavan_help_message_rw_image;
extern const char *cavan_help_message_rw_image_auto;
extern const char *cavan_help_message_driver;
extern const char *cavan_help_message_server;
extern const char *cavan_help_message_service;
extern const char *cavan_help_message_addr_bytes;
extern const char *cavan_help_message_value_bytes;
extern const char *cavan_help_message_big_endian;
extern const char *cavan_help_message_little_endian;
extern const char *cavan_help_message_cmdline;
extern const char *cavan_help_message_user;
extern const char *cavan_help_message_group;
extern const char *cavan_help_message_discovery;
extern const char *cavan_help_message_discovery_delay;

// ============================================================

int print_command_table(const struct cavan_command_map *p, const struct cavan_command_map *p_end);
const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size);
void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[]);

int cavan_exec_waitpid(pid_t pid);
int cavan_exec_command(const char *command, int argc, char *argv[]);
int cavan_redirect_stdio_base(int ttyfds[3]);
int cavan_redirect_stdio_base2(int fd, int flags);
int cavan_redirect_stdio(const char *pathname, int flags);
int cavan_exec_redirect_stdio_base(int ttyfds[3], const char *command);
int cavan_exec_redirect_stdio_base2(int ttyfd, const char *command, int flags);
int cavan_exec_redirect_stdio(char *const ttypath[3], const char *command, int flags);
int cavan_exec_redirect_stdio2(const char *ttypath, int lines, int columns, const char *command, int flags);
int cavan_exec_redirect_stdio_popen(const char *command, int lines, int columns, pid_t *ppid, int flags);
int cavan_exec_redirect_stdio_popen2(const char *command, int lines, int columns, pid_t *ppid, int flags);
int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int in_fd, int out_fd);
int cavan_system(const char *command, int argc, char *argv[]);
int cavan_system2(const char *command, ...);
int cavan_popen(const char *command, char *buff, size_t size, char **buff_ret);
int cavan_popen2(const char *command, char *buff, size_t size);
int cavan_popen3(char *buff, size_t size, char **buff_ret, const char *command, ...);
int cavan_popen4(char *buff, size_t size, const char *command, ...);
int cavan_tty_tee_loop(int ttyfd, int ttyin, int ttyout);
int cavan_tee_main(const char *filename, bool append, bool command);

int cavan_tty_redirect(int ttyin, int ttyout, int ttyerr);
int cavan_tty_redirect3(const char *ttypath);

const char *cavan_getenv(const char *name, const char *default_value);
u32 cavan_getenv_u32(const char *name, u32 default_value);

int tty_get_win_size(int tty, int size[2]);
int tty_get_win_size2(int tty, int *lines, int *columns);
int tty_get_win_size3(int tty, u16 *lines, u16 *columns);
int tty_set_win_size(int tty, u16 lines, u16 columns);
int cavan_exec_set_oom_adj(int pid, int value);

int cavan_exec_get_temp_pipe_pathname(char *pathname, size_t size, pid_t pid, int type);
int cavan_exec_make_temp_pipe(char *pathname, size_t size, pid_t pid, int type);
int cavan_exec_make_temp_pipe2(pid_t pid, int flags);
void cavan_exec_unlink_temp_pipe(char *const ttypath[3], pid_t pid, int count, int flags);
int cavan_exec_open_temp_pipe_master(int ttyfds[3], char *const ttypath[3], pid_t pid, int flags);
int cavan_exec_open_temp_pipe_slave(int ttyfds[3], pid_t pid, int flags);
void cavan_exec_close_temp_pipe(int ttyfds[3], int count);

int cavan_tty_redirect_loop(int ttyfds[][2], int count);
int cavan_tty_redirect_loop2(const int *ttyin, const int *ttyout, int count);
int cavan_tty_redirect_loop3(int in, int out, int err, int ttyin, int ttyout, int ttyerr);

void cavan_set_exit_ask(void);
int cavan_reboot(bool shutdown, const char *command);

static inline int cavan_tty_redirect_loop4(int ttyin, int ttyout, int ttyerr)
{
	return cavan_tty_redirect_loop3(ttyin, ttyout, ttyerr, stdin_fd, stdout_fd, stderr_fd);
}

static inline int cavan_tty_redirect2(int ttyfd)
{
	return cavan_tty_redirect(ttyfd, ttyfd, -1);
}

__END_DECLS;
