#pragma once

/*
 * File:		Udp.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-08-31 11:26:53
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
#include <cavan/udp.h>
#include <cavan++/Thread.h>

class UdpPack {
private:
	u16 mLength;
	struct cavan_udp_header *mHeader;

public:
	UdpPack *next;

public:
	static UdpPack *alloc(const void *buff, u16 length);

public:
	UdpPack(struct cavan_udp_header *header, u16 length) : mLength(length), mHeader(header) {
		next = this;
	}

	virtual ~UdpPack() {
		free(mHeader);
	}

	virtual struct cavan_udp_header *getHeader(void) {
		return mHeader;
	}

	virtual u16 getSequence(void) {
		return mHeader->sequence;
	}

	virtual void setSequence(u16 sequence) {
		mHeader->sequence = sequence;
	}

	virtual u16 copyFrom(const void *buff, u16 size);
	virtual u16 copyTo(void *buff, u16 size);
};

class UdpWin {
private:
	u16 mLength;
	u16 mSequence;
	Condition mCond;
	UdpPack *mPacks[CAVAN_UDP_WIN_SIZE];

public:
	UdpWin(void);
	virtual ~UdpWin() {}

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual u16 getSequence(void) {
		return mSequence;
	}

public:
	virtual bool invalid(u16 sequence);
	virtual bool enqueue(UdpPack *pack, MutexLock *lock, bool nonblock);
	virtual UdpPack *dequeue(MutexLock *lock, bool nonblock);
	virtual int confirm(u16 sequence);
	virtual int receive(const struct cavan_udp_header *header, u16 length);
};

class UdpLink {
private:
	UdpWin mSendWin;
	UdpWin mRecvWin;
	u16 mLocalChannel;
	u16 mRemoteChannel;
	ThreadLock mLock;
	struct sockaddr_in mAddr;

public:
	UdpLink(u16 channel) : mLocalChannel(channel) {}
	virtual ~UdpLink() {}

	virtual u16 getLocalChannel(void) {
		return mLocalChannel;
	}

	virtual void setLocalChannel(u16 channel) {
		mLocalChannel = channel;
	}

	virtual u16 getRemoteChannel(void) {
		return mRemoteChannel;
	}

	virtual void setRemoteChannel(u16 channel) {
		mRemoteChannel = channel;
	}

public:
	virtual ssize_t send(const void *buff, size_t size, bool nonblock);
	virtual ssize_t recv(void *buff, size_t size, bool nonblock);
};

class UdpSock : public SimpleThread {
private:
	int mSockfd;
	u16 mChannel;
	ThreadLock mLock;
	UdpLink *mLinks[0xFFFF];

public:
	UdpSock(void);
	virtual ~UdpSock() {}

	virtual UdpLink *getLink(u16 channel) {
		AutoLock lock(mLock);
		return mLinks[channel];
	}

public:
	virtual int open(u16 port);
	virtual UdpLink *alloc(void);
	virtual ssize_t send(u16 channel, const void *buff, size_t size, bool nonblock);
	virtual ssize_t recv(u16 channel, void *buff, size_t size, bool nonblock);
};
