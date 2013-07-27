#pragma once

/*
 * File:		link.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-24 19:06:36
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
#include <cavan++/lock.h>

template <typename T>
struct LinkNode
{
	T data;
	LinkNode *next;

	LinkNode(T data, LinkNode *next = NULL)
	{
		this->data = data;
		this->next = next;
	}

	bool operator>(LinkNode &node)
	{
		return data > node.data;
	}

	bool operator<(LinkNode &node)
	{
		return data < node.data;
	}

	bool operator==(LinkNode &node)
	{
		return data == node.data;
	}

	bool operator!=(LinkNode &node)
	{
		return data != node.data;
	}

	LinkNode &operator=(LinkNode &node)
	{
		data = node.data;
		return *this;
	}
};

// ================================================================================

template <typename T>
struct DoubleLinkNode
{
	T data;
	DoubleLinkNode *prev;
	DoubleLinkNode *next;

	DoubleLinkNode(T data, DoubleLinkNode *next = NULL, DoubleLinkNode *prev = NULL)
	{
		this->data = data;
		this->prev = prev;
		this->next = next;
	}

	bool operator>(DoubleLinkNode &node)
	{
		return data > node.data;
	}

	bool operator<(DoubleLinkNode &node)
	{
		return data < node.data;
	}

	bool operator==(DoubleLinkNode &node)
	{
		return data == node.data;
	}

	bool operator!=(DoubleLinkNode &node)
	{
		return data != node.data;
	}

	DoubleLinkNode &operator=(DoubleLinkNode &node)
	{
		data = node.data;
		return *this;
	}
};

// ================================================================================

template <typename T>
class Link
{
private:
	LinkNode<T> *mHead;
	MutexLock mLock;

	LinkNode<T> *getLastNode(void)
	{
		if (mHead == NULL)
		{
			return NULL;
		}

		LinkNode<T> *node;

		for (node = mHead; node->next; node = node->next);

		return node;
	}

	LinkNode<T> *findPrevNode(LinkNode<T> *node)
	{
		LinkNode<T> *prev;

		for (prev = mHead; prev->next != node; prev = prev->next);

		return prev;
	}

	void insertBase(LinkNode<T> *prev, LinkNode<T> *node)
	{
		node->next = prev->next;
		prev->next = node;
	}

public:
	Link(LinkNode<T> *head = NULL) : mHead(head), mLock() {}

	void insert(LinkNode<T> *prev, LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(prev, node);
	}

	void append(LinkNode<T> *node)
	{
		node->next = NULL;

		AutoLock lock(mLock);

		LinkNode<T> *prev = getLastNode();
		if (prev == NULL)
		{
			mHead = node;
		}
		else
		{
			prev->next = node;
		}
	}

	void push(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		node->next = mHead;
		mHead = node;
	}

	void remove(LinkNode<T> *prev, LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		prev->next = node->next;
	}

	void remove(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (node == mHead)
		{
			mHead = mHead->next;
		}
		else
		{
			LinkNode<T> *prev = findPrevNode(node);
			if (prev)
			{
				prev->next = node->next;
			}
		}
	}

	LinkNode<T> *pop(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		LinkNode<T> *node = mHead;
		mHead = mHead->next;

		return node;
	}

	void traversal(void (*handler)(LinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		for (LinkNode<T> *node = mHead; node; node = node->next)
		{
			handler(node, data);
		}
	}

	LinkNode<T> *find(bool (*matcher)(LinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		for (LinkNode<T> *node = mHead; node; node = node->next)
		{
			if (matcher(node, data))
			{
				return node;
			}
		}

		return NULL;
	}

	LinkNode<T> *find(T data)
	{
		AutoLock lock(mLock);

		for (LinkNode<T> *node = mHead; node; node = node->next)
		{
			if (node->data == data)
			{
				return node;
			}
		}

		return NULL;
	}

	LinkNode<T> *find(T *data)
	{
		AutoLock lock(mLock);

		for (LinkNode<T> *node = mHead; node; node = node->next)
		{
			if (node->data == *data)
			{
				return node;
			}
		}

		return NULL;
	}
};

// ================================================================================

template <typename T>
class DoubleLink
{
private:
	DoubleLinkNode<T> *mHead;
	MutexLock mLock;

	void insertBase(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		node->prev = prev;
		node->next = next;

		if (prev)
		{
			prev->next = node;
		}

		if (next)
		{
			next->prev = node;
		}
	}

	DoubleLinkNode<T> *getLastNode(void)
	{
		if (mHead == NULL)
		{
			return NULL;
		}

		DoubleLinkNode<T> *node;

		for (node = mHead; node->next; node = node->next);

		return node;
	}

public:
	DoubleLink(DoubleLinkNode<T> *head = NULL) : mHead(head), mLock() {}

	void insert(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(prev, next, node);
	}

	void append(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(prev, prev->next, node);
	}

	void append(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		node->next = NULL;

		DoubleLinkNode<T> *prev = getLastNode();
		insertBase(prev, NULL, node);

		if (prev == NULL)
		{
			mHead = node;
		}
	}

	void push(DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(next->prev, next, node);
	}

	void push(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(NULL, mHead, node);
		mHead = node;
	}

	void traversal(void (*handler)(DoubleLinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		for (DoubleLinkNode<T> *node = mHead; node; node = node->next)
		{
			handler(node, data);
		}
	}

	DoubleLinkNode<T> *find(bool (*matcher)(DoubleLinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		for (DoubleLinkNode<T> *node = mHead; node; node = node->next)
		{
			if (matcher(node, data))
			{
				return node;
			}
		}

		return NULL;
	}

	DoubleLinkNode<T> *find(T data)
	{
		AutoLock lock(mLock);

		for (DoubleLinkNode<T> *node = mHead; node; node = node->next)
		{
			if (node->data == data)
			{
				return node;
			}
		}

		return NULL;
	}

	DoubleLinkNode<T> *find(T *data)
	{
		AutoLock lock(mLock);

		for (DoubleLinkNode<T> *node = mHead; node; node = node->next)
		{
			if (node->data == *data)
			{
				return node;
			}
		}

		return NULL;
	}
};

// ================================================================================

template <typename T>
class LoopLink
{
private:
	LinkNode<T> *mTail;
	MutexLock mLock;

	void insertBase(LinkNode<T> *prev, LinkNode<T> *node)
	{
		if (mTail == NULL)
		{
			node->next = node;
			mTail = node;
		}
		else
		{
			node->next = prev->next;
			prev->next = node;
		}
	}

	void removeBase(LinkNode<T> *prev, LinkNode<T> *node)
	{
		if (node == mTail)
		{
			if (prev == node)
			{
				mTail = NULL;
			}
			else
			{
				mTail = prev;
			}
		}

		prev->next = node->next;
		node->next = node;
	}

	LinkNode<T> *findPrevNode(LinkNode<T> *node)
	{
		LinkNode<T> *prev;

		for (prev = node->next; prev->next != node; prev = prev->next)
		{
			if (prev == node)
			{
				return NULL;
			}
		}

		return prev;
	}

public:
	LoopLink(LinkNode<T> *tail = NULL) : mTail(tail), mLock() {}

	void insert(LinkNode<T> *prev, LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(prev, node);
	}

	void remove(LinkNode<T> *prev, LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		removeBase(prev, node);
	}

	void remove(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		LinkNode<T> *prev = findPrevNode(node);
		if (prev)
		{
			removeBase(prev, node);
		}
	}

	void append(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(mTail, node);
		mTail = node;
	}

	void push(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(mTail, node);
	}

	LinkNode<T> *pop(void)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return NULL;
		}

		LinkNode<T> *node = mTail->next;
		removeBase(mTail, node);

		return node;
	}

	LinkNode<T> *getTopNode(void)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return NULL;
		}

		return mTail->next;
	}

	void setTop(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		mTail = findPrevNode(node);
	}

	void setTail(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		mTail = node;
	}

	LinkNode<T> *getTailNode(void)
	{
		AutoLock lock(mLock);

		return mTail;
	}

	bool hasNode(LinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return false;
		}

		for (LinkNode<T> *head = mTail->next;; head = head->next)
		{
			if (head == node)
			{
				return true;
			}

			if (head == mTail)
			{
				break;
			}
		}

		return false;
	}

	void traversal(void (*handler)(LinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return;
		}

		for (LinkNode<T> *node = mTail->next;; node = node->next)
		{
			handler(node, data);

			if (node == mTail)
			{
				break;
			}
		}
	}

	LinkNode<T> *find(bool (*matcher)(LinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return NULL;
		}

		for (LinkNode<T> *node = mTail->next;; node = node->next)
		{
			if (matcher(node, data))
			{
				return node;
			}

			if (node == mTail)
			{
				break;
			}
		}

		return NULL;
	}

	LinkNode<T> *find(T data)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return NULL;
		}

		for (LinkNode<T> *node = mTail->next;; node = node->next)
		{
			if (node->data == data)
			{
				return node;
			}

			if (node == mTail)
			{
				break;
			}
		}

		return NULL;
	}

	void dump(void)
	{
		AutoLock lock(mLock);

		if (mTail == NULL)
		{
			return;
		}

		int i = 0;

		for (LinkNode<T> *node = mTail->next;; node = node->next, i++)
		{
			cout << "data[" << i << "] = " << node->data << endl;

			if (node == mTail)
			{
				break;
			}
		}
	}
};

// ================================================================================

template <typename T>
class DoubleLoopLink
{
private:
	DoubleLinkNode<T> *mHead;
	MutexLock mLock;

	void insertOnly(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		node->prev = prev;
		node->next = next;

		prev->next = node;
		next->prev = node;
	}

	void insertBase(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		if (mHead == NULL)
		{
			node->prev = node->next = node;
			mHead = node;
		}
		else
		{
			insertOnly(prev, next, node);
		}
	}

	void removeOnly(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next)
	{
		prev->next = next;
		next->prev = prev;
	}

	void removeBase(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		removeOnly(prev, next);
		node->next = node->prev = node;

		if (mHead == node)
		{
			if (node == next)
			{
				mHead = NULL;
			}
			else
			{
				mHead = next;
			}
		}
	}

	void removeBase(DoubleLinkNode<T> *node)
	{
		removeBase(node->prev, node->next, node);
	}

	void moveBase(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *next, DoubleLinkNode<T> *node)
	{
		removeOnly(node->prev, node->next);
		insertOnly(prev, next, node);
	}

public:
	DoubleLoopLink(DoubleLinkNode<T> *head = NULL) : mHead(head), mLock() {}

	void insert(DoubleLinkNode<T> *prev, DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		insertBase(prev, prev->next, node);
	}

	void remove(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		removeBase(node->prev, node->next, node);
	}

	void append(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (mHead)
		{
			insertBase(mHead->prev, mHead, node);
		}
		else
		{
			insertBase(NULL, NULL, node);
		}
	}

	void push(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (mHead)
		{
			insertBase(mHead->prev, mHead, node);
			mHead = node;
		}
		else
		{
			insertBase(NULL, NULL, node);
		}
	}

	DoubleLinkNode<T> *pop(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		DoubleLinkNode<T> *node = mHead;
		removeBase(node);

		return node;
	}

	DoubleLinkNode<T> *delTail(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		DoubleLinkNode<T> *node = mHead->prev;
		removeBase(node);

		return node;
	}

	DoubleLinkNode<T> *getHeadNode(void)
	{
		AutoLock lock(mLock);

		return mHead;
	}

	DoubleLinkNode<T> *getTailNode(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		return mHead->prev;
	}

	void traversal(void (*handler)(DoubleLinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return;
		}

		for (DoubleLinkNode<T> *node = mHead, *tail = node->prev;; node = node->next)
		{
			handler(node, data);

			if (node == tail)
			{
				break;
			}
		}
	}

	DoubleLinkNode<T> *find(bool (*matcher)(DoubleLinkNode<T> *node, void *data), void *data)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		for (DoubleLinkNode<T> *node = mHead, *tail = node->prev;; node = node->next)
		{
			if (matcher(node, data))
			{
				return node;
			}

			if (node == tail)
			{
				break;
			}
		}

		return NULL;
	}

	DoubleLinkNode<T> *find(T data)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return NULL;
		}

		for (DoubleLinkNode<T> *node = mHead, *tail = node->prev;; node = node->next)
		{
			if (node->data == data)
			{
				return node;
			}

			if (node == tail)
			{
				break;
			}
		}

		return NULL;
	}

	int getCount(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return 0;
		}

		int count = 0;

		for (DoubleLinkNode<T> *node = mHead, *tail = node->prev;; node = node->next)
		{
			count++;

			if (node == tail)
			{
				break;
			}
		}

		return count;
	}

	void setTop(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		mHead = node;
	}

	void setTail(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		mHead = node->next;
	}

	void moveToTop(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (mHead && node != mHead)
		{
			if (mHead->prev != node)
			{
				moveBase(mHead->prev, mHead, node);
			}

			mHead = node;
		}
	}

	void moveToTail(DoubleLinkNode<T> *node)
	{
		AutoLock lock(mLock);

		if (mHead && node != mHead->prev)
		{
			if (node == mHead)
			{
				mHead = node->next;
			}
			else
			{
				moveBase(mHead->prev, mHead, node);
			}
		}
	}

	void dump(void)
	{
		AutoLock lock(mLock);

		if (mHead == NULL)
		{
			return;
		}

		int i = 0;

		for (DoubleLinkNode<T> *node = mHead, *tail = node->prev;; node = node->next, i++)
		{
			cout << "data[" << i << "] = " << node->data << endl;

			if (node == tail)
			{
				break;
			}
		}
	}
};
