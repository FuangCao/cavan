/*
 * File:		List.cpp
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
#include <cavan++/list.h>

template <typename T>
List<T>::List(int size)
{
	mData = new T[size];
	if (mData == NULL)
	{
		mSize = 0;
	}
	else
	{
		mSize = size;
	}

	mHead = mTail = mData;
	mLast = mData + mSize - 1;
}

template <typename T>
List<T>::~List(void)
{
	if (mData)
	{
		delete []mData;
	}
}

template <typename T>
T *List<T>::getNext(T *p)
{
	if (p == mLast)
	{
		return mData;
	}

	return p + 1;
}

template <typename T>
int List<T>::append(T &data)
{
	T *next = getNext(mTail);
	if (next == mHead)
	{
		return -1;
	}

	*mTail = data;
	mTail = next;

	return 0;
}

template <typename T>
int List<T>::pop(T &data)
{
	if (isEmpty())
	{
		return -1;
	}

	data = *mHead;
	mHead = getNext(mHead);

	return 0;
}

template <typename T>
int List<T>::getUsedCount(void)
{
	if (mHead > mTail)
	{
		return (mTail - mData) + (mLast - mHead + 1);
	}

	return mTail - mHead;
}

template <typename T>
int List<T>::getFreeCount(void)
{
	if (mHead > mTail)
	{
		return mHead - mTail;
	}

	return (mHead - mData) + (mLast - mTail);
}

template <typename T>
int LinkList<T>::append(T &data)
{
	LinkNode<T> *node = new LinkNode<T>(data);
	if (node == NULL)
	{
		return -1;
	}

	if (mTail)
	{
		mTail->next = node;
	}
	else
	{
		mHead = node;
	}

	mTail = node;

	return 0;
}

template <typename T>
int LinkList<T>::pop(T &data)
{
	if (isEmpty())
	{
		return -1;
	}

	data = mHead->data;

	LinkNode<T> *next = mHead->next;
	delete mHead;
	mHead = next;

	return 0;
}
