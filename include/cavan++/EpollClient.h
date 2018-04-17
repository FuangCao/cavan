#pragma once

/*
 * File:		EpollClient.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-27 11:22:18
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan++/Link.h>

class EpollService;
class EpollDaemon;
class EpollClient;

class EpollBuffer {
protected:
	u16 mOffset;

public:
	EpollBuffer(void) {
		reset();
	}

	virtual ~EpollBuffer() {}

public:
	virtual u16 getSize(void) = 0;
	virtual char *getData(void) = 0;

	virtual int writeTo(EpollClient *client);
	virtual int write(const void *buff, u16 length);

	virtual void reset(void) {
		mOffset = 0;
	}

	virtual u16 getLength(void) {
		return mOffset;
	}

	virtual char *getDataHead(void) {
		return getData() + mOffset;
	}

	virtual u16 getOffset(void) {
		return mOffset;
	}

	virtual void setOffset(u16 offset) {
		mOffset = offset;
	}

	virtual u16 getRemain(void) {
		return getSize() - mOffset;
	}

	virtual bool isCompleted(void) {
		return (mOffset >= getSize());
	}
};

class EpollBufferU16 : public EpollBuffer {
private:
	u16 mLength;

public:
	virtual u16 getSize(void) {
		return sizeof(mLength);
	};

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual char *getData(void) {
		return (char *) &mLength;
	}
};

class EpollBufferAuto : public EpollBuffer {
protected:
	char *mData;
	u16 mSize;
	bool mCompleted;

public:
	EpollBufferAuto(void) : mData(NULL), mSize(0), mCompleted(false) {}

	virtual ~EpollBufferAuto() {
		if (mData != NULL) {
			delete[] mData;
		}
	}

	virtual int alloc(u16 length);
	virtual int write(char c);
	virtual int write(const void *buff, u16 length);

	virtual u16 getSize(void) {
		return mSize;
	}

	virtual char *getData(void) {
		return mData;
	}

	virtual bool isCompleted(void) {
		return mCompleted;
	}

	virtual void setCompleted(bool completed) {
		mCompleted = completed;
	}
};

class EpollPacket : public EpollBuffer, public SimpleLink<EpollPacket> {
protected:
	char *mData;
	u16 mLength;

public:
	EpollPacket(u16 length = 0) : mLength(length) {
		if (length > 0) {
			mData = new char[length];
		} else {
			mData = NULL;
		}
	}

	virtual ~EpollPacket() {
		if (mData != NULL) {
			delete[] mData;
		}
	}

	virtual u16 getSize(void) {
		return mLength;
	}

	virtual char *getData(void) {
		return mData;
	}
};

class EpollClient : public SimpleLink<EpollClient> {
	friend class EpollBuffer;
	friend class EpollPacket;
	friend class EpollService;
	friend class EpollClientQueue;

private:
	SimpleLinkQueue<EpollPacket> mWrQueue;
	SimpleLinkQueue<EpollPacket> mRdQueue;
	EpollPacket *mWrPacket;
	EpollPacket *mRdPacket;

public:
	EpollClient(void) {
		mWrPacket = NULL;
		mRdPacket = NULL;
	}

	virtual ~EpollClient() {}
	virtual int addEpollTo(EpollService *service);
	virtual int removeEpollFrom(EpollService *service);
	virtual int setEpollReadonly(EpollService *service);
	virtual int setEpollReadWrite(EpollService *service);
	virtual int sendEpollPacket(EpollPacket *packet);

	virtual EpollPacket *dequeueEpollPacket(void) {
		return mRdQueue.dequeue();
	}

protected:
	virtual int getEpollFd(void) = 0;

	virtual EpollBuffer *getEpollHeader(void) {
		pr_red_info("getEpollHeader no implement!");
		return NULL;
	}

	virtual int doEpollRead(void *buff, int size) {
		pr_red_info("doEpollRead no implement!");
		return -ENOENT;
	}

	virtual int doEpollWrite(const void *buff, int size) {
		pr_red_info("doEpollWrite no implement!");
		return -ENOENT;
	}

	virtual int onEpollPacketReceived(EpollPacket *packet) {
		pr_red_info("onEpollPacketReceived no implement!");
		return -ENOENT;
	}

	virtual u32 getEpollEventsRO(void) {
		return EPOLLIN | EPOLLERR | EPOLLHUP;
	}

	virtual u32 getEpollEventsRW(void) {
		return getEpollEventsRO() | EPOLLOUT;
	}

protected:
	virtual int onEpollIn(EpollService *service);
	virtual int onEpollDataReceived(EpollService *service, const void *buff, int size);
	virtual EpollPacket *onEpollHeaderReceived(EpollBuffer *header);
	virtual void onEpollEvent(EpollService *service, u32 events);
	virtual int onEpollOut(EpollService *service);
	virtual void onEpollError(EpollService *service);
};

class EpollClientQueue {
private:
	EpollClient *mHead;
	ThreadLock mLock;
	Condition mCond;

public:
	EpollClientQueue(void) : mHead(NULL) {}
	virtual ~EpollClientQueue() {}

public:
	virtual bool enqueue(EpollClient *client);
	virtual EpollClient *dequeueLocked(void);
	virtual EpollClient *dequeue(void);
	virtual void removeLocked(EpollClient *client);
	virtual void remove(EpollClient *client);
	virtual void processPackets(EpollService *service, EpollDaemon *daemon);
};
