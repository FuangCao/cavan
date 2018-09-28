#pragma once

/*
 * File:		regex.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-09-26 11:21:36
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

#define CAVAN_RE_CMD_MAX		200
#define CAVAN_RE_DATA_MAX		4096
#define CAVAN_RE_GROUP_MAX		20

typedef enum {
	CAVAN_RE_NOOP,
	CAVAN_RE_START,
	CAVAN_RE_END,
	CAVAN_RE_HEAD,
	CAVAN_RE_TAIL,
	CAVAN_RE_TXT,
	CAVAN_RE_ANY,
	CAVAN_RE_DOT,
	CAVAN_RE_TIMES,
	CAVAN_RE_SPACE,
	CAVAN_RE_NOT_SPACE,
	CAVAN_RE_DIGIT,
	CAVAN_RE_NOT_DIGIT,
	CAVAN_RE_WORD,
	CAVAN_RE_NOT_WORD,
	CAVAN_RE_ARRAY,
	CAVAN_RE_NOT_ARRAY,
} cavan_regex_code_t;

typedef struct cavan_regex_command {
	cavan_regex_code_t code;
	u16 data;
	u16 length;
} cavan_regex_command_t;

typedef struct cavan_regex_group {
	const char *text;
	int length;
} cavan_regex_group_t;

typedef struct cavan_regex {
	cavan_regex_command_t commands[CAVAN_RE_CMD_MAX];
	char data[CAVAN_RE_DATA_MAX];
} cavan_regex_t;

typedef struct cavan_regex_matcher {
	const cavan_regex_command_t *cmd;
	const cavan_regex_command_t *cmd_end;
	struct cavan_regex_group groups[10];
	int group_count;
} cavan_regex_matcher_t;

const char *cavan_regex_code_tostring(cavan_regex_code_t code);
void cavan_regex_dump(const cavan_regex_t *regex);
int cavan_regex_compile(cavan_regex_t *regex, const char *pattern);
bool cavan_regex_match(const cavan_regex_t *regex, const char *text, int length);

static inline bool cavan_regex_match2(const cavan_regex_t *regex, const char *text)
{
	return cavan_regex_match(regex, text, strlen(text));
}
