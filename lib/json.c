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

void cavan_json_document_free(struct cavan_json_document *doc)
{
	if (doc->nodes != NULL) {
		cavan_json_node_free(doc->nodes);
	}

	if (doc->memory != NULL) {
		free(doc->memory);
	}

	free(doc);
}

struct cavan_json_document *cavan_json_document_parse(char *text, size_t size)
{
	struct cavan_json_node *node, *child;
	struct cavan_json_document *doc;
	char *text_end = text + size;
	cavan_json_type_t type;
	char *p, *name, *value;

	doc = cavan_malloc_type(struct cavan_json_document);
	if (doc == NULL) {
		return NULL;
	}

	doc->nodes = NULL;
	node = child = NULL;
	type = CAVAN_JSON_VALUE;
	name = p = text;
	value = NULL;

	while (text < text_end) {
		switch (*text) {
		case ':':
			type = CAVAN_JSON_VALUE;
			*p++ = 0;
			value = p;
			break;

		case '{':
		case '[':
			if (name == p) {
				name = NULL;
			}

			if (*text == '{') {
				type = CAVAN_JSON_OBJECT;
			} else {
				type = CAVAN_JSON_ARRAY;
			}

			child = cavan_json_node_alloc(name, type, node);
			node = child;
			name = p;
			break;

		case '}':
		case ']':
		case ',':
			if (p > name && value != NULL) {
				*p++ = 0;
				child = cavan_json_node_alloc(name, type, node);
				child->value = value;
				value = NULL;
			}

			if (*text != ',' && node->parent != NULL) {
				node = node->parent;
			}

			name = p;
			break;

		case '"':
			type = CAVAN_JSON_TEXT;

			while (++text < text_end) {
				if (*text == '"') {
					break;
				}

				if (*text == '\\') {
					text++;
				}

				*p++ = *text;
			}
			break;

		case ' ':
		case '\r':
		case '\n':
		case '\f':
		case '\t':
			break;

		default:
			*p++ = *text;
			break;
		}

		text++;
	}

	while (node != NULL) {
		doc->nodes = node;
		node = node->parent;
	}

	doc->memory = NULL;

	return doc;
}

struct cavan_json_document *cavan_json_document_parse_file(const char *pathname)
{
	size_t size = 0;
	struct cavan_json_document *doc;
	char *mem = file_read_all_text(pathname, &size);

	if (mem == NULL) {
		return NULL;
	}

	doc = cavan_json_document_parse(mem, size);
	if (doc == NULL) {
		free(mem);
	} else {
		doc->memory = mem;
	}

	return doc;
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

static char *cavan_json_node_append_line(char *buff, char *buff_end, int level)
{
	if (buff < buff_end) {
		*buff++ = '\n';
	}

	while (level > 0 && buff < buff_end) {
		*buff++ = '\t';
		level--;
	}

	return buff;
}

static char *cavan_json_node_tostring_human(const struct cavan_json_node *node, char *buff, char *buff_end, int level)
{
	const struct cavan_json_type *type = node->type;

	if (node->name != NULL) {
		buff += snprintf(buff, buff_end - buff, "\"%s\":", node->name);
	}

	if (node->childs != NULL) {
		if (type->prefix) {
			if (node->name != NULL && buff < buff_end) {
				buff = cavan_json_node_append_line(buff, buff_end, level);
			}

			if (buff < buff_end) {
				*buff++ = type->prefix;
			}

			buff = cavan_json_node_append_line(buff, buff_end, level + 1);
		}

		buff = cavan_json_node_tostring_human(node->childs, buff, buff_end, level + 1);

		if (type->suffix) {
			buff = cavan_json_node_append_line(buff, buff_end, level);

			if (buff < buff_end) {
				*buff++ = type->suffix;
			}
		}
	} else {
		const char *value = node->value;
		if (value == NULL) {
			value = "null";
		}

		if (buff < buff_end) {
			*buff++ = ' ';
		}

		if (type->type == CAVAN_JSON_TEXT) {
			buff += snprintf(buff, buff_end - buff, "\"%s\"", value);
		} else {
			buff = text_ncopy(buff, value, buff_end - buff);
		}
	}

	if (node->next != NULL && buff + 1 < buff_end) {
		*buff++ = ',';
		buff = cavan_json_node_append_line(buff, buff_end, level);
		buff = cavan_json_node_tostring_human(node->next, buff, buff_end, level);
	}

	return buff;
}

int cavan_json_document_tostring(const struct cavan_json_document *doc, char *buff, size_t size, bool human)
{
	const struct cavan_json_node *nodes = doc->nodes;
	char *buff_end = buff + size;
	char *p = buff;

	if (nodes != NULL) {
		if (human) {
			p = cavan_json_node_tostring_human(nodes, p, buff_end, 0);
		} else {
			p = cavan_json_node_tostring(nodes, p, buff_end);
		}
	}

	if (p < buff_end) {
		*p = 0;
	}

	return p - buff;
}

const struct cavan_json_node *cavan_json_node_find(const struct cavan_json_node *node, const char *name)
{
	const struct cavan_json_node *child = node->childs;

	while (child != NULL) {
		if (child->name != NULL && strcmp(child->name, name) == 0) {
			break;
		}

		child = child->next;
	}

	return child;
}

const struct cavan_json_node *cavan_json_node_get_child(const struct cavan_json_node *node, int index)
{
	const struct cavan_json_node *child = node->childs;

	while (index > 0 && child != NULL) {
		child = child->next;
		index--;
	}

	return child;
}

const struct cavan_json_node *cavan_json_document_find(const struct cavan_json_document *doc, ...)
{
	const struct cavan_json_node *node = doc->nodes;
	va_list ap;

	va_start(ap, doc);

	while (node != NULL) {
		const char *name = va_arg(ap, const char *);
		if (name == NULL) {
			break;
		}

		while (node->type->type == CAVAN_JSON_ARRAY && node->childs != NULL) {
			node = node->childs;
		}

		node = cavan_json_node_find(node, name);
	}

	va_end(ap);

	return node;
}
