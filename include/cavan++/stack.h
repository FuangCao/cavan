#pragma once

/*
 * File:		stack.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-24 19:05:01
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
#include <cavan++/link.h>

template <typename T>
class Stack
{
private:
	T *mTop;
	T *mLast;
	T *mData;

public:
	Stack(int size);
	~Stack(void)
	{
		delete []mData;
	}

	int push(T &data);
	int pop(T &data);

	bool isEmpty(void)
	{
		return mTop <= mData;
	}

	bool isFull(void)
	{
		return mTop > mLast;
	}
};

template <typename T>
class LinkStack
{
private:
	LinkNode<T> *mTop;

public:
	LinkStack(void) : mTop(NULL) {}
	~LinkStack(void);

	int push(T &data);
	int pop(T &data);

	bool isEmpty(void)
	{
		return mTop == NULL;
	}
};
