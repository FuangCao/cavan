/*
 * File:		huamobile.h
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-14
 * Description:	HUAMOBILE LIBRARY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <linux/limits.h>

#include <huamobile/text.h>
#include <huamobile/file.h>
#include <huamobile/input.h>

#define BYTE_IS_LF(b) \
	((b) == '\r' || (b) == '\n')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t')

#define ARRAY_SIZE(a) \
	(sizeof(a) / sizeof((a)[0]))

#ifndef NELEM
#define NELEM(a) \
	ARRAY_SIZE(a)
#endif

#define huamobile_printf	printf

#define pr_std_info(fmt, args ...) \
		huamobile_printf(fmt "\n", ##args)

#define pr_pos_info() \
		pr_std_info("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

#define pr_red_info(fmt, args ...) \
		pr_std_info("\033[31m" fmt "\033[0m", ##args)

#define pr_green_info(fmt, args ...) \
		pr_std_info("\033[32m" fmt "\033[0m", ##args)

#define pr_bold_info(fmt, args ...) \
		pr_std_info("\033[1m" fmt "\033[0m", ##args)

#define pr_error_info(fmt, args ...) \
		if (errno) { \
			pr_red_info("%s[%d] (" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
		} else { \
			pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
		}

int huamobile_msleep(useconds_t ms);
int huamobile_ssleep(useconds_t ss);
