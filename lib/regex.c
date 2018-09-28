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

	case CAVAN_RE_SPACE:
		return "SPACE";

	case CAVAN_RE_NOT_SPACE:
		return "NOT_SPACE";

	case CAVAN_RE_DIGIT:
		return "DIGIT";

	case CAVAN_RE_NOT_DIGIT:
		return "NOT_DIGIT";

	case CAVAN_RE_WORD:
		return "WORD";

	case CAVAN_RE_NOT_WORD:
		return "NOT_WORD";

	case CAVAN_RE_ARRAY:
		return "ARRAY";

	case CAVAN_RE_NOT_ARRAY:
		return "NOT_ARRAY";

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

	command += 2;
	command->code = CAVAN_RE_NOOP;

	return command;
}

static cavan_regex_command_t *cavan_regex_add_char(cavan_regex_command_t *command, char *data, u16 offset, char c)
{
	if (command->code != CAVAN_RE_TXT) {
		command++;
		command->code = CAVAN_RE_TXT;
		command->data = offset;
		command->length = 0;
	}

	command->length++;
	data[offset] = c;

	return command;
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

		case '(':
			break;

		case '[':
			command++;
			command->code = CAVAN_RE_ARRAY;
			command->data = data;

			while (1) {
				char c = *++pattern;
				char end;

				switch (c) {
				case 0:
					pr_red_info("`]' not found");
					return -EINVAL;

				case ']':
					goto label_array_found;

				case '^':
					command->code = CAVAN_RE_NOT_ARRAY;
					data = command->data;
					break;

				case '-':
					end = *++pattern;
					for (c = pattern[-2] + 1; c < end; c++) {
						regex->data[data++] = c;
					}
					break;

				case '\\':
					c = *++pattern;

					switch (c) {
					case 't':
						c = '\t';
						break;

					case 'n':
						c = '\n';
						break;

					case 'r':
						c = '\r';
						break;

					case 'f':
						c = '\f';
						break;

					case 's':
						c = 1 << 7 | CAVAN_RE_SPACE;
						break;

					case 'S':
						c = 1 << 7 | CAVAN_RE_NOT_SPACE;
						break;

					case 'd':
						c = 1 << 7 | CAVAN_RE_DIGIT;
						break;

					case 'D':
						c = 1 << 7 | CAVAN_RE_NOT_DIGIT;
						break;

					case 'w':
						c = 1 << 7 | CAVAN_RE_WORD;
						break;

					case 'W':
						c = 1 << 7 | CAVAN_RE_NOT_WORD;
						break;
					}
				default:
					regex->data[data++] = c;
				}
			}

label_array_found:
			command->length = data - command->data;
			break;

		case '\\':
			switch (*++pattern) {
			case 't':
				command = cavan_regex_add_char(command, regex->data, data, '\t');
				data++;
				break;

			case 'n':
				command = cavan_regex_add_char(command, regex->data, data, '\n');
				data++;
				break;

			case 'r':
				command = cavan_regex_add_char(command, regex->data, data, '\r');
				data++;
				break;

			case 'f':
				command = cavan_regex_add_char(command, regex->data, data, '\f');
				data++;
				break;

			case '+':
				command = cavan_regex_build_times(command, regex->data, data, 1, 0xFF);
				data += 2;
				break;

			case 's':
				command++;
				command->code = CAVAN_RE_SPACE;
				break;

			case 'S':
				command++;
				command->code = CAVAN_RE_NOT_SPACE;
				break;

			case 'd':
				command++;
				command->code = CAVAN_RE_DIGIT;
				break;

			case 'D':
				command++;
				command->code = CAVAN_RE_NOT_DIGIT;
				break;

			case 'w':
				command++;
				command->code = CAVAN_RE_WORD;
				break;

			case 'W':
				command++;
				command->code = CAVAN_RE_NOT_WORD;
				break;

			default:
				command = cavan_regex_add_char(command, regex->data, data, *pattern);
				data++;
				break;
			}
			break;

		default:
			command = cavan_regex_add_char(command, regex->data, data, *pattern);
			data++;
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
	const char *matched;
	u8 times = 0;

	while (times < min) {
		text = cavan_regex_match_base(cmd + 1, 1, data, text, text_end);
		if (text == NULL) {
			return NULL;
		}

		times++;
	}

	matched = text;

	while (times < max) {
		const char *res = cavan_regex_match_base(cmd + 1, 1, data, text, text_end);
		if (res == NULL) {
			break;
		}

		if (cavan_regex_match_base(cmd + 2, CAVAN_RE_CMD_MAX, data, res, text_end) != NULL) {
			matched = res;
		}

		text = res;
		times++;
	}

	return matched;
}

static bool cavan_regex_match_array(const char c, const char *array, u16 length)
{
	const char *end;

	for (end = array + length; array < end; array++) {
		char node = *array;

		if (node & (1 << 7)) {
			switch (node & 0x7F) {
			case CAVAN_RE_SPACE:
				if (cavan_isspace(c)) {
					return true;
				}
				break;

			case CAVAN_RE_NOT_SPACE:
				if (!cavan_isspace(c)) {
					return true;
				}
				break;

			case CAVAN_RE_DIGIT:
				if (cavan_isdigit(c)) {
					return true;
				}
				break;

			case CAVAN_RE_NOT_DIGIT:
				if (!cavan_isdigit(c)) {
					return true;
				}
				break;

			case CAVAN_RE_WORD:
				if (cavan_isletter(c)) {
					return true;
				}
				break;

			case CAVAN_RE_NOT_WORD:
				if (!cavan_isletter(c)) {
					return true;
				}
				break;
			}
		} else if (c == node) {
			return true;
		}
	}

	return false;
}

static const char *cavan_regex_match_base(const cavan_regex_command_t *cmd, int count, const char *data, const char *text, const char *text_end)
{
	const cavan_regex_command_t *cmd_end = cmd + count;
	const char *text_bak = text;

	for (cmd_end = cmd + count; cmd < cmd_end && text < text_end; cmd++) {
		println("command = %s", cavan_regex_code_tostring(cmd->code));
		println("text = %s", text);

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

			println("text = %s", text);

			cmd++;
			break;

		case CAVAN_RE_TXT:
			println("CAVAN_RE_TXT: %d", cmd->length);

			if (text_end - text < cmd->length) {
				return NULL;
			}

			if (strncmp(text, data + cmd->data, cmd->length) != 0) {
				return NULL;
			}

			text += cmd->length;
			break;

		case CAVAN_RE_SPACE:
			if (cavan_isspace(*text)) {
				text++;
				break;
			}

			return NULL;

		case CAVAN_RE_NOT_SPACE:
			if (cavan_isspace(*text)) {
				return NULL;
			}

			text++;
			break;

		case CAVAN_RE_DIGIT:
			if (cavan_isdigit(*text)) {
				text++;
				break;
			}

			return NULL;

		case CAVAN_RE_NOT_DIGIT:
			if (cavan_isdigit(*text)) {
				return NULL;
			}

			text++;
			break;

		case CAVAN_RE_WORD:
			if (cavan_isletter(*text)) {
				text++;
				break;
			}

			return NULL;

		case CAVAN_RE_NOT_WORD:
			if (cavan_isletter(*text)) {
				return NULL;
			}

			text++;
			break;

		case CAVAN_RE_ARRAY:
			if (cavan_regex_match_array(*text, data + cmd->data, cmd->length)) {
				text++;
				break;
			}

			return NULL;

		case CAVAN_RE_NOT_ARRAY:
			if (cavan_regex_match_array(*text, data + cmd->data, cmd->length)) {
				return NULL;
			}

			text++;
			break;

		default:
			pr_red_info("invalid command: %d", cmd->code);
			return NULL;
		}
	}

	while (cmd < cmd_end) {
		switch (cmd->code) {
		case CAVAN_RE_NOOP:
			break;

		case CAVAN_RE_TAIL:
		case CAVAN_RE_END:
			return text;

		default:
			return NULL;
		}

		cmd++;
	}

	return text;
}

bool cavan_regex_match(const cavan_regex_t *regex, const char *text, int length)
{
	return cavan_regex_match_base(regex->commands, CAVAN_RE_CMD_MAX, regex->data, text, text + length) != NULL;
}
