#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/wait.h>

typedef enum
{
	CAVAN_COMMAND_OPTION_ADB,
	CAVAN_COMMAND_OPTION_BASE,
	CAVAN_COMMAND_OPTION_BT_MAC,
	CAVAN_COMMAND_OPTION_COMMAND,
	CAVAN_COMMAND_OPTION_CONTEXT,
	CAVAN_COMMAND_OPTION_COUNT,
	CAVAN_COMMAND_OPTION_DAEMON,
	CAVAN_COMMAND_OPTION_DAEMON_MAX,
	CAVAN_COMMAND_OPTION_DAEMON_MIN,
	CAVAN_COMMAND_OPTION_DEVICE,
	CAVAN_COMMAND_OPTION_ERASE,
	CAVAN_COMMAND_OPTION_EXEC,
	CAVAN_COMMAND_OPTION_HELP,
	CAVAN_COMMAND_OPTION_IMAGE,
	CAVAN_COMMAND_OPTION_IMEI1,
	CAVAN_COMMAND_OPTION_IMEI2,
	CAVAN_COMMAND_OPTION_IMEI3,
	CAVAN_COMMAND_OPTION_IMEI4,
	CAVAN_COMMAND_OPTION_IP,
	CAVAN_COMMAND_OPTION_LENGTH,
	CAVAN_COMMAND_OPTION_LIST,
	CAVAN_COMMAND_OPTION_LOCAL,
	CAVAN_COMMAND_OPTION_LOGFILE,
	CAVAN_COMMAND_OPTION_LONG,
	CAVAN_COMMAND_OPTION_MASK,
	CAVAN_COMMAND_OPTION_MODE,
	CAVAN_COMMAND_OPTION_NONE,
	CAVAN_COMMAND_OPTION_PARENTS,
	CAVAN_COMMAND_OPTION_PIDFILE,
	CAVAN_COMMAND_OPTION_PORT,
	CAVAN_COMMAND_OPTION_PREFIX,
	CAVAN_COMMAND_OPTION_PROXY_HOST,
	CAVAN_COMMAND_OPTION_PROXY_PORT,
	CAVAN_COMMAND_OPTION_READ,
	CAVAN_COMMAND_OPTION_ROOT,
	CAVAN_COMMAND_OPTION_SIZE,
	CAVAN_COMMAND_OPTION_START,
	CAVAN_COMMAND_OPTION_STOP,
	CAVAN_COMMAND_OPTION_SUPER,
	CAVAN_COMMAND_OPTION_UNKNOWN,
	CAVAN_COMMAND_OPTION_VERBOSE,
	CAVAN_COMMAND_OPTION_VERSION,
	CAVAN_COMMAND_OPTION_WIFI_MAC,
	CAVAN_COMMAND_OPTION_WRITE,
} cavan_command_option_t;

struct cavan_command_map
{
	const char *name;
	int (*main_func)(int argc, char *argv[]);
};

#define FIND_EXEC_COMMAND(map) \
	find_and_exec_command(map, ARRAY_SIZE(map), argc, argv);

void print_command_table(const struct cavan_command_map *p, size_t size);
const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size);
void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname);
int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[]);

int cavan_redirect_stdio_base(int ttyfd, int flags);
int cavan_redirect_stdio(const char *pathname, int flags);
int cavan_exec_redirect_stdio_base(const char *ttypath, int lines, int columns, const char *command);
int cavan_exec_redirect_stdio_main(const char *command, int lines, int columns, int in_fd, int out_fd);

int cavan_tty_redirect_base(int ttyfd);
int cavan_tty_redirect(const char *ttypath);
