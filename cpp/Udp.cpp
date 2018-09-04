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

ssize_t UdpPack::send(UdpLink *link)
{
	u16 length = sizeof(struct cavan_udp_header) + mLength;
	return link->getSock()->send(link, mHeader, length);
}

UdpWin::UdpWin(void) : mLength(0), mSequence(0)
{
	for (int i = 0; i < NELEM(mPacks); i++) {
		mPacks[i] = NULL;
	}
}

u8 UdpWin::getWin(u16 sequence)
{
	u16 end = mSequence + CAVAN_UDP_WIN_SIZE;

	if (sequence > end) {
		return 0xFFFF - sequence + end;
	}

	return end - sequence;
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

int UdpWin::confirm(u16 sequence)
{
	u16 index = sequence % NELEM(mPacks);
	UdpPack *pack = mPacks[index];

	if (pack == NULL || pack->getSequence() != sequence) {
		return -EINVAL;
	}

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

	return count;
}

int UdpWin::receive(const struct cavan_udp_header *header, u16 length)
{
	u16 sequence = header->sequence;
	if (invalid(sequence)) {
		return -EINVAL;
	}

	u16 index = sequence % NELEM(mPacks);
	UdpPack *pack = mPacks[index];

	if (pack != NULL) {
		if (pack->getSequence() != sequence) {
			return -EINVAL;
		}

		return 0;
	}

	pack = UdpPack::alloc(header->data, length);
	if (pack == NULL) {
		return -ENOMEM;
	}

	memcpy(pack->getHeader(), header, sizeof(struct cavan_udp_header));
	mPacks[index] = pack;

	int count = 0;

	while (mLength < NELEM(mPacks)) {
		index = (mSequence + mLength) % NELEM(mPacks);
		if (mPacks[index] == NULL) {
			break;
		}

		mLength++;

		if (mLength == 1) {
			mCond.signal();
		}

		count++;
	}

	return count;
}

int UdpWin::flush(UdpLink *link, u16 sequence)
{
	println("flush: %d -> %d", mSequence, sequence);

	for (u16 index = mSequence; index != sequence; index++) {
		UdpPack *pack = mPacks[index % NELEM(mPacks)];
		println("pack = %p", pack);
		if (pack == NULL) {
			continue;
		}

		pack->send(link);
	}

	return 0;
}

ssize_t UdpLink::send(const void *buff, size_t size, bool nonblock)
{
	UdpPack *pack = UdpPack::alloc(buff, size);
	if (pack == NULL) {
		return -ENOMEM;
	}

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
		mSock->post(this, 0);
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
	response.win = mRecvWin.getWin(sequence);
	response.dest_channel = header->src_channel;
	response.src_channel = header->dest_channel;

	return send(&response);
}

ssize_t UdpLink::recv(void *buff, size_t size, bool nonblock)
{
	AutoLock lock(mLock);

	UdpPack *pack = mRecvWin.dequeue(&mLock, nonblock);
	if (pack == NULL) {
		return -EAGAIN;
	}

	size = pack->read(buff, size);
	delete pack;

	return size;
}

void UdpLink::onUdpData(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
}

void UdpLink::onUdpWin(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
}

void UdpLink::onUdpPing(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
}

void UdpLink::onUdpDataAck(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
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
	UdpLink *sock = new UdpLink(this, addr, channel);
	if (sock == NULL) {
		return NULL;
	}

	mLinks[channel] = sock;

	return sock;
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

ssize_t UdpSock::send(u16 channel, const void *buff, size_t size, bool nonblock)
{
	AutoLock lock(mLock);

	UdpLink *link = mLinks[channel];
	if (link == NULL) {
		return -ENOENT;
	}

	return link->send(buff, size, nonblock);
}

ssize_t UdpSock::recv(u16 channel, void *buff, size_t size, bool nonblock)
{
	AutoLock lock(mLock);

	UdpLink *link = mLinks[channel];
	if (link == NULL) {
		return -ENOENT;
	}

	return link->recv(buff, size, nonblock);
}

void UdpSock::onUdpTest(struct cavan_udp_header *header, u16 length)
{
	pr_pos_info();
}

void UdpSock::onUdpSync(struct cavan_udp_header *header, u16 length, struct sockaddr_in *addr)
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

void UdpSock::onUdpSyncAck1(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || header->src_channel == channel) {
		link->setRemoteChannel(header->src_channel);
		link->sendResponse(header, CAVAN_UDP_SYNC_ACK2);
		onUdpConnected(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::onUdpSyncAck2(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link == NULL) {
		return;
	}

	u16 channel = link->getRemoteChannel();

	if (channel == 0 || channel == header->src_channel) {
		link->setRemoteChannel(header->src_channel);
		onUdpAccepted(link);
	} else {
		link->sendResponse(header, CAVAN_UDP_ERROR);
	}
}

void UdpSock::onUdpData(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->onUdpData(header, length);
	}
}

void UdpSock::onUdpWin(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->onUdpWin(header, length);
	}
}

void UdpSock::onUdpPing(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->onUdpPing(header, length);
	}
}

void UdpSock::onUdpDataAck(struct cavan_udp_header *header, u16 length)
{
	UdpLink *link = getLink(header);
	if (link != NULL) {
		link->onUdpDataAck(header, length);
	}
}

void UdpSock::onUdpError(struct cavan_udp_header *header, u16 length)
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
		println("win = %d", header->win);
		println("src = %d", header->src_channel);
		println("dest = %d", header->dest_channel);

		switch (header->type) {
		case CAVAN_UDP_TEST:
			println("CAVAN_UDP_TEST");
			onUdpTest(header, length);
			break;

		case CAVAN_UDP_SYNC:
			println("CAVAN_UDP_SYNC");
			onUdpSync(header, length, &addr);
			break;

		case CAVAN_UDP_SYNC_ACK1:
			println("CAVAN_UDP_SYNC_ACK1");
			onUdpSyncAck1(header, length);
			break;

		case CAVAN_UDP_SYNC_ACK2:
			println("CAVAN_UDP_SYNC_ACK2");
			onUdpSyncAck2(header, length);
			break;

		case CAVAN_UDP_DATA:
			println("CAVAN_UDP_DATA");
			onUdpData(header, length);
			break;

		case CAVAN_UDP_WIND:
			println("CAVAN_UDP_WIND");
			onUdpWin(header, length);
			break;

		case CAVAN_UDP_PING:
			println("CAVAN_UDP_PING");
			onUdpPing(header, length);
			break;

		case CAVAN_UDP_DATA_ACK:
			println("CAVAN_UDP_DATA_ACK");
			onUdpDataAck(header, length);
			break;

		case CAVAN_UDP_ERROR:
			println("CAVAN_UDP_ERROR");
			onUdpError(header, length);
			break;

		default:
			pr_err_info("Invalid type: %d", header->type);
		}
	}
}

void UdpSock::post(UdpLink *link, u64 time)
{
	AutoLock lock(mLock);

	pr_pos_info();

	if (link->next != link) {
		return;
	}

	UdpLink **pp = &mHead;

	while (1) {
		UdpLink *p = *pp;

		if (p == NULL) {
			link->next = NULL;
			break;
		}

		if (p->time > time) {
			link->next = p;
			break;
		}

		pp = &p->next;
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

		mHead = link->next;
		if (mHead == link) {
			mHead = NULL;
		}

		link->next = link;

		mLock.release();
		link->flush();
		mLock.acquire();
	}

	mLock.release();
}

int UdpSock::connect(struct sockaddr_in *addr)
{
	UdpLink *link = alloc(addr);
	if (link == NULL) {
		return -ENOMEM;
	}

	UdpPack *pack = UdpPack::alloc(NULL, 0);
	if (pack == NULL) {
		goto out_recycle_link;
	}

	pack->setHeader(CAVAN_UDP_SYNC, link->getLocalChannel(), 0);
	pack->setSequence(0, CAVAN_UDP_WIN_SIZE);

	if (link->send(pack, true)) {
		return 0;
	}

	delete pack;

out_recycle_link:
	recycle(link);
	return -EFAULT;
}

int UdpSock::connect(const char *host, u16 port)
{
	struct sockaddr_in addr;

	inet_sockaddr_init(&addr, host, port);

	return connect(&addr);
}

int UdpSock::connect(const char *url)
{
	struct sockaddr_in addr;

	inet_sockaddr_init_url(&addr, url);

	return connect(&addr);
}
