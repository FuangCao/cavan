/*
 * File:		dic_generator.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-27 19:43:21
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/command.h>
#include <cavan/network.h>

static int dic_generator_rand(int argc, char *argv[])
{
	int count;
	int range;
	int max, min;

	assert(argc > 3);

	min = text2value_unsigned(argv[1], NULL, 10);
	max = text2value_unsigned(argv[2], NULL, 10);
	count = text2value_unsigned(argv[3], NULL, 10);

	println("min = %d", min);
	println("max = %d", max);
	println("count = %d", count);

	range = max - min + 1;

	while (count > 0) {
		int length = min + get_rand_value() % range;
		char buff[length + 1];

		for (int i = 0; i < length; i++) {
			buff[i] = 33 + get_rand_value() % 94;
		}

		buff[length] = 0;
		puts(buff);
		count--;
	}

	return 0;
}

static void dic_dic_generator_phone_one(char *line)
{
	int i;

	for (i = 0; i < 10000; i++) {
		sprintf(line + 7, "%04d", i);
		puts(line);
	}
}

static int dic_generator_phone(int argc, char *argv[])
{
	FILE *fp;

	assert(argc > 1);

	fp = fopen(argv[1], "rb");
	if (fp != NULL) {
		while (1) {
			char line[16];

			if (fgets(line, sizeof(line), fp) == NULL) {
				break;
			}

			dic_dic_generator_phone_one(line);
		}

		fclose(fp);
	} else {
		char line[16];

		strncpy(line, argv[1], sizeof(line));
		dic_dic_generator_phone_one(line);
	}

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "rand", dic_generator_rand },
	{ "phone", dic_generator_phone },
} CAVAN_COMMAND_MAP_END;
