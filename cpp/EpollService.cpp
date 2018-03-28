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

void EpollDaemon::run(void)
{
	mService->runEpollDaemon();
}

int EpollService::doEpollCtrl(int op, int fd, u32 events, EpollClient *client)
{
	struct epoll_event event;

	event.events = events;
	event.data.ptr = client;

	AutoLock lock(mLock);

	return epoll_ctl(mEpollFd, op, fd, &event);
}

int EpollService::startEpollDaemon(void)
{
	EpollDaemon *daemon = new EpollDaemon(this);
	if (daemon == NULL) {
		return -ENOMEM;
	}

	int ret = daemon->start();
	if (ret < 0) {
		delete daemon;
		return ret;
	}

	mDaemonQueue.enqueue(daemon);

	return 0;
}

void EpollService::run(void)
{
	int fd;

	while ((fd = epoll_create(0xFFFF)) < 0) {
		pr_err_info("epoll_create: %d", fd);
		msleep(2000);
	}

	mEpollFd = fd;

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
			EpollClient *client = (EpollClient *) p->data.ptr;
			client->onEpollEvent(this, p->events);
		}
	}

	if (fd == mEpollFd) {
		mEpollFd = -1;
		::close(fd);
	}

	pr_red_info("epoll thread exit!");
}

void EpollService::runEpollDaemon(void)
{
	while (1) {
		EpollPacket *packet = dequeueEpollPacket();
		EpollClient *client = packet->getClient();

		if (client->onEpollPacketReceived(packet) < 0) {
			client->onEpollError(this);
		}

		delete packet;
	}
}
