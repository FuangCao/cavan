#pragma once

/*
 * File:		list.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-24 19:05:22
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
class List
{
private:
	T *mHead;
	T *mTail;
	T *mLast;
	T *mData;
	int mSize;

public:
	List(int size);
	~List(void);

	T *getNext(T *p);
	int append(T &data);
	int pop(T &data);
	int getUsedCount(void);
	int getFreeCount(void);

	bool isFull(void)
	{
		return getNext(mTail) == mHead;
	}

	bool isEmpty(void)
	{
		return mHead == mTail;
	}
};

template <typename T>
class LinkList
{
private:
	LinkNode<T> *mHead;
	LinkNode<T> *mTail;

public:
	LinkList(void)
	{
		mHead = mTail = NULL;
	}

	int append(T &data);
	int pop(T &data);

	bool isEmpty(void)
	{
		return mHead == NULL;
	}
};
