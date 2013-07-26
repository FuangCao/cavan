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
