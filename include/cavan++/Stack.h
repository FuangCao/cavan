#pragma once

/*
 * File:		Stack.h
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
#include <cavan++/Link.h>

template <typename T>
class Stack
{
private:
	T *mTop;
	T *mLast;
	T *mData;

public:
	Stack(int size)
	{
		mData = new T[size];
		if (mData == NULL)
		{
			size = 0;
		}

		mLast = mData + size - 1;
		clear();
	}

	~Stack(void)
	{
		delete []mData;
	}

	bool push(T data)
	{
		if (isFull())
		{
			return false;
		}

		*mTop++ = data;

		return true;
	}

	bool pop(T &data)
	{
		if (isEmpty())
		{
			return false;
		}

		data = *--mTop;

		return true;
	}

	bool top(T &data)
	{
		if (isEmpty())
		{
			return false;
		}

		data = *(mTop - 1);

		return true;
	}

	bool isEmpty(void)
	{
		return mTop <= mData;
	}

	bool isFull(void)
	{
		return mTop > mLast;
	}

	bool hasData(void)
	{
		return mTop > mData;
	}

	void clear(void)
	{
		mTop = mData;
	}

	int getUsedCount(void)
	{
		return mTop - mData;
	}

	int getFreeCount(void)
	{
		return mLast - mTop;
	}

	int count(void)
	{
		return getUsedCount();
	}

	int available(void)
	{
		return getFreeCount();
	}

	T *getArray(void)
	{
		return mData;
	}
};

template <typename T>
class LinkStack
{
private:
	LinkNodeT<T> *mTop;

public:
	LinkStack(void) : mTop(NULL) {}
	~LinkStack(void)
	{
		while (mTop)
		{
			LinkNodeT<T> *next = mTop->next;
			delete mTop;
			mTop = next;
		}
	}

	int push(T &data)
	{
		LinkNodeT<T> *node = new LinkNodeT<T>(data, mTop);
		if (node == NULL)
		{
			return -1;
		}

		mTop = node;

		return 0;
	}

	int pop(T &data)
	{
		if (isEmpty())
		{
			return -1;
		}

		data = mTop->data;

		LinkNodeT<T> *next = mTop->next;
		delete mTop;
		mTop = next;

		return 0;
	}

	bool isEmpty(void)
	{
		return mTop == NULL;
	}
};
