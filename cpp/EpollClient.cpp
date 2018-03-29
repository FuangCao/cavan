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

int EpollPacket::writeTo(EpollClient *client)
{
	int wrlen = client->doEpollWrite(mData + mOffset, mLength - mOffset);
	if (wrlen > 0) {
		mOffset += wrlen;
	}

	return wrlen;
}

int EpollPacket::write(const void *buff, u16 length)
{
	u16 remain = mLength - mOffset;
	if (length > remain) {
		length = remain;
	}

	memcpy(mData, buff, length);

	return length;
}

// ================================================================================

int EpollClient::addEpollTo(EpollService *service)
{
	return service->addEpollClient(getEpollFd(), getEpollEventsRO(), this);
}

int EpollClient::removeEpollFrom(EpollService *service)
{
	return service->removeEpollClient(getEpollFd());
}

int EpollClient::setEpollReadonly(EpollService *service)
{
	return service->modifyEpollClient(getEpollFd(), getEpollEventsRO(), this);
}

int EpollClient::setEpollReadWrite(EpollService *service)
{
	return service->modifyEpollClient(getEpollFd(), getEpollEventsRW(), this);
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

	while (rdlen > 0) {
		int length = onEpollDataReceived(service, p, rdlen);
		if (length < 0) {
			return length;
		}

		p += length;
		rdlen -= length;
	}

	return rdlen;
}

int EpollClient::onEpollDataReceived(EpollService *service, const void *buff, int size)
{
	if (mRdPacket == NULL) {
		return onEpollHeaderReceived(buff, size);
	}

	int length = mRdPacket->write(buff, size);
	if (length > 0 && mRdPacket->isCompleted()) {
		mRdQueue.enqueue(mRdPacket);
		mRdPacket = NULL;
		service->enqueueEpollClient(this);
	}

	return length;
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
		if (length < 0) {
			return length;
		}
	}
}

void EpollClient::onEpollError(EpollService *service)
{
	removeEpollFrom(service);
}

int EpollClient::processEpollPackages(void)
{
	while (1) {
		EpollPacket *packet = mRdQueue.dequeue();
		if (packet == NULL) {
			break;
		}

		int ret = onEpollPacketReceived(packet);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}
