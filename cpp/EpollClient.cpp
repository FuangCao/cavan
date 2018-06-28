/*
 * File:		EpollClient.cpp
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
#include <cavan++/EpollClient.h>
#include <cavan++/EpollService.h>

int EpollBuffer::write(const void *buff, u16 length, bool &completed)
{
	u16 remain = getRemain();

	if (length >= remain) {
		length = remain;
		completed = true;
	}

	memcpy(getDataHead(), buff, length);
	mOffset += length;

	return length;
}

int EpollBuffer::writeTo(EpollClient *client)
{
	u16 size = getSize();
	u16 remain = size - mOffset;
	int wrlen = client->doEpollWrite(getDataHead(), remain);

	if (wrlen < remain) {
		if (wrlen < 0) {
			return wrlen;
		}

		mOffset += wrlen;

		return 0;
	}

	mOffset = size;

	return 1;
}

int EpollBufferAuto::alloc(u16 length)
{
	if (mSize < length) {
		u16 size = 32;

		while (size < length) {
			size <<= 1;
		}

		char *data = new char[size];
		if (data == NULL) {
			return -ENOMEM;
		}

		if (mData != NULL) {
			memcpy(data, mData, mOffset);
			delete[] mData;
		}

		mData = data;
	}

	return 0;
}

int EpollBufferAuto::write(char c)
{
	int ret = alloc(mOffset + 1);
	if (ret < 0) {
		return ret;
	}

	mData[mOffset++] = c;

	return 1;
}

int EpollBufferAuto::write(const void *buff, u16 length, bool &completed)
{
	int ret = alloc(mOffset + length);
	if (ret < 0) {
		return ret;
	}

	memcpy(getDataHead(), buff, length);
	mOffset += length;

	return length;
}

// ================================================================================

int EpollClient::addEpollTo(EpollService *service)
{
	return service->addEpollClient(getEpollFd(), EPOLLIN | EPOLLOUT, this);
}

int EpollClient::removeEpollFrom(EpollService *service)
{
	return service->removeEpollClient(getEpollFd(), this);
}

void EpollClient::sendEpollPacket(EpollPacket *packet)
{
	packet->seek(0);
	packet->mNext = NULL;

	AutoLock lock(mWrLock);

	if (mWrHead == NULL) {
		mWrHead = packet;
		mEpollService->postEpollClient(this, 0);
	} else {
		mWrTail->mNext = packet;
	}

	mWrTail = packet;
}

bool EpollClient::onEpollEvent(EpollService *service)
{
	while ((mEpollEvents & (EPOLLERR | EPOLLHUP)) == 0) {
		int count = 0;

		if (mEpollEvents & EPOLLIN) {
			int ret = onEpollIn(service);
			if (ret < 0) {
				break;
			}

			if (ret > 0) {
				count++;
			}
		}

		if (mEpollEvents & EPOLLOUT) {
			int ret = onEpollOut(service);
			if (ret < 0) {
				break;
			}

			if (ret > 0) {
				count++;
			}
		}

		return (count > 0);
	}

	onEpollErr(service);

	return false;
}

int EpollClient::onEpollIn(EpollService *service)
{
	char buff[4096];
	int rdlen;

	rdlen = doEpollRead(buff, sizeof(buff));
	if (rdlen > 0) {
		char *p = buff;

		while (1) {
			int wrlen = onEpollDataReceived(service, p, rdlen);

			if (wrlen < rdlen) {
				if (wrlen < 0) {
					return wrlen;
				}

				rdlen -= wrlen;
				p += wrlen;
			} else {
				break;
			}
		}

		return 1;
	}

	if (rdlen < 0) {
		if (ERRNO_NOT_RETRY()) {
			return rdlen;
		}

		mEpollEvents &= ~EPOLLIN;

		return 0;
	}

	return -EFAULT;
}

int EpollClient::onEpollOut(EpollService *service)
{
	AutoLock lock(mWrLock);

	EpollPacket *head = mWrHead;
	if (head == NULL) {
		return 0;
	}

	int ret = head->writeTo(this);

	if (ret > 0) {
		mWrHead = head->mNext;
		delete head;
		return ret;
	}

	if (ret < 0) {
		if (ERRNO_NOT_RETRY()) {
			return ret;
		}

		mEpollEvents &= ~EPOLLOUT;
	}

	return 0;
}

void EpollClient::onEpollErr(EpollService *service)
{
	removeEpollFrom(service);
}
