/*
 * File:		gpio.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2020-04-07 14:18:21
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

int cavan_gpio_parse(const char *text)
{
	int gpio = text2value_unsigned(text, &text, 10);

	if (*text == 0) {
		return gpio;
	}

	if (*text == '-' || *text == '_') {
		text++;
	}

	gpio *= 32;

	switch (*text) {
	case 0:
		return gpio;

	case 'b':
	case 'B':
		gpio += 8;
		text++;
		break;

	case 'c':
	case 'C':
		gpio += 16;
		text++;
		break;

	case 'd':
	case 'D':
		gpio += 24;
		text++;
		break;

	case 'e':
	case 'E':
		gpio += 32;
		text++;
		break;

	case 'f':
	case 'F':
		gpio += 40;
		text++;
		break;

	case 'g':
	case 'G':
		gpio += 48;
		text++;
		break;
	}

	gpio += text2value_unsigned(text, &text, 10);

	return gpio;
}

int cavan_gpio_export(int gpio)
{
	return file_printf(FILE_GPIO_EXPORT, "%d", gpio);
}

int cavan_gpio_open(int gpio, const char *name, int flags)
{
	char pathname[1024];
	int fd;

	snprintf(pathname, sizeof(pathname), PATH_SYS_GPIO "/gpio%d/%s", gpio, name);
	println("pathname = %s", pathname);

	fd = open(pathname, flags);

	if (fd < 0 && errno == ENOENT) {
		int ret = cavan_gpio_export(gpio);
		if (ret < 0) {
			pr_err_info("cavan_gpio_export: %d", ret);
			return ret;
		}

		fd = open(pathname, flags);
	}

	return fd;
}

int cavan_gpio_write(int gpio, const char *name, const void *buff, int length)
{
	int ret;
	int fd;

	fd = cavan_gpio_open(gpio, name, O_WRONLY);
	if (fd < 0) {
		pr_err_info("cavan_gpio_open: %d", fd);
		return fd;
	}

	ret = ffile_write(fd, buff, length);
	close(fd);

	return ret;
}

int cavan_gpio_printf(int gpio, const char *name, const char *fmt, ...)
{
	char buff[1024];
	int length;
	va_list ap;

	va_start(ap, fmt);
	length = vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	return cavan_gpio_write(gpio, name, buff, length);
}

int cavan_gpio_read(int gpio, const char *name, void *buff, int size)
{
	int ret;
	int fd;

	fd = cavan_gpio_open(gpio, name, O_RDONLY);
	if (fd < 0) {
		pr_err_info("cavan_gpio_open: %d", fd);
		return fd;
	}

	ret = ffile_read(fd, buff, size);
	close(fd);

	return ret;
}

int cavan_gpio_direction(int gpio, const char *value)
{
	return cavan_gpio_write(gpio, "direction", value, strlen(value));
}

int cavan_gpio_input(int gpio)
{
	return cavan_gpio_direction(gpio, "in");
}

int cavan_gpio_output(int gpio)
{
	return cavan_gpio_direction(gpio, "out");
}

int cavan_gpio_get(int gpio)
{
	char buff[1024];
	int ret;

	ret = cavan_gpio_read(gpio, "value", buff, sizeof(buff));
	if (ret < 0) {
		pr_err_info("cavan_gpio_read: %d", ret);
		return ret;
	}

	return buff[0] != '0';
}

int cavan_gpio_set(int gpio, int value)
{
	return cavan_gpio_printf(gpio, "value", "%d", value);
}
