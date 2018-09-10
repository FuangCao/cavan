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
#include <cavan/network.h>
#include <cavan++/Link.h>
#include <cavan++/Thread.h>

#define CAVAN_UDP_WIN_SIZE			64
#define CAVAN_UDP_CWND				8
#define CAVAN_UDP_MAX_SEND			20
#define CAVAN_UDP_RTO_DEF			500
#define CAVAN_UDP_RTO_MIN			10
#define CAVAN_UDP_RTO_MAX			5000

typedef enum {
	CAVAN_UDP_TEST = 0,
	CAVAN_UDP_SYNC = 1,
	CAVAN_UDP_SYNC_ACK1,
	CAVAN_UDP_SYNC_ACK2,
	CAVAN_UDP_DATA,
	CAVAN_UDP_WIND,
	CAVAN_UDP_PING,
	CAVAN_UDP_DATA_ACK,
	CAVAN_UDP_ERROR,
} cavan_udp_pack_t;

#pragma pack(1)
typedef struct {
	u16 dest_channel;
	u16 src_channel;
	u16	sequence;
	u8 type;

	union {
		u8 data[0];
		u16 data16[0];
		u32 data32[0];
		u64 data64[0];
	};
} cavan_udp_header_t;
#pragma pack()

class UdpSock;
class UdpLink;

class UdpPack {
private:
	u64 mTime;
	u16 mTimes;
	u16 mLength;
	cavan_udp_header_t *mHeader;

public:
	UdpPack *next;

public:
	static UdpPack *alloc(const void *buff, u16 length);

public:
	UdpPack(cavan_udp_header_t *header, const void *data, u16 length);

	virtual ~UdpPack() {
		free(mHeader);
	}

	virtual cavan_udp_header_t *getHeader(void) {
		return mHeader;
	}

	virtual void setHeader(const cavan_udp_header_t *header) {
		memcpy(&mHeader, header, sizeof(cavan_udp_header_t));
	}

	virtual void setHeader(cavan_udp_pack_t type, u16 src, u16 dest) {
		mHeader->type = type;
		mHeader->src_channel = src;
		mHeader->dest_channel = dest;
	}

	virtual u8 *getData(void) {
		return mHeader->data;
	}

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual u16 getSequence(void) {
		return mHeader->sequence;
	}

	virtual void setSequence(u16 sequence) {
		mHeader->sequence = sequence;
	}

	virtual u64 getTime(void) {
		return mTime;
	}

	virtual u16 getTimes(void) {
		return mTimes;
	}

	virtual u16 write(const void *buff, u16 size);
	virtual u16 read(void *buff, u16 size);
	virtual bool send(UdpLink *link, u64 time);
};

class UdpWin {
private:
	u16 mLength;
	u16 mSendSeq;
	u16 mSequence;
	Condition mCond;
	UdpPack *mPacks[CAVAN_UDP_WIN_SIZE];

public:
	UdpWin(u16 sequence);
	virtual ~UdpWin() {}

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual u16 getSequence(void) {
		return mSequence;
	}

	virtual UdpPack *getFirstPack(void) {
		return mPacks[mSequence % CAVAN_UDP_WIN_SIZE];
	}

public:
	virtual bool invalid(u16 sequence);
	virtual bool enqueue(UdpPack *pack, MutexLock *lock, bool nonblock);
	virtual UdpPack *dequeue(MutexLock *lock, bool nonblock);
	virtual int confirm(UdpLink *link, u16 sequence);
	virtual bool receive(UdpLink *link, const cavan_udp_header_t *header, u16 length);
	virtual int flush(UdpLink *link, u64 time);
	virtual u64 resend(UdpLink *link, u64 time);
};

class UdpLink : public SimpleLink<UdpLink> {
friend class UdpWin;
friend class UdpSock;

private:
	u16 mRto;
	u16 mRtts;
	u16 mRttd;
	u16 mAcks;
	u16 mCwnd;
	UdpSock *mSock;
	UdpWin mSendWin;
	UdpWin mRecvWin;
	u16 mLocalChannel;
	u16 mRemoteChannel;
	ThreadLock mLock;
	struct sockaddr_in mAddr;

public:
	u64 time;
	UdpLink *snext;

public:
	UdpLink(UdpSock *sock, const struct sockaddr_in *addr, u16 channel);
	virtual ~UdpLink() {}

	virtual UdpSock *getSock(void) {
		return mSock;
	}

	virtual u16 getUdpCwnd(void) {
		return mCwnd;
	}

	virtual u16 getUdpRto(void) {
		return mRto;
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

	virtual void setSockAddr(const struct sockaddr_in *addr) {
		memcpy(&mAddr, addr, sizeof(mAddr));
	}

	virtual void setSockAddr(const char *host, u16 port);
	virtual void setSockAddr(const char *url);

	virtual u16 getReadSeq(void) {
		return mRecvWin.getSequence();
	}

public:
	virtual ssize_t send(const void *buff, size_t size, bool nonblock);
	virtual bool send(UdpPack *pack, bool nonblock);
	virtual ssize_t send(cavan_udp_header_t *header);
	virtual ssize_t sendResponse(cavan_udp_header_t *header, cavan_udp_pack_t type);
	virtual int flush(void);
	virtual bool connect(void);

protected:
	virtual void processUdpPackData(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackAck(cavan_udp_header_t *header, u16 length);

protected:
	virtual void onUdpTimerFire(u64 time);
	virtual void onUdpPackLose(UdpPack *pack, u64 time);
	virtual void onUdpPackSended(UdpPack *pack, u64 time);

	virtual bool onUdpAccepted(void) {
		return false;
	}

	virtual bool onUdpConnected(void) {
		return false;
	}

	virtual void onUdpDisconnected(void) {
		pr_pos_info();
	}

	virtual void onUdpDataReceived(const void *buff, u16 length) {
		pr_pos_info();
	}
};

class UdpSock {
private:
	u64 mTime;
	int mSockfd;
	u16 mChannel;
	pthread_t mSendThread;
	pthread_t mRecvThread;
	Condition mCond;
	ThreadLock mLock;
	UdpLink *mLinks[0xFFFF];
	UdpLink *mHead;
	SimpleWaitQueue<UdpLink> mQueueReady;
	SimpleLinkQueue<UdpLink> mQueuePending;

	static void *SendThread(void *data) {
		UdpSock *sock = (UdpSock *) data;
		sock->sendLoop();
		return NULL;
	}

	static void *RecvThread(void *data) {
		UdpSock *sock = (UdpSock *) data;
		sock->recvLoop();
		return NULL;
	}

public:
	UdpSock(void);
	virtual ~UdpSock() {}

	virtual UdpLink *getLink(u16 channel) {
		AutoLock lock(mLock);
		return mLinks[channel];
	}

	virtual UdpLink *getLink(cavan_udp_header_t *header) {
		return getLink(header->dest_channel);
	}

public:
	virtual int open(u16 port);
	virtual UdpLink *alloc(const struct sockaddr_in *addr);
	virtual void recycle(u16 channel);

	virtual void recycle(UdpLink *link) {
		return recycle(link->getLocalChannel());
	}

	virtual int start(void);
	virtual int join(void);
	virtual void post(UdpLink *link, u64 time);

	virtual ssize_t send(UdpLink *link, const void *buff, size_t size) {
		return inet_sendto(mSockfd, buff, size, link->getSockAddr());
	}

	virtual UdpLink *connect(struct sockaddr_in *addr);
	virtual UdpLink *connect(const char *host, u16 port);
	virtual UdpLink *connect(const char *url);

	virtual UdpLink *accept(void) {
		return mQueueReady.dequeue();
	}

	virtual void recvLoop(void);
	virtual void sendLoop(void);

protected:
	virtual void processUdpPackTest(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackSync(cavan_udp_header_t *header, u16 length, struct sockaddr_in *addr);
	virtual void processUdpPackSyncAck1(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackSyncAck2(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackData(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackDataAck(cavan_udp_header_t *header, u16 length);
	virtual void processUdpPackError(cavan_udp_header_t *header, u16 length);

	virtual UdpLink *newUdpLink(const struct sockaddr_in *addr, u16 channel) {
		return new UdpLink(this, addr, channel);
	}

	virtual void onUdpAccepted(UdpLink *link) {
		if (!link->onUdpAccepted()) {
			mQueueReady.enqueue(link);
		}
	}

	virtual void onUdpConnected(UdpLink *link) {
		if (!link->onUdpConnected()) {
			mQueueReady.enqueue(link);
		}
	}

	virtual void onUdpDisconnected(UdpLink *link) {
		link->onUdpDisconnected();
	}

	virtual void onUdpKeepAlive(void);
};

template <class T>
class UdpSockT : public UdpSock {
public:
	UdpSockT(void) : UdpSock() {}

protected:
	virtual UdpLink *newUdpLink(const struct sockaddr_in *addr, u16 channel) override {
		return new T(this, addr, channel);
	}
};
