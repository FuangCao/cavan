#pragma once

/*
 * File:		EpollService.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-27 11:22:11
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
#include <cavan++/Thread.h>
#include <cavan++/EpollClient.h>

class EpollDaemon : public SimpleThread, public SimpleLink<EpollDaemon> {
private:
	EpollService *mService;

public:
	EpollDaemon(EpollService *service) : mService(service) {}
	virtual ~EpollDaemon() {}

protected:
	virtual void run(void);
};

class EpollService : public SimpleThread {
	friend EpollDaemon;

private:
	int mEpollFd;
	ThreadLock mLock;
	SimpleLinkQueue<EpollDaemon> mDaemonQueue;
	SimpleLinkQueue<EpollPacket> mPacketQueue;

public:
	EpollService(void) : mEpollFd(-1) {}
	virtual ~EpollService() {}

public:
	virtual int startEpollDaemon(void);
	virtual int doEpollCtrl(int op, int fd, u32 events, EpollClient *client);

	virtual int addEpollClient(int fd, u32 events, EpollClient *client) {
		return doEpollCtrl(EPOLL_CTL_ADD, fd, events, client);
	}

	virtual int modifyEpollClient(int fd, u32 events, EpollClient *client) {
		return doEpollCtrl(EPOLL_CTL_MOD, fd, events, client);
	}

	virtual int removeEpollClient(int fd) {
		return doEpollCtrl(EPOLL_CTL_DEL, fd, 0, NULL);
	}

	virtual void enqueueEpollPacket(EpollPacket *packet) {
		mPacketQueue.enqueue(packet);
	}

	virtual EpollPacket *dequeueEpollPacket(void) {
		return mPacketQueue.dequeue();
	}

protected:
	virtual void run(void);
	virtual void runEpollDaemon(void);
};
