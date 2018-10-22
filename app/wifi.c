/*
 * File:		wifi.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-10-22 11:47:52
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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

int main(int argc, char *argv[])
{
	FILE *fp;
	int i;

	if (argc > 1) {
		const char *pathname = argv[1];

		println("open: %s", pathname);

		fp = fopen(pathname, "w");
		if (fp == NULL) {
			pr_err_info("fopen");
			return -EFAULT;
		}
	} else {
		fp = stdout;
	}

	for (i = 0; i < 100000000; i++) {
		fprintf(fp, "%08d\n", i);
	}

	if (fp != stdout) {
		fclose(fp);
	}

	return 0;
}
