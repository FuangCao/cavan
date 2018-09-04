/*
 * File:		test_json.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-14 18:31:43
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
#include <cavan/json.h>

#define DUMP_MEM_SIZE		MB(1)

int main(int argc, char *argv[])
{
	struct cavan_json_document *doc;
	char *mem;

	assert(argc > 1);

	doc = cavan_json_document_parse_file(argv[1]);
	if (doc == NULL) {
		return -ENOENT;
	}

	mem = malloc(DUMP_MEM_SIZE);
	if (mem != NULL) {
		int length = cavan_json_document_tostring(doc, mem, DUMP_MEM_SIZE, true);
		print_ntext(mem, length);
		print_char('\n');
	}

	cavan_json_document_free(doc);

	return 0;
}
