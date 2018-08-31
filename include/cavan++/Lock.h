#pragma once

/*
 * File:		Lock.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-23 14:51:20
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/timer.h>

class ILock {
public:
	virtual ~ILock(void) {}
	virtual int acquire(void) = 0;
	virtual int release(void) = 0;

	virtual int tryLock(void) {
		return -ENOENT;
	}
};

class MutexLock : public ILock {
protected:
	pthread_mutex_t mLock;

public:
	MutexLock(void) {
		init();
	}

	~MutexLock(void) {
		pthread_mutex_destroy(&mLock);
	}

	virtual pthread_mutex_t *getLock(void) {
		return &mLock;
	}

	virtual int acquire(void) {
		return pthread_mutex_lock(&mLock);
	}

	virtual int release(void) {
		return pthread_mutex_unlock(&mLock);
	}

	virtual int tryLock() {
		return pthread_mutex_trylock(&mLock);
	}

protected:
	virtual int init(void) {
		return pthread_mutex_init(&mLock, NULL);
	}
};

class ThreadLock : public MutexLock {
protected:
	virtual int init(void);
};

class AutoLock {
private:
	ILock *mLock;

public:
	AutoLock(ILock *lock) : mLock(lock) {
		lock->acquire();
	}

	AutoLock(ILock &lock) : mLock(&lock) {
		lock.acquire();
	}

	virtual ~AutoLock() {
		mLock->release();
	}
};

class Condition {
private:
	pthread_cond_t mCond;

public:
	Condition(const pthread_condattr_t *attr = NULL) {
		pthread_cond_init(&mCond, attr);
	}

	virtual ~Condition(void) {
		pthread_cond_destroy(&mCond);
	}

	virtual int signal(void) {
		return pthread_cond_signal(&mCond);
	}

	virtual int broadcast(void) {
		return pthread_cond_broadcast(&mCond);
	}

	virtual int notify(void) {
		return signal();
	}

	virtual int notifyAll(void) {
		return broadcast();
	}

	virtual int waitLocked(MutexLock *lock) {
		return pthread_cond_wait(&mCond, lock->getLock());
	}

	virtual int wait(MutexLock *lock) {
		AutoLock alock(lock);
		return waitLocked(lock);
	}

	virtual int waitLocked(const struct timespec *abstime, MutexLock *lock) {
		return pthread_cond_timedwait(&mCond, lock->getLock(), abstime);
	}

	virtual int wait(const struct timespec *abstime, MutexLock *lock) {
		AutoLock alock(lock);
		return waitLocked(abstime, lock);
	}

	virtual int wait(u32 ms, MutexLock *lock) {
		struct timespec abstime;
		cavan_timer_set_timespec_ms(&abstime, ms);
		return wait(&abstime, lock);
	}
};
