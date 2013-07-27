/*
 * File:		Link.cpp
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
#include <cavan++/link.h>

LinkNode *Link::getLastNode(void)
{
	if (mHead == NULL)
	{
		return NULL;
	}

	LinkNode *node;

	for (node = mHead; node->next; node = node->next);

	return node;
}

LinkNode *Link::findPrevNode(LinkNode *node)
{
	LinkNode *prev;

	for (prev = mHead; prev->next != node; prev = prev->next);

	return prev;
}

void Link::insertBase(LinkNode *prev, LinkNode *node)
{
	node->next = prev->next;
	prev->next = node;
}

void Link::insert(LinkNode *prev, LinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(prev, node);
}

void Link::append(LinkNode *node)
{
	node->next = NULL;

	AutoLock lock(mLock);

	LinkNode *prev = getLastNode();
	if (prev == NULL)
	{
		mHead = node;
	}
	else
	{
		prev->next = node;
	}
}

void Link::push(LinkNode *node)
{
	AutoLock lock(mLock);

	node->next = mHead;
	mHead = node;
}

void Link::remove(LinkNode *prev, LinkNode *node)
{
	AutoLock lock(mLock);

	prev->next = node->next;
}

void Link::remove(LinkNode *node)
{
	AutoLock lock(mLock);

	if (node == mHead)
	{
		mHead = mHead->next;
	}
	else
	{
		LinkNode *prev = findPrevNode(node);
		if (prev)
		{
			prev->next = node->next;
		}
	}
}

LinkNode *Link::pop(void)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return NULL;
	}

	LinkNode *node = mHead;
	mHead = mHead->next;

	return node;
}

void Link::traversal(void (*handler)(LinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	for (LinkNode *node = mHead; node; node = node->next)
	{
		handler(node, data);
	}
}

LinkNode *Link::find(bool (*matcher)(LinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	for (LinkNode *node = mHead; node; node = node->next)
	{
		if (matcher(node, data))
		{
			return node;
		}
	}

	return NULL;
}

// ================================================================================

void DoubleLink::insertBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
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

DoubleLinkNode *DoubleLink::getLastNode(void)
{
	if (mHead == NULL)
	{
		return NULL;
	}

	DoubleLinkNode *node;

	for (node = mHead; node->next; node = node->next);

	return node;
}

void DoubleLink::insert(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(prev, next, node);
}

void DoubleLink::append(DoubleLinkNode *prev, DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(prev, prev->next, node);
}

void DoubleLink::append(DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	node->next = NULL;

	DoubleLinkNode *prev = getLastNode();
	insertBase(prev, NULL, node);

	if (prev == NULL)
	{
		mHead = node;
	}
}

void DoubleLink::push(DoubleLinkNode *next, DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(next->prev, next, node);
}

void DoubleLink::push(DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(NULL, mHead, node);
	mHead = node;
}

void DoubleLink::traversal(void (*handler)(DoubleLinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	for (DoubleLinkNode *node = mHead; node; node = node->next)
	{
		handler(node, data);
	}
}

DoubleLinkNode *DoubleLink::find(bool (*matcher)(DoubleLinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	for (DoubleLinkNode *node = mHead; node; node = node->next)
	{
		if (matcher(node, data))
		{
			return node;
		}
	}

	return NULL;
}

// ================================================================================

void LoopLink::insertBase(LinkNode *prev, LinkNode *node)
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

void LoopLink::removeBase(LinkNode *prev, LinkNode *node)
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

LinkNode *LoopLink::findPrevNode(LinkNode *node)
{
	LinkNode *prev;

	for (prev = node->next; prev->next != node; prev = prev->next)
	{
		if (prev == node)
		{
			return NULL;
		}
	}

	return prev;
}

void LoopLink::insert(LinkNode *prev, LinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(prev, node);
}

void LoopLink::remove(LinkNode *prev, LinkNode *node)
{
	AutoLock lock(mLock);

	removeBase(prev, node);
}

void LoopLink::remove(LinkNode *node)
{
	AutoLock lock(mLock);

	LinkNode *prev = findPrevNode(node);
	if (prev)
	{
		removeBase(prev, node);
	}
}

void LoopLink::append(LinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(mTail, node);
	mTail = node;
}

void LoopLink::push(LinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(mTail, node);
}

LinkNode *LoopLink::pop(void)
{
	AutoLock lock(mLock);

	if (mTail == NULL)
	{
		return NULL;
	}

	LinkNode *node = mTail->next;
	removeBase(mTail, node);

	return node;
}

LinkNode *LoopLink::getTopNode(void)
{
	AutoLock lock(mLock);

	if (mTail == NULL)
	{
		return NULL;
	}

	return mTail->next;
}

void LoopLink::setTop(LinkNode *node)
{
	AutoLock lock(mLock);

	mTail = findPrevNode(node);
}

void LoopLink::setTail(LinkNode *node)
{
	AutoLock lock(mLock);

	mTail = node;
}

LinkNode *LoopLink::getTailNode(void)
{
	AutoLock lock(mLock);

	return mTail;
}

bool LoopLink::hasNode(LinkNode *node)
{
	AutoLock lock(mLock);

	if (mTail == NULL)
	{
		return false;
	}

	for (LinkNode *head = mTail->next;; head = head->next)
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

void LoopLink::traversal(void (*handler)(LinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	if (mTail == NULL)
	{
		return;
	}

	for (LinkNode *node = mTail->next;; node = node->next)
	{
		handler(node, data);

		if (node == mTail)
		{
			break;
		}
	}
}

LinkNode *LoopLink::find(bool (*matcher)(LinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	if (mTail == NULL)
	{
		return NULL;
	}

	for (LinkNode *node = mTail->next;; node = node->next)
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

// ================================================================================

void DoubleLoopLink::insertOnly(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
{
	node->prev = prev;
	node->next = next;

	prev->next = node;
	next->prev = node;
}

void DoubleLoopLink::insertBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
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

void DoubleLoopLink::removeOnly(DoubleLinkNode *prev, DoubleLinkNode *next)
{
	prev->next = next;
	next->prev = prev;
}

void DoubleLoopLink::removeBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
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

void DoubleLoopLink::removeBase(DoubleLinkNode *node)
{
	removeBase(node->prev, node->next, node);
}

void DoubleLoopLink::moveBase(DoubleLinkNode *prev, DoubleLinkNode *next, DoubleLinkNode *node)
{
	removeOnly(node->prev, node->next);
	insertOnly(prev, next, node);
}

void DoubleLoopLink::insert(DoubleLinkNode *prev, DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	insertBase(prev, prev->next, node);
}

void DoubleLoopLink::remove(DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	removeBase(node->prev, node->next, node);
}

void DoubleLoopLink::append(DoubleLinkNode *node)
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

void DoubleLoopLink::push(DoubleLinkNode *node)
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

DoubleLinkNode *DoubleLoopLink::pop(void)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return NULL;
	}

	DoubleLinkNode *node = mHead;
	removeBase(node);

	return node;
}

DoubleLinkNode *DoubleLoopLink::delTail(void)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return NULL;
	}

	DoubleLinkNode *node = mHead->prev;
	removeBase(node);

	return node;
}

DoubleLinkNode *DoubleLoopLink::getHeadNode(void)
{
	AutoLock lock(mLock);

	return mHead;
}

DoubleLinkNode *DoubleLoopLink::getTailNode(void)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return NULL;
	}

	return mHead->prev;
}

void DoubleLoopLink::traversal(void (*handler)(DoubleLinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return;
	}

	for (DoubleLinkNode *node = mHead, *tail = node->prev;; node = node->next)
	{
		handler(node, data);

		if (node == tail)
		{
			break;
		}
	}
}

DoubleLinkNode *DoubleLoopLink::find(bool (*matcher)(DoubleLinkNode *node, void *data), void *data)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return NULL;
	}

	for (DoubleLinkNode *node = mHead, *tail = node->prev;; node = node->next)
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

int DoubleLoopLink::getCount(void)
{
	AutoLock lock(mLock);

	if (mHead == NULL)
	{
		return 0;
	}

	int count = 0;

	for (DoubleLinkNode *node = mHead, *tail = node->prev;; node = node->next)
	{
		count++;

		if (node == tail)
		{
			break;
		}
	}

	return count;
}

void DoubleLoopLink::setTop(DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	mHead = node;
}

void DoubleLoopLink::setTail(DoubleLinkNode *node)
{
	AutoLock lock(mLock);

	mHead = node->next;
}

void DoubleLoopLink::moveToTop(DoubleLinkNode *node)
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

void DoubleLoopLink::moveToTail(DoubleLinkNode *node)
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
