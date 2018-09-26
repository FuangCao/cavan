/*
 * File:		regex.c
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
#include <cavan/regex.h>

const char *cavan_regex_code_tostring(cavan_regex_code_t code)
{
	switch (code) {
	case CAVAN_RE_NOOP:
		return "NOOP";

	case CAVAN_RE_START:
		return "START";

	case CAVAN_RE_END:
		return "END";

	case CAVAN_RE_HEAD:
		return "HEAD";

	case CAVAN_RE_TAIL:
		return "TAIL";

	case CAVAN_RE_TXT:
		return "TXT";

	case CAVAN_RE_ANY:
		return "ANY";

	case CAVAN_RE_DOT:
		return "DOT";

	case CAVAN_RE_TIMES:
		return "TIMES";

	case CAVAN_RE_NUM:
		return "NUM";

	case CAVAN_RE_NOT_NUM:
		return "NOT_NUM";

	case CAVAN_RE_WORD:
		return "WORD";

	case CAVAN_RE_NOT_WORD:
		return "NOT_WORD";

	case CAVAN_RE_WORDS:
		return "WORDS";

	case CAVAN_RE_NOT_WORDS:
		return "NOT_WORDS";

	default:
		return "INVALID";
	}
}

void cavan_regex_dump(const cavan_regex_t *regex)
{
	const cavan_regex_command_t *cmd = regex->commands;

	while (1) {
		println("code = %s", cavan_regex_code_tostring(cmd->code));
		println("data = %d", cmd->data);
		println("length = %d\n", cmd->length);

		if (cmd->code == CAVAN_RE_END) {
			break;
		}

		cmd++;
	}
}

static cavan_regex_command_t *cavan_regex_build_times(cavan_regex_command_t *command, char *data, u16 offset, u8 min, u8 max)
{
	if (command->code == CAVAN_RE_TXT && command->length > 1) {
		cavan_regex_command_t *next, *prev;

		next = command + 2;
		prev = command;
		command++;

		prev->length--;

		next->length = 1;
		next->code = CAVAN_RE_TXT;
		next->data = prev->data + prev->length;
	} else {
		command[1] = command[0];
	}

	command->code = CAVAN_RE_TIMES;
	command->data = offset;
	command->length = 2;
	data[offset] = min;
	data[offset + 1] = max;

	return command + 1;
}

int cavan_regex_compile(cavan_regex_t *regex, const char *pattern)
{
	cavan_regex_command_t *command = regex->commands;
	u16 data = 0;

	println("compile: %s", pattern);

	command->code = CAVAN_RE_NOOP;

	while (1) {
		switch (*pattern) {
		case 0:
			goto out_completed;

		case '.':
			command++;
			command->code = CAVAN_RE_ANY;
			break;

		case '^':
			command++;
			command->code = CAVAN_RE_HEAD;
			break;

		case '$':
			command++;
			command->code = CAVAN_RE_TAIL;
			break;

		case '*':
			command = cavan_regex_build_times(command, regex->data, data, 0, 0xFF);
			data += 2;
			break;

		case '?':
			command = cavan_regex_build_times(command, regex->data, data, 0, 1);
			data += 2;
			break;

		case '+':
			command = cavan_regex_build_times(command, regex->data, data, 1, 0xFF);
			data += 2;
			break;

		case '(':
		case ')':
			break;

		case '[':
		case ']':
			break;

		case '\\':
			break;

		default:
			if (command->code != CAVAN_RE_TXT && command->code != CAVAN_RE_WORDS) {
				command++;
				command->code = CAVAN_RE_TXT;
				command->data = data;
				command->length = 0;
			}

			regex->data[data++] = *pattern;
			command->length++;
		}

		pattern++;
	}

out_completed:
	command++;
	command->code = CAVAN_RE_END;
	return 0;
}

static const char *cavan_regex_match_base(const cavan_regex_command_t *cmd, int count, const char *data, const char *text, const char *text_end);

static const char *cavan_regex_match_times(const cavan_regex_command_t *cmd, const char *data, const char *text, const char *text_end)
{
	u8 max = data[cmd->data + 1];
	u8 min = data[cmd->data];
	u8 times = 0;

	while (times < min) {
		text = cavan_regex_match_base(cmd + 1, 1, data, text, text_end);
		if (text == NULL) {
			return NULL;
		}
	}

	while (times < max) {
		const char *res = cavan_regex_match_base(cmd + 1, 1, data, text, text_end);
		if (res == NULL) {
			break;
		}

		if (cavan_regex_match_base(cmd + 2, CAVAN_RE_CMD_MAX, data, res, text_end) == NULL) {
			break;
		}

		text = res;
	}

	return text;
}

static const char *cavan_regex_match_base(const cavan_regex_command_t *cmd, int count, const char *data, const char *text, const char *text_end)
{
	const cavan_regex_command_t *cmd_end = cmd + count;
	const char *text_bak = text;

	for (cmd_end = cmd + count; cmd < cmd_end; cmd++) {
		println("command = %s", cavan_regex_code_tostring(cmd->code));

		switch (cmd->code) {
		case CAVAN_RE_NOOP:
			break;

		case CAVAN_RE_END:
			return text;

		case CAVAN_RE_HEAD:
			if (text != text_bak) {
				return NULL;
			}
			break;

		case CAVAN_RE_TAIL:
			if (text < text_end) {
				return NULL;
			}
			break;

		case CAVAN_RE_ANY:
			if (text < text_end) {
				text++;
				break;
			}
			return NULL;

		case CAVAN_RE_TIMES:
			text = cavan_regex_match_times(cmd, data, text, text_end);
			if (text == NULL) {
				return NULL;
			}

			cmd++;
			break;

		case CAVAN_RE_TXT:
			if (text_end - text < cmd->length) {
				return NULL;
			}

			if (strncmp(text, data + cmd->data, cmd->length) != 0) {
				return NULL;
			}

			text += cmd->length;
			break;

		default:
			pr_red_info("invalid command: %d", cmd->code);
			return NULL;
		}
	}

	return text;
}

bool cavan_regex_match(const cavan_regex_t *regex, const char *text, int length)
{
	return cavan_regex_match_base(regex->commands, CAVAN_RE_CMD_MAX, regex->data, text, text + length) != NULL;
}
