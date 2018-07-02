#pragma once

/*
 * File:		Link.h
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
#include <cavan++/Lock.h>

struct LinkNode {
	LinkNode *next;
};

template <typename T>
struct LinkNodeT : public LinkNode {
	T data;

	LinkNodeT(void) {}
	LinkNodeT(T data) : data(data) {}
};

// ================================================================================

struct DoubleLinkNode {
	DoubleLinkNode *prev, *next;
};

template <typename T>
struct DoubleLinkNodeT : public DoubleLinkNode {
	T data;

	DoubleLinkNodeT(void) {}
	DoubleLinkNodeT(T data) : data(data) {}
};

// ================================================================================

class Link {
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

	bool isEmpty(void) {
		AutoLock lock(mLock);

		return mHead == NULL;
	}

	bool hasNode(void) {
		AutoLock lock(mLock);

		return mHead != NULL;
	}
};

// ================================================================================

class DoubleLink {
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

class LoopLink {
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

class DoubleLoopLink {
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

	DoubleLinkNode *getTop(void) {
		return getHeadNode();
	}

	DoubleLinkNode *getTail(void) {
		return getTailNode();
	}
};

template <class T>
class SimpleLink {
private:
	SimpleLink<T> *prev;
	SimpleLink<T> *next;

public:
	SimpleLink(void) {
		reset();
	}

	virtual ~SimpleLink() {}

	virtual T *getPrev(void) {
		return (T *) prev;
	}

	virtual T *getNext(void) {
		return (T *) next;
	}

	virtual void reset(void) {
		next = prev = this;
	}

	virtual void prepend(T *node) {
		node->next = next;
		node->prev = this;
		next->prev = node;
		next = node;
	}

	virtual void append(T *node) {
		node->prev = prev;
		node->next = this;
		prev->next = node;
		prev = node;
	}

	virtual void remove(void) {
		prev->next = next;
		next->prev = prev;
		reset();
	}

	virtual T *removeFirst(void) {
		SimpleLink<T> *node = next;

		if (node == this) {
			return NULL;
		}

		next = node->next;
		next->prev = this;
		node->reset();

		return (T *) node;
	}

	virtual T *removeLast(void) {
		SimpleLink<T> *node = prev;

		if (node == this) {
			return NULL;
		}

		prev = node->prev;
		prev->next = this;
		node->reset();

		return (T *) node;
	}

	virtual bool isFree(void) {
		return (next == prev);
	}

	virtual bool isUsed(void) {
		return (next != prev);
	}

	virtual bool isEmpty(void) {
		return (next == prev);
	}

	virtual bool isNotEmpty(void) {
		return (next != prev);
	}
};

template <class T>
class SimpleLinkQueue {
protected:
	SimpleLink<T> mHead;
	ThreadLock mLock;

public:
	virtual ~SimpleLinkQueue() {}

	virtual ILock &getLock(void) {
		return mLock;
	}

	virtual void lock(void) {
		mLock.acquire();
	}

	virtual void unlock(void) {
		mLock.release();
	}

	virtual void enqueue(T *node) {
		AutoLock lock(mLock);
		mHead.append(node);
	}

	virtual T *dequeue(void) {
		AutoLock lock(mLock);

		T *node = mHead.removeFirst();
		if (node == NULL) {
			return NULL;
		}

		return node;
	}

	virtual void remove(T *node) {
		AutoLock lock(mLock);
		node->remove();
	}

	virtual T *getFirst(void) {
		return mHead.getNext();
	}

	virtual T *getLast(void) {
		return mHead.getPrev();
	}

	virtual bool hasNext(T *node) {
		return (node != &mHead);
	}
};

template <class T>
class SimpleWaitQueue : public SimpleLinkQueue<T> {
	using SimpleLinkQueue<T>::mCount;
	using SimpleLinkQueue<T>::mLock;
	using SimpleLinkQueue<T>::mHead;

protected:
	Condition mCond;

private:
	virtual void enqueueLocked(T *node) {
		mHead.append(node);

		if (++mCount == 1) {
			mCond.signal();
		}
	}

public:
	virtual void enqueue(T *node) {
		AutoLock lock(mLock);
		enqueueLocked(node);
	}

	virtual bool enqueueSafe(T *node) {
		AutoLock lock(mLock);

		if (node->isUsed()) {
			return false;
		}

		enqueueLocked(node);

		return true;
	}

	virtual T *dequeue(void) {
		AutoLock lock(mLock);

		while (1) {
			T *node = mHead.removeFirst();
			if (node != NULL) {
				mCount--;
				return node;
			}

			mCond.waitLocked(mLock);
		}
	}
};

template <class T, int SIZE>
class MultiBufferQueue {
private:
	SimpleLink<T> mHeads[SIZE];
	ThreadLock mLock;
	u8 mIndex;

public:
	MultiBufferQueue(void) : mIndex(0) {}
	virtual ~MultiBufferQueue() {}

	SimpleLink<T> *swap(void) {
		AutoLock lock(mLock);
		mIndex = (mIndex + 1) % SIZE;
		return mHeads + (mIndex + 1) % SIZE;
	}

	void enqueue(T *node) {
		AutoLock lock(mLock);
		mHeads[mIndex].append(node);
	}
};
