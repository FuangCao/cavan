/*
 * File:		EpollService.cpp
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
#include <cavan++/EpollService.h>

int EpollService::doEpollCreate(void)
{
	AutoLock lock(mLock);

	if (mEpollFd < 0) {
		mEpollFd = epoll_create(0xFFFF);
	}

	return mEpollFd;
}

int EpollService::doEpollCtrl(int op, int fd, u32 events, EpollClient *client)
{
	struct epoll_event event;

	event.events = events | EPOLLHUP | EPOLLERR | EPOLLET;
	event.data.ptr = client;

	AutoLock lock(mLock);

	return epoll_ctl(mEpollFd, op, fd, &event);
}

void EpollService::postEpollClient(EpollClient *client, u32 events)
{
	AutoLock lock(mLock);

	client->setEpollEvents(events);

	if (client->mEpollNext == client) {
		client->mEpollNext = NULL;

		if (mEpollHead == NULL) {
			mEpollHead = client;
			mCond.notify();
		} else {
			mEpollTail->mEpollNext = client;
		}

		mEpollTail = client;

		if (mDaemonReady == 0 && mDaemonTotal < mDaemonMax) {
			cavan_pthread_run(EpollDaemonHandler, this);
		}
	}
}

void EpollService::run(void)
{
	int fd;

	while ((fd = doEpollCreate()) < 0) {
		pr_err_info("epoll_create: %d", fd);
		msleep(2000);
	}

	if (onEpollStarted() < 0) {
		goto out_exit;
	}

	while (1) {
		struct epoll_event *p, *p_end;
		struct epoll_event events[10];
		int count;

		count = epoll_wait(fd, events, NELEM(events), -1);
		if (count < 0) {
			pr_err_info("epoll_wait: %d", count);

			if (ERRNO_NOT_RETRY()) {
				break;
			}

			continue;
		}

		for (p = events, p_end = p + count; p < p_end; p++) {
			onEpollEvent((EpollClient *) p->data.ptr, p->events);
		}
	}

	onEpollStopped();

out_exit:
	if (fd == mEpollFd) {
		mEpollFd = -1;
		::close(fd);
	}

	pr_red_info("epoll thread exit!");
}

void EpollService::runEpollDaemon(void)
{
	mLock.acquire();

	mDaemonTotal++;

#if EPOLL_SERVICE_DEBUG
	println("started (%d/%d)", mDaemonTotal - mDaemonReady, mDaemonTotal);
#endif

	while (1) {
		EpollClient *client = mEpollHead;

		if (client == NULL) {
			if (mDaemonReady > mDaemonMin) {
				break;
			}

			mDaemonReady++;

#if EPOLL_SERVICE_DEBUG
			println("ready (%d/%d)", mDaemonTotal - mDaemonReady, mDaemonTotal);
#endif

			mCond.waitLocked(mLock);

			mDaemonReady--;

#if EPOLL_SERVICE_DEBUG
			println("busy (%d/%d)", mDaemonTotal - mDaemonReady, mDaemonTotal);
#endif

			continue;
		}

		mEpollHead = client->mEpollNext;
		client->setEpollPending(false);

		mLock.release();

		int ret = client->onEpollEvent();
		if (ret < 0) {
			client->onEpollErr();
			mLock.acquire();
		} else {
			mLock.acquire();

			if (ret > 0 || client->isEpollPending()) {
				if (mEpollHead == NULL) {
					mEpollHead = client;
				} else {
					mEpollTail->mEpollNext = client;
				}

				mEpollTail = client;
				client->mEpollNext = NULL;
			} else {
				client->mEpollNext = client;
			}
		}
	}

	mDaemonTotal--;

#if EPOLL_SERVICE_DEBUG
	println("stopped (%d/%d)", mDaemonTotal - mDaemonReady, mDaemonTotal);
#endif

	mLock.release();
}

void EpollService::onEpollEvent(EpollClient *client, u32 events)
{
#if EPOLL_SERVICE_DEBUG
	println("onEpollEvent: %08x", events);
#endif

	postEpollClient(client, events);
}
