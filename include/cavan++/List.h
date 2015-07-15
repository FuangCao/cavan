#pragma once

/*
 * File:		List.h
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
#include <cavan++/Link.h>

template <typename T>
class List
{
private:
	T *mHead;
	T *mTail;
	T *mLast;
	T *mData;
	T *mCurr;
	int mSize;

public:
	List(int size)
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

		mLast = mData + mSize - 1;

		clear();
	}

	~List(void)
	{
		if (mData)
		{
			delete []mData;
		}
	}

	void clear(void)
	{
		mHead = mTail = mData;
	}

	T *getNext(T *p)
	{
		if (p == mLast)
		{
			return mData;
		}

		return p + 1;
	}

	bool append(T data)
	{
		T *next = getNext(mTail);
		if (next == mHead)
		{
			return false;
		}

		*mTail = data;
		mTail = next;

		return true;
	}

	bool pop(T &data)
	{
		if (isEmpty())
		{
			return false;
		}

		data = *mHead;
		mHead = getNext(mHead);

		return true;
	}

	int getUsedCount(void)
	{
		if (mHead > mTail)
		{
			return (mTail - mData) + (mLast - mHead + 1);
		}

		return mTail - mHead;
	}

	int count(void)
	{
		return getUsedCount();
	}

	int getFreeCount(void)
	{
		if (mHead > mTail)
		{
			return mHead - mTail;
		}

		return (mHead - mData) + (mLast - mTail);
	}

	bool isFull(void)
	{
		return getNext(mTail) == mHead;
	}

	bool isEmpty(void)
	{
		return mHead == mTail;
	}

	static void sort(T *start, T *end, int (*compare)(T left, T right))
	{
		T *p = start;
		T *q = end;
		T middle = *p;

		while (p < q)
		{
			for (; compare(*q, middle) >= 0; q--)
			{
				if (q <= p)
				{
					goto label_found;
				}
			}

			*p = *q;

			for (; compare(*p, middle) <= 0; p++)
			{
				if (q <= p)
				{
					goto label_found;
				}
			}

			*q = *p;
		}

label_found:
		*p = middle;

		if (p - start > 1)
		{
			sort(start, p - 1, compare);
		}

		if (end - q > 1)
		{
			sort(q + 1, end, compare);
		}
	}

	void sort(int (*compare)(T left, T right))
	{
		if (mTail > mHead)
		{
			sort(mHead, mTail - 1, compare);
		}
	}

	void start(void)
	{
		mCurr = mHead;
	}

	T *next(void)
	{
		if (mCurr == mTail)
		{
			return NULL;
		}

		T *data = mCurr;
		mCurr = getNext(data);

		return data;
	}
};

template <typename T>
class LinkList
{
private:
	LinkNodeT<T> *mHead;
	LinkNodeT<T> *mTail;

public:
	LinkList(void)
	{
		mHead = mTail = NULL;
	}

	int append(T &data)
	{
		LinkNodeT<T> *node = new LinkNodeT<T>(data);
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

	int pop(T &data)
	{
		if (isEmpty())
		{
			return -1;
		}

		data = mHead->data;

		LinkNodeT<T> *next = mHead->next;
		delete mHead;
		mHead = next;

		return 0;
	}

	bool isEmpty(void)
	{
		return mHead == NULL;
	}
};
