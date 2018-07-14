#pragma once

/*
 * File:		json.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-14 14:15:09
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

typedef enum {
	CAVAN_JSON_OBJECT,
	CAVAN_JSON_ARRAY,
	CAVAN_JSON_TEXT,
	CAVAN_JSON_VALUE,
} cavan_json_type_t;

struct cavan_json_type {
	cavan_json_type_t type;
	char prefix;
	char suffix;
};

struct cavan_json_node {
	const struct cavan_json_type *type;
	const char *name;
	const char *value;
	struct cavan_json_node *next;
	struct cavan_json_node *parent;
	struct cavan_json_node *childs;
};

struct cavan_json_document {
	char *memory;
	struct cavan_json_node *nodes;
};

void cavan_json_document_free(struct cavan_json_document *doc);
struct cavan_json_document *cavan_json_document_parse(char *text, size_t size);
struct cavan_json_document *cavan_json_document_parse_file(const char *pathname);
int cavan_json_document_tostring(const struct cavan_json_document *doc, char *buff, size_t size);
const struct cavan_json_node *cavan_json_node_find(const struct cavan_json_node *node, const char *name);
const struct cavan_json_node *cavan_json_node_get_child(const struct cavan_json_node *node, int index);
const struct cavan_json_node *cavan_json_document_find(const struct cavan_json_document *doc, ...);
