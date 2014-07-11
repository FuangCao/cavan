#pragma once

/*
 * File:		pool.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-29 11:51:01
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
union PoolNode
{
	T data;
	LinkNode node;
};

template <typename T>
class DataPool : public DoubleLinkNode
{
private:
	int mFree;
	int mSize;
	Link mLink;
	void *mData;
	MutexLock mLock;
	PoolNode<T> *mNodes;

public:
	DataPool(int size) : mLink(), mLock()
	{
		mNodes = new PoolNode<T>[size];
		if (mNodes)
		{
			mSize = size;
		}
		else
		{
			mSize = 0;
		}

		for (PoolNode<T> *node = mNodes + mSize - 1; node >= mNodes; node--)
		{
			mLink.push((LinkNode *) node);
		}

		mFree = mSize;
	}

	~DataPool()
	{
		if (mNodes)
		{
			delete[] mNodes;
		}
	}

	void setData(void *data)
	{
		AutoLock lock(mLock);

		mData = data;
	}

	void *getData(void)
	{
		AutoLock lock(mLock);

		return mData;
	}

	int getFree(void)
	{
		AutoLock lock(mLock);

		return mFree;
	}

	int getSize(void)
	{
		AutoLock lock(mLock);

		return mSize;
	}

	int getUsed(void)
	{
		AutoLock lock(mLock);

		return mSize - mFree;
	}

	T *getNode(void)
	{
		AutoLock lock(mLock);

		T *data = (T *) mLink.pop();
		if (data == NULL)
		{
			return NULL;
		}

		mFree--;

		return data;
	}

	void putNode(T *data)
	{
		AutoLock lock(mLock);

		mLink.push((LinkNode *) data);
		mFree++;
	}
};

template <typename T>
struct LinkPoolNode
{
	T data;
	DataPool<LinkPoolNode<T> > *pool;
};

template <typename T>
class LinkPool
{
private:
	int mStep;
	int mNumStep;
	MutexLock mLock;
	DoubleLoopLink mFreeLink;
	DoubleLoopLink mFullLink;

	DataPool<LinkPoolNode<T> > *createPool(void)
	{
		DataPool<LinkPoolNode<T> > *pool = new DataPool<LinkPoolNode<T> >(mNumStep * mStep);
		if (pool == NULL)
		{
			return NULL;
		}

		mFreeLink.push(pool);
		mNumStep++;

		return pool;
	}

	void deletePool(DataPool<LinkPoolNode<T> > *pool)
	{
		mFreeLink.remove(pool);
		delete pool;
		mNumStep--;
	}

public:
	LinkPool(int step = 10) : mLock(), mFreeLink(), mFullLink()
	{
		if (step > 2)
		{
			mStep = step;
		}
		else
		{
			mStep = 2;
		}

		mNumStep = 1;
	}

	~LinkPool(void)
	{
		DataPool<LinkPoolNode<T> > *pool;

		while ((pool = static_cast<DataPool<LinkPoolNode<T> > *>(mFreeLink.pop())))
		{
			delete pool;
		}

		while ((pool = static_cast<DataPool<LinkPoolNode<T> > *>(mFullLink.pop())))
		{
			delete pool;
		}
	}

	T *getNode(void)
	{
		AutoLock lock(mLock);

		while (1)
		{
			DataPool<LinkPoolNode<T> > *pool = static_cast<DataPool<LinkPoolNode<T> > *>(mFreeLink.getTop());
			if (pool == NULL)
			{
				pool = createPool();
			}

			LinkPoolNode<T> *node = pool->getNode();
			if (node == NULL)
			{
				mFullLink.push(pool);
				continue;
			}

			if (pool->getFree() == 0)
			{
				mFreeLink.remove(pool);
				mFullLink.push(pool);
			}

			node->pool = pool;
			return (T *) node;
		}

		return NULL;
	}

	void putNode(T *data)
	{
		AutoLock lock(mLock);

		LinkPoolNode<T> *node = (LinkPoolNode<T> *) data;
		DataPool<LinkPoolNode<T> > *pool = node->pool;
		pool->putNode(node);
		if (pool->getFree() == 1)
		{
			mFullLink.remove(pool);
			mFreeLink.push(pool);
		}
		else if (pool->getUsed() == 0 && pool->getSize() > mStep)
		{
			deletePool(pool);
		}
	}
};
