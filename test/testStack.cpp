/*
 * File:		testStack.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-25 14:25:30
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan++/Stack.h>

int main(int argc, char *argv[])
{
	Stack<int> stack1(10);

	for (int i = 0; i < 10; i++)
	{
		stack1.push(i);
	}

	for (int i = 0; i < 10; i++)
	{
		int data;

		if (stack1.pop(data) >= 0)
		{
			cout << "data = " << data << endl;
		}
	}

	return 0;
}
