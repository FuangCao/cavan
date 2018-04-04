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
	friend class EpollDaemon;
	friend class EpollClient;
	friend class EpollClientQueue;

private:
	int mEpollFd;
	u16 mDaemons;
	u16 mIdleDaemons;
	ThreadLock mLock;
	EpollClientQueue mClientQueue;
	SimpleLinkQueue<EpollDaemon> mDaemonQueue;

public:
	EpollService(void) : mEpollFd(-1), mDaemons(0), mIdleDaemons(0) {}
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

	virtual int removeEpollClient(int fd, EpollClient *client) {
		int ret = doEpollCtrl(EPOLL_CTL_DEL, fd, 0, NULL);
		mClientQueue.remove(client);
		return ret;
	}

	virtual void enqueueEpollClient(EpollClient *client) {
		if (mClientQueue.enqueue(client)) {
			AutoLock lock(mLock);
			if (mIdleDaemons == 0 && mDaemons < EPOLL_DAEMON_MAX) {
				startEpollDaemon();
			}
		}
	}

protected:
	virtual int onEpollStarted(void) {
		return 0;
	}

	virtual void onEpollDaemonStarted(EpollDaemon *daemon) {
		AutoLock lock(mLock);
		mDaemons++;

#if EPOLL_SERVICE_DEBUG
		println("onEpollDaemonStarted: %d", mDaemons);
#endif
	}

	virtual void onEpollDaemonStopped(EpollDaemon *daemon) {
		AutoLock lock(mLock);
		mDaemons--;

#if EPOLL_SERVICE_DEBUG
		println("onEpollDaemonStopped: %d", mDaemons);
#endif
	}

	virtual bool onEpollDaemonReady(EpollDaemon *daemon) {
		AutoLock lock(mLock);

		if (mIdleDaemons > EPOLL_DAEMON_MIN) {
			return true;
		}

		mIdleDaemons++;

#if EPOLL_SERVICE_DEBUG
		println("onEpollDaemonReady: %d", mIdleDaemons);
#endif

		return false;
	}

	virtual void onEpollDaemonBusy(EpollDaemon *daemon) {
		AutoLock lock(mLock);
		mIdleDaemons--;

#if EPOLL_SERVICE_DEBUG
		println("onEpollDaemonBusy: %d", mIdleDaemons);
#endif
	}

	virtual void onEpollStopped(void) {}

	virtual void run(void);
	virtual void runEpollDaemon(EpollDaemon *daemon);
};
