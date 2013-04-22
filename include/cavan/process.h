#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu May 17 14:54:59 CST 2012
 */

#include <cavan.h>
#include <pthread.h>

ssize_t process_get_cmdline_simple1(pid_t pid, char *buff, size_t size);
ssize_t process_get_cmdline_simple2(const char *pid, char *buff, size_t size);
char *process_get_cmdline_base(const char *filename, char *buff, size_t size);
char *process_get_cmdline1(pid_t pid, char *buff, size_t size);
char *process_get_cmdline2(const char *pid, char *buff, size_t size);
pid_t process_find_by_cmdline(const char *proc_path, const char *cmdline);
