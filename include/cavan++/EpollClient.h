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
class EpollClient;

class EpollPacket : public SimpleLink<EpollPacket> {
private:
	char *mData;
	u16 mLength;
	u16 mOffset;

public:
	EpollPacket(u16 length = 0) : mLength(length), mOffset(0) {
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

	virtual int writeTo(EpollClient *client);
	virtual int write(const void *buff, u16 length);

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual void seek(u16 offset) {
		mOffset = offset;
	}

	virtual bool isPending(void) {
		return (mOffset < mLength);
	}

	virtual bool isCompleted(void) {
		return (mOffset >= mLength);
	}
};

class EpollClient : public SimpleLink<EpollClient> {
	friend class EpollPacket;
	friend class EpollService;
	friend class EpollClientQueue;

private:
	SimpleLinkQueue<EpollPacket> mWrQueue;
	SimpleLinkQueue<EpollPacket> mRdQueue;
	EpollPacket *mWrPacket;
	EpollPacket *mRdPacket;
	EpollPacket *mHeader;

public:
	virtual ~EpollClient() {}
	virtual int addEpollTo(EpollService *service);
	virtual int removeEpollFrom(EpollService *service);
	virtual int setEpollReadonly(EpollService *service);
	virtual int setEpollReadWrite(EpollService *service);

	virtual EpollPacket *dequeueEpollPacket(void) {
		return mRdQueue.dequeue();
	}

protected:
	virtual int getEpollFd(void) = 0;
	virtual EpollPacket *newEpollHeader(void) = 0;
	virtual int doEpollRead(void *buff, int size) = 0;
	virtual int doEpollWrite(const void *buff, int size) = 0;
	virtual int onEpollPacketReceived(EpollPacket *packet) = 0;

	virtual u32 getEpollEventsRO(void) {
		return EPOLLIN | EPOLLERR | EPOLLHUP;
	}

	virtual u32 getEpollEventsRW(void) {
		return getEpollEventsRO() | EPOLLOUT;
	}

protected:
	virtual int onEpollIn(EpollService *service);
	virtual int onEpollDataReceived(EpollService *service, const void *buff, int size);
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
	virtual void processPackets(void);
};