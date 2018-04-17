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

int EpollBuffer::write(const void *buff, u16 length)
{
	u16 remain = getRemain();
	if (length > remain) {
		length = remain;
	}

	memcpy(getDataHead(), buff, length);
	mOffset += length;

	return length;
}

int EpollBuffer::writeTo(EpollClient *client)
{
	int wrlen = client->doEpollWrite(getDataHead(), getRemain());
	if (wrlen > 0) {
		mOffset += wrlen;
	}

	return wrlen;
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

int EpollBufferAuto::write(const void *buff, u16 length)
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
	return service->addEpollClient(getEpollFd(), getEpollEventsRO(), this);
}

int EpollClient::removeEpollFrom(EpollService *service)
{
	return service->removeEpollClient(getEpollFd(), this);
}

int EpollClient::setEpollReadonly(EpollService *service)
{
	return service->modifyEpollClient(getEpollFd(), getEpollEventsRO(), this);
}

int EpollClient::setEpollReadWrite(EpollService *service)
{
	return service->modifyEpollClient(getEpollFd(), getEpollEventsRW(), this);
}

int EpollClient::sendEpollPacket(EpollPacket *packet)
{
	return -ENOENT;
}

void EpollClient::onEpollEvent(EpollService *service, u32 events)
{
	while ((events & (EPOLLERR | EPOLLHUP)) == 0) {
		if ((events & EPOLLIN) && onEpollIn(service) < 0) {
			break;
		}

		if ((events & EPOLLOUT) && onEpollOut(service) < 0) {
			break;
		}

		return;
	}

	onEpollError(service);
}

int EpollClient::onEpollIn(EpollService *service)
{
	char buff[4096], *p = buff;
	int rdlen = doEpollRead(buff, sizeof(buff));

	if (rdlen <= 0) {
		return -EFAULT;
	}

	while (1) {
		int length = onEpollDataReceived(service, p, rdlen);
		if (length < rdlen) {
			if (length < 0) {
				return length;
			}

			p += length;
			rdlen -= length;
		} else {
			break;
		}
	}

	return 0;
}

int EpollClient::onEpollDataReceived(EpollService *service, const void *buff, int size)
{
	int length;

	if (mRdPacket == NULL) {
		EpollBuffer *header = getEpollHeader();
		if (header == NULL) {
			return -ENOMEM;
		}

		length = header->write(buff, size);
		if (header->isCompleted()) {
			mRdPacket = onEpollHeaderReceived(header);
			if (mRdPacket == NULL) {
				return -ENOMEM;
			}
		} else {
			return length;
		}
	} else {
		length = mRdPacket->write(buff, size);
	}

	if (mRdPacket->isCompleted()) {
		mRdQueue.enqueue(mRdPacket);
		mRdPacket = NULL;
		service->enqueueEpollClient(this);
	}

	return length;
}

EpollPacket *EpollClient::onEpollHeaderReceived(EpollBuffer *header)
{
	EpollPacket *packet;

	packet = new EpollPacket(header->getLength());
	header->reset();
	return packet;
}

int EpollClient::onEpollOut(EpollService *service)
{
	while (1) {
		if (mWrPacket == NULL) {
			mWrPacket = mWrQueue.dequeue();
			if (mWrPacket == NULL) {
				return setEpollReadonly(service);
			}
		}

		int length = mWrPacket->writeTo(this);
		if (length <= 0) {
			return length;
		}

		if (mWrPacket->isCompleted()) {
			delete mWrPacket;
			mWrPacket = NULL;
		}
	}
}

void EpollClient::onEpollError(EpollService *service)
{
	removeEpollFrom(service);
}

bool EpollClientQueue::enqueue(EpollClient *client)
{
	AutoLock lock(mLock);

	if (mHead != NULL) {
		if (client->isUsed()) {
			return false;
		}

		mHead->append(client);
	} else {
		// client->reset();
		mHead = client;
	}

	mCond.signal();

	return true;
}

EpollClient *EpollClientQueue::dequeueLocked(void)
{
	EpollClient *client = mHead;
	if (client == NULL) {
		return NULL;
	}

	EpollClient *next = client->getNext();
	if (next == client) {
		mHead = NULL;
	} else {
		client->remove();
		mHead = next;
	}

	return client;
}

EpollClient *EpollClientQueue::dequeue(void)
{
	AutoLock lock(mLock);
	return dequeueLocked();
}

void EpollClientQueue::removeLocked(EpollClient *client)
{
	if (client == mHead) {
		EpollClient *next = client->getNext();
		if (next == client) {
			mHead = NULL;
		} else {
			client->remove();
			mHead = next;
		}
	} else {
		client->remove();
	}
}

void EpollClientQueue::remove(EpollClient *client)
{
	AutoLock lock(mLock);
	removeLocked(client);
}

void EpollClientQueue::processPackets(EpollService *service, EpollDaemon *daemon)
{
	mLock.acquire();

	while (1) {
		while (mHead == NULL) {
			if (service->onEpollDaemonReady(daemon)) {
				goto out_exit;
			}

			mCond.waitLocked(mLock);
			service->onEpollDaemonBusy(daemon);
		}

		EpollClient *client = mHead;
		mHead = client->getNext();

		EpollPacket *packet = client->dequeueEpollPacket();
		if (packet == NULL) {
			if (client == mHead) {
				mHead = NULL;
			} else {
				client->remove();
			}
		} else {
			mLock.release();
			client->onEpollPacketReceived(packet);
			delete packet;
			mLock.acquire();
		}
	}

out_exit:
	mLock.release();
}
