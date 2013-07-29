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

struct LinkNode
{
	LinkNode *next;
};

template <typename T>
struct LinkNodeT : public LinkNode
{
	T data;

	LinkNodeT(void) {}
	LinkNodeT(T data) : data(data) {}
};

// ================================================================================

struct DoubleLinkNode
{
	DoubleLinkNode *prev, *next;
};

template <typename T>
struct DoubleLinkNodeT : public DoubleLinkNode
{
	T data;

	DoubleLinkNodeT(void) {}
	DoubleLinkNodeT(T data) : data(data) {}
};

// ================================================================================

class Link
{
protected:
	LinkNode *mHead;
	MutexLock mLock;

	LinkNode *getLastNode(void);
	LinkNode *findPrevNode(LinkNode *node);
	void insertBase(LinkNode *prev, LinkNode *node);

public:
	Link(LinkNode *head = NULL) : mHead(head), mLock() {}
	void insert(LinkNode *prev, LinkNode *node);
	void append(LinkNode *node);
	void push(LinkNode *node);
	void remove(LinkNode *prev, LinkNode *node);
	void remove(LinkNode *node);
	LinkNode *pop(void);
	void traversal(void (*handler)(LinkNode *node, void *data), void *data);
	LinkNode *find(bool (*matcher)(LinkNode *node, void *data), void *data);

	bool isEmpty(void)
	{
		AutoLock lock(mLock);

		return mHead == NULL;
	}

	bool hasNode(void)
	{
		AutoLock lock(mLock);

		return mHead != NULL;
	}
};

// ================================================================================

class DoubleLink
{
private:
	DoubleLinkNode *mHead;
	MutexLock mLock;

	void insertBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);
	DoubleLinkNode *getLastNode(void);

public:
	DoubleLink(DoubleLinkNode *head = NULL) : mHead(head), mLock() {}
	void insert(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);
	void append(DoubleLinkNode *prev, DoubleLinkNode *node);
	void append(DoubleLinkNode *node);
	void push(DoubleLinkNode *next, DoubleLinkNode *node);
	void push(DoubleLinkNode *node);
	void traversal(void (*handler)(DoubleLinkNode *node, void *data), void *data);
	DoubleLinkNode *find(bool (*matcher)(DoubleLinkNode *node, void *data), void *data);
};

// ================================================================================

class LoopLink
{
private:
	LinkNode *mTail;
	MutexLock mLock;

	void insertBase(LinkNode *prev, LinkNode *node);
	void removeBase(LinkNode *prev, LinkNode *node);
	LinkNode *findPrevNode(LinkNode *node);

public:
	LoopLink(LinkNode *tail = NULL) : mTail(tail), mLock() {}
	void insert(LinkNode *prev, LinkNode *node);
	void remove(LinkNode *prev, LinkNode *node);
	void remove(LinkNode *node);
	void append(LinkNode *node);
	void push(LinkNode *node);
	LinkNode *pop(void);
	LinkNode *getTopNode(void);
	void setTop(LinkNode *node);
	void setTail(LinkNode *node);
	LinkNode *getTailNode(void);
	bool hasNode(LinkNode *node);
	void traversal(void (*handler)(LinkNode *node, void *data), void *data);
	LinkNode *find(bool (*matcher)(LinkNode *node, void *data), void *data);
};

// ================================================================================

class DoubleLoopLink
{
private:
	DoubleLinkNode *mHead;
	MutexLock mLock;

	void insertOnly(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);
	void insertBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);
	void removeOnly(DoubleLinkNode *prev, DoubleLinkNode *next);
	void removeBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);
	void removeBase(DoubleLinkNode *node);
	void moveBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node);

public:
	DoubleLoopLink(DoubleLinkNode *head = NULL) : mHead(head), mLock() {}

	void insert(DoubleLinkNode *prev, DoubleLinkNode *node);
	void remove(DoubleLinkNode *node);
	void append(DoubleLinkNode *node);
	void push(DoubleLinkNode *node);
	DoubleLinkNode *pop(void);
	DoubleLinkNode *delTail(void);
	DoubleLinkNode *getHeadNode(void);
	DoubleLinkNode *getTailNode(void);
	void traversal(void (*handler)(DoubleLinkNode *node, void *data), void *data);
	DoubleLinkNode *find(bool (*matcher)(DoubleLinkNode *node, void *data), void *data);
	int getCount(void);
	void setTop(DoubleLinkNode *node);
	void setTail(DoubleLinkNode *node);
	void moveToTop(DoubleLinkNode *node);
	void moveToTail(DoubleLinkNode *node);

	DoubleLinkNode *getTop(void)
	{
		return getHeadNode();
	}

	DoubleLinkNode *getTail(void)
	{
		return getTailNode();
	}
};
