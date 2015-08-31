#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/wait.h>

#define FIND_EXEC_COMMAND_MAIN(map) \
	int main(int argc, char *argv[]) \
	{ \
		if (argc > 1) \
		{ \
			return find_and_exec_command(map, ARRAY_SIZE(map), argc - 1, argv + 1); \
		} \
		return print_command_table(map, map + ARRAY_SIZE(map)); \
	}

#define CAVAN_COMMAND_MAP_START \
	static struct cavan_command_map __local_cmd_map[] = \
	{

#define CAVAN_COMMAND_MAP_END \
	}; \
	FIND_EXEC_COMMAND_MAIN(__local_cmd_map);

typedef enum
{
	CAVAN_COMMAND_OPTION_ADB = 0x256,
	CAVAN_COMMAND_OPTION_AUTO,
	CAVAN_COMMAND_OPTION_APPEND,
	CAVAN_COMMAND_OPTION_BASE,
	CAVAN_COMMAND_OPTION_BOARD,
	CAVAN_COMMAND_OPTION_BT_MAC,
	CAVAN_COMMAND_OPTION_COMMAND,
	CAVAN_COMMAND_OPTION_CONTEXT,
	CAVAN_COMMAND_OPTION_COUNT,
	CAVAN_COMMAND_OPTION_CONFIG,
	CAVAN_COMMAND_OPTION_CHECK_ALL,
	CAVAN_COMMAND_OPTION_DAEMON,
	CAVAN_COMMAND_OPTION_DAEMON_MAX,
	CAVAN_COMMAND_OPTION_DAEMON_MIN,
	CAVAN_COMMAND_OPTION_DATE,
	CAVAN_COMMAND_OPTION_DEVICE,
	CAVAN_COMMAND_OPTION_DRIVER,
	CAVAN_COMMAND_OPTION_ERASE,
	CAVAN_COMMAND_OPTION_EXEC,
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
	CAVAN_COMMAND_OPTION_ROOT,
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
	CAVAN_COMMAND_OPTION_SYSTEM,
	CAVAN_COMMAND_OPTION_USERDATA,
	CAVAN_COMMAND_OPTION_RECOVERY,
	CAVAN_COMMAND_OPTION_MISC,
	CAVAN_COMMAND_OPTION_BOOT,
	CAVAN_COMMAND_OPTION_KERNEL,
	CAVAN_COMMAND_OPTION_KERNEL_ADDR,
	CAVAN_COMMAND_OPTION_KERNEL_OFFSET,
	CAVAN_COMMAND_OPTION_RAMDISK,
	CAVAN_COMMAND_OPTION_RAMDISK_ADDR,
	CAVAN_COMMAND_OPTION_RAMDISK_OFFSET,
	CAVAN_COMMAND_OPTION_SECOND,
	CAVAN_COMMAND_OPTION_SECOND_ADDR,
	CAVAN_COMMAND_OPTION_SECOND_OFFSET,
	CAVAN_COMMAND_OPTION_TAGS_ADDR,
	CAVAN_COMMAND_OPTION_TAGS_OFFSET,
	CAVAN_COMMAND_OPTION_DT,
	CAVAN_COMMAND_OPTION_CMDLINE,
	CAVAN_COMMAND_OPTION_UBOOT,
	CAVAN_COMMAND_OPTION_NAME,
	CAVAN_COMMAND_OPTION_PAGE_SIZE,
	CAVAN_COMMAND_OPTION_UNUSED,
	CAVAN_COMMAND_OPTION_REMAIN,
	CAVAN_COMMAND_OPTION_RESOURCE
} cavan_command_option_t;

struct cavan_command_map
{
	const char *name;
	int (*main_func)(int argc, char *argv[]);
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

// ============================================================

int print_command_table(const struct cavan_command_map *p, const struct cavan_command_map *p_end);
const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size);
void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[]);

int cavan_redirect_stdio_base(int ttyfd, int flags);
int cavan_redirect_stdio(const char *pathname, int flags);
int cavan_exec_redirect_stdio_base(int ttyfd, const char *command, int flags);
int cavan_exec_redirect_stdio(const char *ttypath, int lines, int columns, const char *command, int flags);
int cavan_exec_redirect_stdio_popen(const char *command, int lines, int columns, pid_t *ppid, int flags);
int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int in_fd, int out_fd);
int cavan_system(const char *command);
int cavan_system2(const char *command, ...);
int cavan_tty_tee_loop(int ttyfd, int ttyin, int ttyout);
int cavan_tee_main(const char *filename, bool append, bool command);

int cavan_tty_redirect_loop(int ttyfd, int ttyin, int ttyout);
int cavan_tty_redirect_base(int ttyfd);
int cavan_tty_redirect(const char *ttypath);

const char *cavan_getenv(const char *name, const char *default_value);
u32 cavan_getenv_u32(const char *name, u32 default_value);

int tty_get_win_size(int tty, u16 *lines, u16 *columns);
int tty_set_win_size(int tty, u16 lines, u16 columns);

static inline int cavan_exec_waitpid(pid_t pid)
{
	int status;

	if (waitpid(pid, &status, 0) == pid)
	{
		return WEXITSTATUS(status);
	}

	return 0;
}
