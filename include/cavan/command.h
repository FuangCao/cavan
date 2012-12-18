#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <sys/wait.h>

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

int cavan_exec_redirect_stdio_base(const char *ttypath, const char *command);
int cavan_exec_redirect_stdio_main(const char *command, int in_fd, int out_fd);
