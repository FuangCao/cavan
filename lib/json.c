/*
 * File:		json.c
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
#include <cavan/json.h>

static const struct cavan_json_type cavan_json_types[] = {
	[CAVAN_JSON_OBJECT] = { CAVAN_JSON_OBJECT, '{', '}' },
	[CAVAN_JSON_ARRAY] = { CAVAN_JSON_ARRAY, '[', ']' },
	[CAVAN_JSON_TEXT] = { CAVAN_JSON_TEXT, 0, 0 },
	[CAVAN_JSON_VALUE] = { CAVAN_JSON_VALUE, 0, 0 },
};

static void cavan_json_node_free(struct cavan_json_node *node)
{
	if (node->next != NULL) {
		cavan_json_node_free(node->next);
	}

	if (node->childs != NULL) {
		cavan_json_node_free(node->childs);
	}

	free(node);
}

static struct cavan_json_node *cavan_json_node_alloc(char *name, cavan_json_type_t type, struct cavan_json_node *parent)
{
	struct cavan_json_node *node = cavan_malloc_type(struct cavan_json_node);
	if (node == NULL) {
		return NULL;
	}

	node->type = cavan_json_types + type;
	node->name = name;
	node->value = NULL;

	node->parent = parent;
	node->childs = NULL;
	node->next = NULL;

	if (parent != NULL) {
		struct cavan_json_node *child = parent->childs;
		if (child == NULL) {
			parent->childs = node;
		} else {
			while (child->next != NULL) {
				child = child->next;
			}

			child->next = node;
		}
	}

	return node;
}

int cavan_json_document_init(struct cavan_json_document *doc)
{
	doc->memory = NULL;
	doc->nodes = NULL;
	return 0;
}

void cavan_json_document_deinit(struct cavan_json_document *doc)
{
	if (doc->nodes != NULL) {
		cavan_json_node_free(doc->nodes);
	}

	if (doc->memory != NULL) {
		free(doc->memory);
	}
}

int cavan_json_document_parse(struct cavan_json_document *doc, char *text, size_t size)
{
	cavan_json_type_t type = CAVAN_JSON_VALUE;
	struct cavan_json_node *node, *child;
	char *text_end = text + size;
	char *p, *name, *value;

	node = child = NULL;
	name = p = text;
	value = NULL;

	while (text < text_end) {
		switch (*text) {
		case ':':
			type = CAVAN_JSON_VALUE;
			*p++ = 0;
			value = p;
			text++;
			break;

		case '{':
			if (name == p) {
				name = NULL;
			}

			child = cavan_json_node_alloc(name, CAVAN_JSON_OBJECT, node);
			node = child;
			name = p;
			text++;
			break;

		case '[':
			if (name == p) {
				name = NULL;
			}

			child = cavan_json_node_alloc(name, CAVAN_JSON_ARRAY, node);
			node = child;
			name = p;
			text++;
			break;

		case '}':
		case ']':
		case ',':
			if (value != NULL) {
				*p++ = 0;
				child = cavan_json_node_alloc(name, type, node);
				child->value = value;
				value = NULL;
			}

			if (*text != ',' && node->parent != NULL) {
				node = node->parent;
			}

			name = p;
			text++;
			break;

		case '"':
			type = CAVAN_JSON_TEXT;

			while (++text < text_end) {
				char c = *text;
				if (c == '"') {
					text++;
					break;
				}

				if (c == '\\') {
					*p++ = *++text;

				} else {
					*p++ = c;
				}
			}
			break;

		case '\r':
		case '\n':
		case '\f':
		case '\t':
		case ' ':
			text++;
			break;

		default:
			*p++ = *text++;
			break;
		}
	}

	if (node != NULL) {
		while (node->parent != NULL) {
			node = node->parent;
		}

		doc->nodes = node;
	}

	return 0;
}

int cavan_json_document_parse_file(struct cavan_json_document *doc, const char *pathname)
{
	int ret;
	size_t size = 0;
	char *mem = file_read_all_text(pathname, &size);

	if (mem == NULL) {
		return -EFAULT;
	}

	ret = cavan_json_document_parse(doc, mem, size);
	if (ret < 0) {
		free(mem);
	} else {
		doc->memory = mem;
	}

	return ret;
}

static char *cavan_json_node_tostring(const struct cavan_json_node *node, char *buff, char *buff_end)
{
	const struct cavan_json_type *type = node->type;

	if (node->name != NULL) {
		buff += snprintf(buff, buff_end - buff, "\"%s\":", node->name);
	}

	if (node->childs != NULL) {
		if (type->prefix && buff < buff_end) {
			*buff++ = type->prefix;
		}

		buff = cavan_json_node_tostring(node->childs, buff, buff_end);

		if (type->suffix && buff < buff_end) {
			*buff++ = type->suffix;
		}
	} else {
		const char *value = node->value;
		if (value == NULL) {
			value = "null";
		}

		if (type->type == CAVAN_JSON_TEXT) {
			buff += snprintf(buff, buff_end - buff, "\"%s\"", value);
		} else {
			buff = text_ncopy(buff, value, buff_end - buff);
		}
	}

	if (node->next != NULL && buff < buff_end) {
		*buff++ = ',';
		buff = cavan_json_node_tostring(node->next, buff, buff_end);
	}

	return buff;
}

int cavan_json_document_tostring(const struct cavan_json_document *doc, char *buff, size_t size)
{
	const struct cavan_json_node *nodes = doc->nodes;
	char *buff_end = buff + size;
	char *p = buff;

	if (nodes != NULL) {
		p = cavan_json_node_tostring(nodes, p, buff_end);
	}

	if (p < buff_end) {
		*p = 0;
	}

	return p - buff;
}
