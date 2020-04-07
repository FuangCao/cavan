#pragma once

/*
 * File:		gpio.h
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

#define PATH_SYS_GPIO		"/sys/class/gpio"
#define FILE_GPIO_EXPORT	PATH_SYS_GPIO "/export"

int cavan_gpio_parse(const char *text);
int cavan_gpio_export(int gpio);
int cavan_gpio_open(int gpio, const char *name, int flags);
int cavan_gpio_write(int gpio, const char *name, const void *buff, int length);
int cavan_gpio_printf(int gpio, const char *name, const char *fmt, ...);
int cavan_gpio_read(int gpio, const char *name, void *buff, int size);
int cavan_gpio_direction(int gpio, const char *value);
int cavan_gpio_input(int gpio);
int cavan_gpio_output(int gpio);
int cavan_gpio_get(int gpio);
int cavan_gpio_set(int gpio, int value);
