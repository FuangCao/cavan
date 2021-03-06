/*
 * File:		Udp.cpp
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
#include <cavan/math.h>
#include <cavan++/Udp.h>

#define CAVAN_UDP_DEBUG		0

UdpPack *UdpPack::alloc(const void *buff, u16 length)
{
	cavan_udp_header_t *header = (cavan_udp_header_t *) malloc(sizeof(cavan_udp_header_t) + length);
	if (header == NULL) {
		return NULL;
	}

	UdpPack *pack = new UdpPack(header, buff, length);
	if (pack == NULL) {
		free(header);
		return NULL;
	}

	return pack;
}

UdpPack::UdpPack(cavan_udp_header_t *header, const void *data, u16 length) : mLength(length), mHeader(header)
{
	if (data != NULL) {
		memcpy(header->data, data, length);
	}

	mTime = 0;
	mTimes = 0;
	next = this;
}

u16 UdpPack::write(const void *buff, u16 size)
{
	if (size > mLength) {
		size = mLength;
	}

	memcpy(mHeader->data, buff, size);

	return size;
}

u16 UdpPack::read(void *buff, u16 size)
{
	if (size > mLength) {
		size = mLength;
	}

	memcpy(buff, mHeader->data, size);

	return size;
}

bool UdpPack::send(UdpLink *link, u64 time)
{
	u16 length = sizeof(cavan_udp_header_t) + mLength;

#if CAVAN_UDP_DEBUG
	println("send[%d]: %d", mHeader->sequence, length);
#endif

	mTime = time;
	mTimes++;

	if (link->getSock()->send(link, mHeader, length) < 0) {
		return false;
	}

	return true;
}

UdpWin::UdpWin(u16 sequence) : mLength(0), mSendSeq(0), mSequence(sequence)
{
	for (int i = 0; i < NELEM(mPacks); i++) {
		mPacks[i] = NULL;
	}
}

bool UdpWin::invalid(u16 sequence)
{
	u16 end = mSequence + NELEM(mPacks);

	if (mSequence < end) {
		return (sequence < mSequence || sequence >= end);
	} else {
		return (sequence < mSequence && sequence >= end);
	}
}

bool UdpWin::enqueue(UdpPack *pack, MutexLock *lock, bool nonblock)
{
	while (1) {
		if (mLength < NELEM(mPacks)) {
			u16 sequence = mSequence + mLength;
			u16 index = sequence % NELEM(mPacks);
			pack->setSequence(sequence);
			mPacks[index] = pack;
			mLength++;
			return true;
		}

		if (nonblock) {
			break;
		}

		mCond.waitLocked(lock);
	}

	return false;
}

UdpPack *UdpWin::dequeue(MutexLock *lock, bool nonblock)
{
	while (1) {
		if (mLength > 0) {
			u16 index = mSequence % NELEM(mPacks);
			UdpPack *pack = mPacks[index];
			mPacks[index] = NULL;
			mSequence++;
			mLength--;
			return pack;
		}

		if (nonblock) {
			break;
		}

		mCond.waitLocked(lock);
	}

	return NULL;
}

int UdpWin::confirm(UdpLink *link, u16 sequence)
{
	u16 index = sequence % NELEM(mPacks);
	UdpPack *pack = mPacks[index];

	if (pack == NULL || pack->getSequence() != sequence) {
		return -EINVAL;
	}

	u64 time = clock_gettime_real_ms();

	link->onUdpPackSended(pack, time);
	mPacks[index] = NULL;
	delete pack;

	int count = 0;

	while (mLength > 0) {
		index = mSequence % NELEM(mPacks);
		if (mPacks[index] == NULL) {
			if (mSequence == mSendSeq) {
				mSendSeq++;
			}

			mSequence++;
			mLength--;
			mCond.signal();
		} else {
			break;
		}

		count++;
	}

	flush(link, time);

	return count;
}

bool UdpWin::receive(UdpLink *link, const cavan_udp_header_t *header, u16 length)
{
	u16 sequence = header->sequence;

	if (invalid(sequence)) {
		return true;
	}

	u16 index = sequence % NELEM(mPacks);
	UdpPack *pack = mPacks[index];

	if (pack != NULL) {
		return true;
	}

#if CAVAN_UDP_DEBUG
	println("receive: %d <> %d", sequence, mSequence);
#endif

	length -= sizeof(cavan_udp_header_t);

	if (sequence != mSequence) {
		pack = UdpPack::alloc(header->data, length);
		if (pack == NULL) {
			return false;
		}

		memcpy(pack->getHeader(), header, sizeof(cavan_udp_header_t));
		mPacks[index] = pack;

		return true;
	}

	link->onUdpDataReceived(header->data, length);

	while (1) {
		u16 index = ++mSequence % CAVAN_UDP_WIN_SIZE;
		UdpPack *pack = mPacks[index];
		if (pack == NULL) {
			break;
		}

		link->onUdpDataReceived(pack->getData(), pack->getLength());
		mPacks[index] = NULL;
		free(pack);
	}

	return true;
}

int UdpWin::flush(UdpLink *link, u64 time)
{
	u16 cwnd = link->getUdpCwnd();
	u16 seq = mSendSeq;
	int count = 0;

	if (cwnd < mLength) {
		u16 sequence = mSequence + cwnd;

		if (sequence < mSequence) {
			if (seq < mSequence && seq > sequence) {
				return 0;
			}
		} else {
			if (seq < mSequence || seq > sequence) {
				return 0;
			}
		}
	} else {
		cwnd = mLength;
	}

	for (u16 end = mSequence + cwnd; seq != end; seq++) {
		UdpPack *pack = mPacks[seq % NELEM(mPacks)];
		if (pack == NULL) {
			continue;
		}

		pack->send(link, time);
		count++;
	}

	mSendSeq = seq;

	return count;
}

u64 UdpWin::resend(UdpLink *link, u64 time)
{
	UdpPack *pack = getFirstPack();
	if (pack == NULL) {
		return 0;
	}

	if (pack->getTimes() > 10) {
		link->onUdpError();
		return 0;
	}

	u64 overtime = pack->getTime();
	if (overtime == 0) {
		return 0;
	}

	overtime += link->getUdpRto();

	if (time < overtime) {
		return overtime;
	}

	link->onUdpPackLose(pack, time);
	pack->send(link, time);
	mSendSeq = mSequence + 1;

	return time + link->getUdpRto();
}

UdpLink::UdpLink(UdpSock *sock, const struct sockaddr_in *addr, u16 channel) : mSock(sock), mSendWin(0), mRecvWin(1), mLocalChannel(channel)
{
	setSockAddr(addr);
	snext = this;
	mAcks = 0;
	mCwnd = 1;
	mRttd = 0;
	mRtts = CAVAN_UDP_RTO_DEF;
	mRto = CAVAN_UDP_RTO_DEF;
}

void UdpLink::setSockAddr(const char *host, u16 port)
{
	inet_sockaddr_init(&mAddr, host, port);
}

void UdpLink::setSockAddr(const char *url)
{
	inet_sockaddr_init_url(&mAddr, url);
}

ssize_t UdpLink::send(const void *buff, size_t size, bool nonblock)
{
	UdpPack *pack = UdpPack::alloc(buff, size);
	if (pack == NULL) {
		return -ENOMEM;
	}

	pack->setHeader(CAVAN_UDP_DATA, mLocalChannel, mRemoteChannel);

	if (send(pack, nonblock)) {
		return size;
	}

	delete pack;

	return -EAGAIN;
}

bool UdpLink::send(UdpPack *pack, bool nonblock)
{
	bool success;

	mLock.acquire();
	success = mSendWin.enqueue(pack, &mLock, nonblock);
	mLock.release();

	if (success) {
		flush();
		return true;
	}

	return false;
}

ssize_t UdpLink::send(const cavan_udp_header_t *header)
{
	return mSock->send(this, header, sizeof(cavan_udp_header_t));
}

ssize_t UdpLink::sendResponse(const cavan_udp_header_t *header, cavan_udp_pack_t type)
{
	cavan_udp_header_t response;
	u16 sequence = header->sequence;

	response.type = type;
	response.sequence = sequence;
	response.dest_channel = header->src_channel;
	response.src_channel = header->dest_channel;

	return send(&response);
}

int UdpLink::flush(void)
{
	u64 time = clock_gettime_real_ms();
	int count;

	AutoLock lock(mLock);

	count = mSendWin.flush(this, time);
	if (count > 0) {
		mSock->post(this, time + mRto);
	}

	return count;
}

bool UdpLink::connect(void)
{
	UdpPack *pack = UdpPack::alloc(NULL, 0);
	if (pack == NULL) {
		return false;
	}

	pack->setHeader(CAVAN_UDP_SYNC, getLocalChannel(), 0);
	pack->setSequence(0);

	if (send(pack, true)) {
		return true;
	}

	delete pack;

	return false;
}

void UdpLink::processUdpPackData(const cavan_udp_header_t *header, u16 length)
{
	AutoLock lock(mLock);

	if (mRecvWin.receive(this, header, length)) {
		sendResponse(header, CAVAN_UDP_DATA_ACK);
	}
}

void UdpLink::confirm(u16 sequence)
{
	mLock.acquire();
	mSendWin.confirm(this, sequence);
	mLock.release();

	flush();
}

void UdpLink::onUdpError(void)
{
	mSock->recycle(this);
}

void UdpLink::onUdpTimerFire(u64 time)
{
	AutoLock lock(mLock);

	time = mSendWin.resend(this, time);
	if (time != 0) {
		mSock->post(this, time);
	}
}

void UdpLink::onUdpPackLose(UdpPack *pack, u64 time)
{
	mCwnd = 1;
	mAcks = 0;

	mRto <<= 1;

	if (mRto > CAVAN_UDP_RTO_MAX) {
		mRto = CAVAN_UDP_RTO_MAX;
	}
}

void UdpLink::onUdpPackSended(UdpPack *pack, u64 time)
{
	if (pack->getTimes() == 1) {
		u32 rtt = time - pack->getTime();
		u32 rttd = mRttd * 3 + RANG(mRtts, rtt);

		mRtts = (mRtts * 7 + rtt) >> 3;
		mRttd = rttd >> 2;
		mRto = (mRtts + rttd) << 1;

		if (mRto < CAVAN_UDP_RTO_MIN) {
			mRto = CAVAN_UDP_RTO_MIN;
		}

#if CAVAN_UDP_DEBUG
		println("RTT = %d, RTTS = %d, RTTD = %d, RTO = %d", rtt, mRtts, mRttd, mRto);
#endif

		if (mAcks < mCwnd) {
			mAcks++;
		} else {
			mAcks= 0;

			if (mCwnd < CAVAN_UDP_CWND) {
				mCwnd++;
			}
		}

#if CAVAN_UDP_DEBUG
		println("cwnd = %d, acks = %d", mCwnd, mAcks);
#endif
	}
}

UdpSock::UdpSock(void) : mSockfd(INVALID_SOCKET), mChannel(0)
{
	for (int i = 0; i < NELEM(mLinks); i++) {
		mLinks[i] = NULL;
	}

	mHead = NULL;
	mTime = 0;
}

int UdpSock::open(u16 port)
{
	int sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	if (port > 0) {
		struct sockaddr_in addr;

		inet_sockaddr_init(&addr, NULL, port);

		int ret = inet_bind(sockfd, &addr);
		if (ret < 0) {
			close(sockfd);
			return ret;
		}
	}

	mSockfd = sockfd;

	return sockfd;
}

UdpLink *UdpSock::alloc(const sockaddr_in *addr)
{
	u16 channel;

	AutoLock lock(mLock);

	for (channel = mChannel + 1; channel > 0; channel++) {
		if (mLinks[channel] == NULL) {
			goto out_found;
		}
	}

	for (channel = 1; channel < mChannel; channel++) {
		if (mLinks[channel] == NULL) {
			goto out_found;
		}
	}

	return NULL;

out_found:
	UdpLink *link = newUdpLink(addr, channel);
	if (link == NULL) {
		return NULL;
	}

	mLinks[channel] = link;

	return link;
}

void UdpSock::recycle(u16 channel)
{
	AutoLock lock(mLock);

	UdpLink *link = mLinks[channel];
	if (link == NULL) {
		return;
	}

	mLinks[channel] = NULL;

	link->onUdpRecycle();
	mGarbageCollector.enqueue(link);
}

ssize_t UdpSock::sendResponse(const cavan_udp_header_t *header, cavan_udp_pack_t type, const struct sockaddr_in *addr)
{
	cavan_udp_header_t response;

	response.src_channel = header->dest_channel;
	response.dest_channel = header->src_channel;
	response.sequence = header->sequence;
	response.type = type;

	return inet_sendto(mSockfd, &response, sizeof(response), addr);
}

void UdpSock::processUdpPackTest(const cavan_udp_header_t *header)
{
	pr_pos_info();
}

void UdpSock::processUdpPackSync(const cavan_udp_header_t *header, const struct sockaddr_in *addr)
{
	UdpLink *link = alloc(addr);
	if (link == NULL) {
		return;
	}

	UdpPack *pack = UdpPack::alloc(NULL, 0);
	if (pack == NULL) {
		recycle(link);
		return;
	}

	pack->setHeader(CAVAN_UDP_SYNC_ACK1, link->getLocalChannel(), header->src_channel);
	pack->setSequence(link->getReadSeq());
	link->send(pack, false);
}

void UdpSock::processUdpPackSyncAck1(const cavan_udp_header_t *header)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || header->src_channel == channel) {
		link->setRemoteChannel(header->src_channel);
		link->sendResponse(header, CAVAN_UDP_SYNC_ACK2);
		link->confirm(header->sequence);
		onUdpConnected(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::processUdpPackSyncAck2(const cavan_udp_header_t *header)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || channel == header->src_channel) {
		link->setRemoteChannel(header->src_channel);
		link->confirm(header->sequence);
		onUdpAccepted(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::processUdpPackData(const cavan_udp_header_t *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->processUdpPackData(header, length);
	}
}

void UdpSock::processUdpPackDataAck(const cavan_udp_header_t *header)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->confirm(header->sequence);
	}
}

void UdpSock::processUdpPackShut(const cavan_udp_header_t *header, const struct sockaddr_in *addr)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		recycle(link);
	}

	sendResponse(header, CAVAN_UDP_SHUT_ACK, addr);
}

void UdpSock::processUdpPackShutAck(const cavan_udp_header_t *header)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->confirm(header->sequence);
		recycle(link);
	}
}

void UdpSock::processUdpPackPing(const cavan_udp_header_t *header)
{
	pr_pos_info();
}

void UdpSock::processUdpPackPong(const cavan_udp_header_t *header)
{
	pr_pos_info();
}

void UdpSock::processUdpPackError(const cavan_udp_header_t *header)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		recycle(link);
	}
}

void UdpSock::recvLoop(void)
{
	cavan_udp_header_t *header;
	struct sockaddr_in addr;
	char buff[1500];

	header = (cavan_udp_header_t *) buff;

	while (1) {
		int length = inet_recvfrom(mSockfd, buff, sizeof(buff), &addr);
		if (length < (int) sizeof(cavan_udp_header_t)) {
			if (length < 0) {
				break;
			}

			continue;
		}

#if 0
		println("inet_recvfrom: %d", length);
		println("type = %d", header->type);
		println("sequence = %d", header->sequence);
		println("src = %d", header->src_channel);
		println("dest = %d", header->dest_channel);
#endif

		switch (header->type) {
		case CAVAN_UDP_TEST:
			println("CAVAN_UDP_TEST");
			processUdpPackTest(header);
			break;

		case CAVAN_UDP_SYNC:
			println("CAVAN_UDP_SYNC");
			processUdpPackSync(header, &addr);
			break;

		case CAVAN_UDP_SYNC_ACK1:
			println("CAVAN_UDP_SYNC_ACK1");
			processUdpPackSyncAck1(header);
			break;

		case CAVAN_UDP_SYNC_ACK2:
			println("CAVAN_UDP_SYNC_ACK2");
			processUdpPackSyncAck2(header);
			break;

		case CAVAN_UDP_DATA:
#if CAVAN_UDP_DEBUG
			println("CAVAN_UDP_DATA[%d]", header->sequence);
#endif
			processUdpPackData(header, length);
			break;

		case CAVAN_UDP_DATA_ACK:
#if CAVAN_UDP_DEBUG
			println("CAVAN_UDP_DATA_ACK[%d]", header->sequence);
#endif
			processUdpPackDataAck(header);
			break;

		case CAVAN_UDP_SHUT:
			println("CAVAN_UDP_SHUT");
			processUdpPackShut(header, &addr);
			break;

		case CAVAN_UDP_SHUT_ACK:
			println("CAVAN_UDP_SHUT_ACK");
			processUdpPackShutAck(header);
			break;

		case CAVAN_UDP_PING:
			processUdpPackPing(header);
			break;

		case CAVAN_UDP_PONG:
			processUdpPackPong(header);
			break;

		case CAVAN_UDP_ERROR:
			println("CAVAN_UDP_ERROR");
			processUdpPackError(header);
			break;

		default:
			pr_err_info("Invalid type: %d", header->type);
		}
	}
}

void UdpSock::post(UdpLink *link, u64 time)
{
	AutoLock lock(mLock);

	if (link->snext != link) {
		return;
	}

	UdpLink **pp = &mHead;

	while (1) {
		UdpLink *p = *pp;

		if (p == NULL) {
			link->snext = NULL;
			break;
		}

		if (p->time > time) {
			link->snext = p;
			break;
		}

		pp = &p->snext;
	}

	link->time = time;
	*pp = link;

	if (mHead == link) {
		mCond.signal();
	}
}

void UdpSock::sendLoop(void)
{
	mLock.acquire();

	while (1) {
		struct timespec time;
		UdpLink *link;
		u64 mseconds;
		u32 delay;

		while (1) {
			clock_gettime_real(&time);
			mseconds = cavan_timespec_mseconds(&time);

			if (mseconds < mTime) {
				delay = mTime - mseconds;
			} else {
				delay = 2000;
				onUdpKeepAlive();
				mTime = mseconds + delay;
			}

			link = mHead;

			if (link != NULL && mLinks[link->getLocalChannel()] == link) {
				if (link->time <= mseconds) {
					break;
				}

				if (link->time < mTime) {
					delay = link->time - mseconds;
				}
			}

			cavan_timespec_add_ms(&time, delay);
			// println("wait: %d", delay);
			mCond.waitLocked(&time, &mLock);
		}

		// println("link = %p", link);

		mHead = link->snext;
		if (mHead == link) {
			mHead = NULL;
		}

		link->snext = link;

		mLock.release();
		link->onUdpTimerFire(mseconds);
		mLock.acquire();
	}

	mLock.release();
}

UdpLink *UdpSock::connect(struct sockaddr_in *addr)
{
	UdpLink *link = alloc(addr);
	if (link == NULL) {
		return NULL;
	}

	if (link->connect()) {
		return link;
	}

	recycle(link);

	return NULL;
}

UdpLink *UdpSock::connect(const char *host, u16 port)
{
	struct sockaddr_in addr;

	inet_sockaddr_init(&addr, host, port);

	return connect(&addr);
}

UdpLink *UdpSock::connect(const char *url)
{
	struct sockaddr_in addr;

	inet_sockaddr_init_url(&addr, url);

	return connect(&addr);
}

int UdpSock::start(void)
{
	int ret;

	ret = cavan_pthread_create(&mSendThread, SendThread, this, true);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create");
		return ret;
	}

	ret = cavan_pthread_create(&mRecvThread, RecvThread, this, true);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create");
		return ret;
	}

	return 0;
}

int UdpSock::join(void)
{
	int ret;

	ret = pthread_join(mSendThread, NULL);
	if (ret < 0) {
		pr_err_info("pthread_join");
		return ret;
	}

	ret = pthread_join(mRecvThread, NULL);
	if (ret < 0) {
		pr_err_info("pthread_join");
		return ret;
	}

	return 0;
}

void UdpSock::onUdpKeepAlive(void)
{
	SimpleLink<UdpLink> *head = mGarbageCollector.swap();
	head->destroy();
}
