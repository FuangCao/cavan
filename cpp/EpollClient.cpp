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

void EpollClient::cleanup(void)
{
	EpollPacket *pack;

	pack = mWrHead;
	mWrHead = NULL;

	while (pack != NULL) {
		EpollPacket *next = pack->mNext;
		delete pack;
		pack = next;
	}
}

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
		bool pending = false;

		if (mEpollEvents & EPOLLIN) {
			int ret = onEpollIn(service);
			if (ret < 0) {
				break;
			}

			if (ret > 0) {
				pending = true;
			}
		}

		if (mEpollEvents & EPOLLOUT) {
			int ret = onEpollOut(service);
			if (ret < 0) {
				break;
			}

			if (ret > 0) {
				pending = true;
			}
		}

		return pending;
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

void EpollClientPacked::cleanup(void)
{
	EpollClient::cleanup();

	if (mData != NULL) {
		delete mData;
		mData = NULL;
		mSize = 0;
	}
}

int EpollClientPacked::onEpollDataReceived(EpollService *service, const void *buff, u16 size)
{
	if (mLength > 0) {
		u16 remain = mLength - mOffset;

		if (size < remain) {
			memcpy(mData + mOffset, buff, size);
			mOffset += size;
			return size;
		}

		memcpy(mData + mOffset, buff, remain);

		int ret = onEpollPackReceived(mData, mLength);
		if (ret < 0) {
			return ret;
		}

		mLength = 0;

		return remain;
	}

	bool completed = false;
	EpollBuffer *header = getEpollHeader();
	int wrlen = header->write(buff, size, completed);

	if (completed) {
		mLength = header->getLength();

		if (mLength > 0) {
			if (mLength > mSize) {
				if (mData != NULL) {
					delete mData;
				}

				for (mSize = 32; mSize < mLength; mSize <<= 1);

				mData = new char[mSize];
				if (mData == NULL) {
					return -ENOMEM;
				}
			}

			mOffset = 0;
		} else {
			int ret = onEpollPackReceived(mData, 0);
			if (ret < 0) {
				return ret;
			}
		}

		header->seek(0);
	}

	return wrlen;
}
