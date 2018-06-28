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

#define EPOLL_DAEMON_MIN		20
#define EPOLL_DAEMON_MAX		200
#define EPOLL_SERVICE_DEBUG		0

class EpollService : public SimpleThread {
friend class EpollDaemon;
friend class EpollClient;

private:
	int mEpollFd;
	u16 mDaemonMax;
	u16 mDaemonMin;
	u16 mDaemonTotal;
	u16 mDaemonReady;
	ThreadLock mLock;
	Condition mCond;
	EpollClient *mEpollHead;
	EpollClient *mEpollTail;
	SimpleLinkQueue<EpollClient> mClientQueue;

	static void *EpollDaemonHandler(void *data) {
		static_cast<EpollService *>(data)->runEpollDaemon();
		return NULL;
	}

public:
	EpollService(u16 max = 200, u16 min = 20) : mEpollFd(-1), mDaemonMax(max), mDaemonMin(min), mDaemonTotal(0), mDaemonReady(0) {
		doEpollCreate();
	}

	virtual ~EpollService() {}

public:
	virtual int doEpollCreate(void);
	virtual int doEpollCtrl(int op, int fd, u32 events, EpollClient *client);
	virtual void postEpollClient(EpollClient *client, u32 events);

	virtual int addEpollClient(int fd, u32 events, EpollClient *client) {
		return doEpollCtrl(EPOLL_CTL_ADD, fd, events, client);
	}

	virtual int modifyEpollClient(int fd, u32 events, EpollClient *client) {
		return doEpollCtrl(EPOLL_CTL_MOD, fd, events, client);
	}

	virtual int removeEpollClient(int fd, EpollClient *client) {
		return doEpollCtrl(EPOLL_CTL_DEL, fd, 0, NULL);
	}

protected:
	virtual int onEpollStarted(void) {
		return 0;
	}

	virtual void onEpollStopped(void) {}

	virtual void run(void);
	virtual void runEpollDaemon(void);
	virtual void onEpollEvent(EpollClient *client, u32 events);
};
