/*
 * File:		testLock.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-09 11:22:35
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan++/Lock.h>
#include <cavan++/Thread.h>

static int TestLockThreadHandler(CavanThread *thread)
{
	ThreadLock *lock = (ThreadLock *) thread->getData();

	lock->acquire();
	pr_pos_info();
	msleep(500);

	pr_pos_info();
	lock->acquire();
	pr_pos_info();
	msleep(500);

	pr_pos_info();
	lock->release();
	pr_pos_info();
	msleep(500);

	pr_pos_info();
	lock->release();
	pr_pos_info();
	msleep(500);

	return 0;
}

int main(int argc, char *argv[])
{
	ThreadLock lock;
	CavanThread thread("TEST-LOCK", TestLockThreadHandler);

	thread.setData(&lock);
	thread.start();

	while (1)
	{
		lock.acquire();
		pr_pos_info();
		msleep(500);

		pr_pos_info();
		lock.acquire();
		pr_pos_info();
		msleep(500);

		pr_pos_info();
		lock.release();
		pr_pos_info();
		msleep(500);

		pr_pos_info();
		lock.release();
		pr_pos_info();
		msleep(500);
	}

	return 0;
}
