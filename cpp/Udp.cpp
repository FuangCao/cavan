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

u16 UdpPack::copyFrom(const void *buff, u16 size)
{
	if (size > mLength) {
		size = mLength;
	}

	memcpy(mHeader->data, buff, size);

	return size;
}

u16 UdpPack::copyTo(void *buff, u16 size)
{
	if (size > mLength) {
		size = mLength;
	}

	memcpy(buff, mHeader->data, size);

	return size;
}

UdpWin::UdpWin(void) : mLength(0), mSequence(0)
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

ssize_t UdpLink::send(const void *buff, size_t size, bool nonblock)
{
	UdpPack *pack = UdpPack::alloc(buff, size);
	if (pack == NULL) {
		return -ENOMEM;
	}

	AutoLock lock(mLock);

	if (mSendWin.enqueue(pack, &mLock, nonblock)) {
		return size;
	}

	delete pack;

	return -EAGAIN;
}

ssize_t UdpLink::recv(void *buff, size_t size, bool nonblock)
{
	AutoLock lock(mLock);

	UdpPack *pack = mRecvWin.dequeue(&mLock, nonblock);
	if (pack == NULL) {
		return -EAGAIN;
	}

	size = pack->copyTo(buff, size);
	delete pack;

	return size;
}

UdpSock::UdpSock(void) : mSockfd(INVALID_SOCKET), mChannel(0)
{
	for (int i = 0; i < NELEM(mLinks); i++) {
		mLinks[i] = NULL;
	}
}

int UdpSock::open(u16 port)
{
	int sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	struct sockaddr_in addr;

	inet_sockaddr_init(&addr, NULL, port);

	int ret = inet_bind(sockfd, &addr);
	if (ret < 0) {
		close(sockfd);
		return ret;
	}

	mSockfd = sockfd;

	return sockfd;
}

UdpLink *UdpSock::alloc(void)
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
	UdpLink *sock = new UdpLink(channel);
	if (sock == NULL) {
		return NULL;
	}

	mLinks[channel] = sock;

	return sock;
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
