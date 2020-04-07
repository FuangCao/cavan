/*
 * File:		Android.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-11 11:55:09
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/debug.h>

#include <utils/CallStack.h>

char *dump_backtrace(char *buff, size_t size)
{
#if 0
	android::String8 str;
	android::CallStack stack;

	stack.update(5);
	str = stack.toString();

	return text_ncopy(buff, str.string(), size);
#else
	return buff;
#endif
}

char *address_to_symbol(const void *addr, char *buff, size_t size)
{
	return buff;
}
