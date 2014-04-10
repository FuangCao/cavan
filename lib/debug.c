/*
 * File:			debug.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-04-10 09:28:26
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

#include <cavan.h>
#include <execinfo.h>

char *dump_backtrace(char *buff, size_t size)
{
	int i, nptrs;
	void *ptrs[100];
	char **strings;
	char *buff_end = buff + size;

	nptrs = backtrace(ptrs, NELEM(ptrs));
	strings = backtrace_symbols(ptrs, nptrs);
	if (strings == NULL)
	{
		pr_error_info("backtrace_symbols");
		return NULL;
	}

	buff += snprintf(buff, buff_end - buff, "backtrace() returned %d addresses\n", nptrs);

	for (i = 0; i < nptrs && buff < buff_end; i++)
	{
		buff += snprintf(buff, buff_end - buff, "%s\n", strings[i]);
	}

	free(strings);

	return buff;
}

int dump_stack(int (*print_func)(const char *, ...))
{
	char buff[4096];

	if (print_func == NULL)
	{
		print_func = printf;
	}

	if (dump_backtrace(buff, sizeof(buff))== NULL)
	{
		return -EFAULT;
	}

	print_func("%s", buff);

	return 0;
}

char *address_to_symbol(void *addr, char *buff, size_t size)
{
	char **strings;
	void *ptrs[] = {addr};

	strings = backtrace_symbols(ptrs, 1);
	if (strings == NULL)
	{
		pr_error_info("backtrace_symbols");
		return NULL;
	}

	strncpy(buff, strings[0], size);

	free(strings);

	return buff;
}
