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

UdpPack *UdpPack::alloc(const void *buff, u16 length)
{
	struct cavan_udp_header *header = (struct cavan_udp_header *) malloc(sizeof(struct cavan_udp_header) + length);
	if (header == NULL) {
		return NULL;
	}

	UdpPack *pack = new UdpPack(header, length);
	if (pack == NULL) {
		free(header);
		return NULL;
	}

	if (length > 0) {
		memcpy(header->data, buff, length);
	}

	return pack;
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
	u16 length = sizeof(struct cavan_udp_header) + mLength;

	mTime = time;
	mSendTimes++;

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
			mSequence++;
			mLength--;
		} else {
			break;
		}

		count++;
	}

	flush(link, time);

	return count;
}

bool UdpWin::receive(UdpLink *link, const struct cavan_udp_header *header, u16 length)
{
	u16 sequence = header->sequence;
	if (invalid(sequence)) {
		pr_pos_info();
		return true;
	}

	u16 index = sequence % NELEM(mPacks);
	UdpPack *pack = mPacks[index];

	if (pack != NULL) {
		pr_pos_info();
		return true;
	}

	println("receive: %d <> %d", sequence, mSequence);

	if (sequence != mSequence) {
		pack = UdpPack::alloc(header->data, length);
		if (pack == NULL) {
			pr_pos_info();
			return false;
		}

		memcpy(pack->getHeader(), header, sizeof(struct cavan_udp_header));
		mPacks[index] = pack;

		pr_pos_info();
		return true;
	}

	link->onUdpDataReceived(header->data, length);

	while (1) {
		u16 index = ++mSequence % CAVAN_UDP_WIN_SIZE;
		UdpPack *pack = mPacks[index];
		if (pack == NULL) {
			break;
		}

		link->onUdpDataReceived(header->data, pack->getLength());
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

		println("flush[%d]: pack = %p", seq, pack);
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
	AutoLock lock(mLock);

	if (mSendWin.enqueue(pack, &mLock, nonblock)) {
		flush();
		return true;
	}

	return false;
}

ssize_t UdpLink::send(struct cavan_udp_header *header)
{
	return mSock->send(this, header, sizeof(struct cavan_udp_header));
}

ssize_t UdpLink::sendResponse(struct cavan_udp_header *header, cavan_udp_pack_t type)
{
	struct cavan_udp_header response;
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

	count = mSendWin.flush(this, time);
	println("count = %d", count);
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
	pack->setSequence(0, CAVAN_UDP_WIN_SIZE);

	if (send(pack, true)) {
		return true;
	}

	delete pack;

	return false;
}

void UdpLink::processUdpPackData(struct cavan_udp_header *header, u16 length)
{
	if (mRecvWin.receive(this, header, length)) {
		sendResponse(header, CAVAN_UDP_DATA_ACK);
	}
}

void UdpLink::processUdpPackAck(struct cavan_udp_header *header, u16 length)
{
	mSendWin.confirm(this, header->sequence);
		flush();
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
	if (pack->getSendTimes() == 1) {
		u32 rtt = time - pack->getTime();
		u32 rttd = mRttd * 3 + RANG(mRtts, rtt);

		mRtts = (mRtts * 7 + rtt) >> 3;
		mRttd = rttd >> 2;
		mRto = (mRtts + rttd) << 1;

		if (mRto < CAVAN_UDP_RTO_MIN) {
			mRto = CAVAN_UDP_RTO_MIN;
		}

		println("RTT = %d, RTTS = %d, RTTD = %d, RTO = %d", rtt, mRtts, mRttd, mRto);

		if (mAcks < mCwnd) {
			mAcks++;
		} else {
			mAcks= 0;

			if (mCwnd < CAVAN_UDP_CWND) {
				mCwnd++;
			}
		}

		println("cwnd = %d, acks = %d", mCwnd, mAcks);
	}
}

UdpSock::UdpSock(void) : mSockfd(INVALID_SOCKET), mChannel(0)
{
	for (int i = 0; i < NELEM(mLinks); i++) {
		mLinks[i] = NULL;
	}

	mHead = NULL;
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
	UdpLink *link = mLinks[channel];
	if (link == NULL) {
		return;
	}

	mLinks[channel] = NULL;
	delete link;
}

void UdpSock::processUdpPackTest(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
}

void UdpSock::processUdpPackSync(struct cavan_udp_header *header, u16 length, struct sockaddr_in *addr)
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
	pack->setSequence(link->getReadSeq(), CAVAN_UDP_WIN_SIZE);
	link->send(pack, false);
}

void UdpSock::processUdpPackSyncAck1(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || header->src_channel == channel) {
		link->setRemoteChannel(header->src_channel);
		link->sendResponse(header, CAVAN_UDP_SYNC_ACK2);
		link->processUdpPackAck(header, length);
		onUdpConnected(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::processUdpPackSyncAck2(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || channel == header->src_channel) {
		link->setRemoteChannel(header->src_channel);
		link->processUdpPackAck(header, length);
		onUdpAccepted(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::processUdpPackData(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->processUdpPackData(header, length);
	}
}

void UdpSock::processUdpPackDataAck(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->processUdpPackAck(header, length);
	}
}

void UdpSock::processUdpPackError(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		recycle(link);
	}
}

void UdpSock::recvLoop(void)
{
	struct cavan_udp_header *header;
	struct sockaddr_in addr;
	char buff[1500];

	header = (struct cavan_udp_header *) buff;

	while (1) {
		int length = inet_recvfrom(mSockfd, buff, sizeof(buff), &addr);
		if (length < 0) {
			break;
		}

		println("inet_recvfrom: %d", length);
		println("type = %d", header->type);
		println("sequence = %d", header->sequence);
		println("src = %d", header->src_channel);
		println("dest = %d", header->dest_channel);

		switch (header->type) {
		case CAVAN_UDP_TEST:
			println("CAVAN_UDP_TEST");
			processUdpPackTest(header, length);
			break;

		case CAVAN_UDP_SYNC:
			println("CAVAN_UDP_SYNC");
			processUdpPackSync(header, length, &addr);
			break;

		case CAVAN_UDP_SYNC_ACK1:
			println("CAVAN_UDP_SYNC_ACK1");
			processUdpPackSyncAck1(header, length);
			break;

		case CAVAN_UDP_SYNC_ACK2:
			println("CAVAN_UDP_SYNC_ACK2");
			processUdpPackSyncAck2(header, length);
			break;

		case CAVAN_UDP_DATA:
			println("CAVAN_UDP_DATA");
			processUdpPackData(header, length);
			break;

		case CAVAN_UDP_DATA_ACK:
			println("CAVAN_UDP_DATA_ACK");
			processUdpPackDataAck(header, length);
			break;

		case CAVAN_UDP_ERROR:
			println("CAVAN_UDP_ERROR");
			processUdpPackError(header, length);
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
		UdpLink *link = mHead;
		if (link == NULL) {
			println("wait");
			mCond.waitLocked(&mLock);
			continue;
		}

		struct timespec time;
		u64 mseconds;

		clock_gettime_real(&time);
		mseconds = cavan_timespec_mseconds(&time);

		if (link->time > mseconds) {
			u32 delay = link->time - mseconds;
			cavan_timespec_add_ms(&time, delay);
			println("wait: %d", delay);
			mCond.waitLocked(&time, &mLock);
			continue;
		}

		println("link = %p", link);

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
