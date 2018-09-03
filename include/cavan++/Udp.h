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

class UdpSock;
class UdpLink;

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

	virtual void setHeader(const struct cavan_udp_header *header) {
		memcpy(&mHeader, header, sizeof(struct cavan_udp_header));
	}

	virtual void setHeader(cavan_udp_pack_t type, u16 src, u16 dest) {
		mHeader->type = type;
		mHeader->src_channel = src;
		mHeader->dest_channel = dest;
	}

	virtual u16 getSequence(void) {
		return mHeader->sequence;
	}

	virtual void setSequence(u16 sequence) {
		mHeader->sequence = sequence;
	}

	virtual void setSequence(u16 sequence, u8 win) {
		mHeader->sequence = sequence;
		mHeader->win = win;
	}

	virtual u16 write(const void *buff, u16 size);
	virtual u16 read(void *buff, u16 size);
	virtual ssize_t send(UdpLink *link);
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

	virtual u8 getWin(u16 sequence);

public:
	virtual bool invalid(u16 sequence);
	virtual bool enqueue(UdpPack *pack, MutexLock *lock, bool nonblock);
	virtual UdpPack *dequeue(MutexLock *lock, bool nonblock);
	virtual int confirm(u16 sequence);
	virtual int receive(const struct cavan_udp_header *header, u16 length);
	virtual int flush(UdpLink *link, u16 sequence);
};

class UdpLink {
friend class UdpSock;

private:
	u16 mSequence;
	UdpSock *mSock;
	UdpWin mSendWin;
	UdpWin mRecvWin;
	u16 mLocalChannel;
	u16 mRemoteChannel;
	ThreadLock mLock;
	struct sockaddr_in mAddr;

public:
	u64 time;
	UdpLink *next;

public:
	UdpLink(UdpSock *sock, u16 channel) : mSock(sock), mLocalChannel(channel) {
		mSequence = 1;
		next = this;
	}

	virtual ~UdpLink() {}

	virtual UdpSock *getSock(void) {
		return mSock;
	}

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

	virtual struct sockaddr_in *getSockAddr(void) {
		return &mAddr;
	}

	virtual u16 getReadSeq(void) {
		return mRecvWin.getSequence();
	}

public:
	virtual ssize_t send(const void *buff, size_t size, bool nonblock);
	virtual bool send(UdpPack *pack, bool nonblock);
	virtual ssize_t send(struct cavan_udp_header *header);
	virtual ssize_t sendResponse(struct cavan_udp_header *header, cavan_udp_pack_t type);
	virtual ssize_t recv(void *buff, size_t size, bool nonblock);

	virtual int flush(void) {
		return mSendWin.flush(this, mSequence);
	}

protected:
	virtual void onUdpData(struct cavan_udp_header *header, u16 length);
	virtual void onUdpWin(struct cavan_udp_header *header, u16 length);
	virtual void onUdpPing(struct cavan_udp_header *header, u16 length);
	virtual void onUdpDataAck(struct cavan_udp_header *header, u16 length);
};

class UdpSock {
private:
	int mSockfd;
	u16 mChannel;
	Condition mCond;
	ThreadLock mLock;
	UdpLink *mLinks[0xFFFF];
	UdpLink *mHead;

public:
	UdpSock(void);
	virtual ~UdpSock() {}

	virtual UdpLink *getLink(u16 channel) {
		AutoLock lock(mLock);
		return mLinks[channel];
	}

	virtual UdpLink *getLink(struct cavan_udp_header *header) {
		return getLink(header->dest_channel);
	}

public:
	virtual int open(u16 port);
	virtual UdpLink *alloc(void);
	virtual void recycle(u16 channel);

	virtual void recycle(UdpLink *link) {
		return recycle(link->getLocalChannel());
	}

	virtual ssize_t send(u16 channel, const void *buff, size_t size, bool nonblock);
	virtual ssize_t recv(u16 channel, void *buff, size_t size, bool nonblock);
	virtual void post(UdpLink *link, u64 time);

	virtual ssize_t send(UdpLink *link, const void *buff, size_t size) {
		return inet_sendto(mSockfd, buff, size, link->getSockAddr());
	}

	virtual int connect(struct sockaddr_in *addr);
	virtual int connect(const char *host, u16 port);
	virtual int connect(const char *url);

	virtual void recvLoop(void);
	virtual void sendLoop(void);

protected:
	virtual void onUdpTest(struct cavan_udp_header *header, u16 length);
	virtual void onUdpSync(struct cavan_udp_header *header, u16 length);
	virtual void onUdpSyncAck1(struct cavan_udp_header *header, u16 length);
	virtual void onUdpSyncAck2(struct cavan_udp_header *header, u16 length);
	virtual void onUdpData(struct cavan_udp_header *header, u16 length);
	virtual void onUdpWin(struct cavan_udp_header *header, u16 length);
	virtual void onUdpPing(struct cavan_udp_header *header, u16 length);
	virtual void onUdpDataAck(struct cavan_udp_header *header, u16 length);
	virtual void onUdpError(struct cavan_udp_header *header, u16 length);

	virtual void onUdpConnected(UdpLink *link) {}
	virtual void onUdpAccepted(UdpLink *link) {}
	virtual void onUdpDisconnected(UdpLink *link) {}
};
