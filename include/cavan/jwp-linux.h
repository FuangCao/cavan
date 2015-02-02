#pragma once

/*
 * File:			jwp-linux.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-22 10:11:44
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

#define JWP_ARCH_NAME				"linux"

#define JWP_DEBUG					1
#define JWP_DEBUG_MEMBER			1
#define JWP_SHOW_ERROR				1
#define JWP_PRINTF_ENABLE			1
#define JWP_WRITE_LOG_ENABLE		1

#define JWP_POLL_ENABLE				0
#define JWP_SLEEP_ENABLE			1
#define JWP_CHECKSUM_ENABLE			1

#define JWP_QUEUE_ENABLE			1
#define JWP_TX_QUEUE_ENABLE			0
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	0
#define JWP_RX_DATA_QUEUE_ENABLE	0

#define JWP_TIMER_ENABLE			1
#define JWP_TX_TIMER_ENABLE			1
#define JWP_TX_DATA_TIMER_ENABLE	0
#define JWP_TX_PKG_TIMER_ENABLE		0
#define JWP_RX_PKG_TIMER_ENABLE		0

#define JWP_TX_LOOP_ENABLE			0
#define JWP_RX_LOOP_ENABLE			1
#define JWP_RX_PKG_LOOP_ENABLE		1
#define JWP_TX_DATA_LOOP_ENABLE		0

#define JWP_TX_NOTIFY_ENABLE		1
#define JWP_RX_CMD_NOTIFY_ENABLE	1
#define JWP_RX_DATA_NOTIFY_ENABLE	1
#define JWP_QUEUE_NOTIFY_ENABLE		1

#define JWP_MTU						128
#define JWP_POLL_TIME				10
#define JWP_TX_LATENCY				200
#define JWP_TX_RETRY_COUNT			20
#define JWP_TX_TIMEOUT				2000
#define JWP_QUEUE_SIZE				(JWP_MTU * 3)

// ============================================================

#define jwp_msleep(msec) \
	msleep(msec)

#define jwp_memcpy(dest, src, size) \
	memcpy(dest, src, size)

#define jwp_lock_init(lock) \
	pthread_mutex_init(&lock, NULL)

#define jwp_lock_acquire(lock) \
	pthread_mutex_lock(&lock)

#define jwp_lock_release(lock) \
	pthread_mutex_unlock(&lock)

#define jwp_signal_init(signal, available) \
	pthread_cond_init(&signal, NULL)

#define jwp_signal_wait_locked(signal, lock) \
	pthread_cond_wait(&signal, &lock)

#define jwp_signal_timedwait_locked(signal, lock, msec) \
	do { \
		long __msec; \
		struct timespec __ts; \
		clock_gettime(CLOCK_REALTIME, &__ts); \
		__msec = __ts.tv_nsec / 1000000L + msec; \
		__ts.tv_sec += __msec / 1000; \
		__ts.tv_nsec = (__msec % 1000) * 1000000L; \
		pthread_cond_timedwait(&signal, &lock, &__ts); \
	} while (0)

#define jwp_signal_notify_locked(signal, lock) \
	pthread_cond_signal(&signal)

// ============================================================

typedef u8 jwp_u8;
typedef u16 jwp_u16;
typedef u32 jwp_u32;
typedef u32 jwp_size_t;
typedef bool jwp_bool;
typedef pthread_cond_t jwp_signal_t;
typedef pthread_mutex_t jwp_lock_t;

#include <cavan/jwp.h>
#include <cavan/timer.h>

struct jwp_linux_desc
{
	struct jwp_desc jwp;

#if JWP_TIMER_ENABLE
	struct cavan_timer_service timer_service;
#endif
};

const char *jwp_device_to_string(jwp_device_t device);
jwp_bool jwp_linux_init(struct jwp_linux_desc *jwp_linux, void *data);
jwp_bool jwp_linux_start(struct jwp_linux_desc *jwp_linux);

static inline struct jwp_linux_desc *jwp_to_jwp_linux(struct jwp_desc *jwp)
{
	return (struct jwp_linux_desc *) jwp;
}
