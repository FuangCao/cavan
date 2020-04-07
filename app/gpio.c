#define CAVAN_CMD_NAME gpio

/*
 * File:		gpio.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2020-04-07 14:18:39
 *
 * Copyright (c) 2020 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/gpio.h>
#include <cavan/command.h>

static int main_gpio_export(int argc, char *argv[])
{
	assert(argc > 1);
	return cavan_gpio_export(cavan_gpio_parse(argv[1]));
}

static int main_gpio_input(int argc, char *argv[])
{
	assert(argc > 1);
	return cavan_gpio_input(cavan_gpio_parse(argv[1]));
}

static int main_gpio_output(int argc, char *argv[])
{
	assert(argc > 1);
	return cavan_gpio_output(cavan_gpio_parse(argv[1]));
}

static int main_gpio_get(int argc, char *argv[])
{
	int ret;

	assert(argc > 1);

	ret = cavan_gpio_get(cavan_gpio_parse(argv[1]));
	if (ret < 0) {
		pr_err_info("cavan_gpio_get: %d", ret);
		return ret;
	}

	println("%d", ret);

	return 0;
}

static int main_gpio_set(int argc, char *argv[])
{
	assert(argc > 2);
	return cavan_gpio_set(cavan_gpio_parse(argv[1]), atoi(argv[2]));
}

static int main_gpio_poll(int argc, char *argv[])
{
	int delay;
	int gpio;

	assert(argc > 1);

	gpio = cavan_gpio_parse(argv[1]);

	if (argc > 2) {
		delay = atoi(argv[2]);
	} else {
		delay = 500;
	}

	while (1) {
		int ret = cavan_gpio_get(gpio);
		if (ret < 0) {
			pr_err_info("cavan_gpio_get: %d", ret);
			break;
		}

		println("%d", ret);
		msleep(delay);
	}

	return 0;
}

static int main_gpio_input_get(int argc, char *argv[])
{
	int gpio;
	int ret;

	assert(argc > 1);

	gpio = cavan_gpio_parse(argv[1]);

	ret = cavan_gpio_input(gpio);
	if (ret < 0) {
		pr_err_info("cavan_gpio_input: %d", ret);
		return ret;
	}

	ret = cavan_gpio_get(gpio);
	if (ret < 0) {
		pr_err_info("cavan_gpio_get: %d", ret);
		return ret;
	}

	println("%d", ret);

	return 0;
}

static int main_gpio_input_poll(int argc, char *argv[])
{
	int delay;
	int gpio;
	int ret;

	assert(argc > 1);

	gpio = cavan_gpio_parse(argv[1]);

	if (argc > 2) {
		delay = atoi(argv[2]);
	} else {
		delay = 500;
	}

	ret = cavan_gpio_input(gpio);
	if (ret < 0) {
		pr_err_info("cavan_gpio_input: %d", ret);
		return ret;
	}

	while (1) {
		ret = cavan_gpio_get(gpio);
		if (ret < 0) {
			pr_err_info("cavan_gpio_get: %d", ret);
			return ret;
		}

		println("%d", ret);
		msleep(delay);
	}

	return 0;
}

static int main_gpio_output_set(int argc, char *argv[])
{
	int gpio;
	int ret;

	assert(argc > 2);

	gpio = cavan_gpio_parse(argv[1]);

	ret = cavan_gpio_output(gpio);
	if (ret < 0) {
		pr_err_info("cavan_gpio_input: %d", ret);
		return ret;
	}

	ret = cavan_gpio_set(gpio, atoi(argv[2]));
	if (ret < 0) {
		pr_err_info("cavan_gpio_get: %d", ret);
		return ret;
	}

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "exp", main_gpio_export },
	{ "in", main_gpio_input },
	{ "out", main_gpio_output },
	{ "get", main_gpio_get },
	{ "set", main_gpio_set },
	{ "poll", main_gpio_poll },
	{ "in-get", main_gpio_input_get },
	{ "in-poll", main_gpio_input_poll },
	{ "out-set", main_gpio_output_set },
} CAVAN_COMMAND_MAP_END;
