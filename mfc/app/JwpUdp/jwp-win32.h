#pragma once

/*
 * File:			jwp-win32.h
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

#define JWP_DEBUG					0
#define JWP_DEBUG_MEMBER			1
#define JWP_SHOW_ERROR				1
#define JWP_PRINTF_ENABLE			1

#define JWP_POLL_ENABLE				0
#define JWP_SLEEP_ENABLE			1
#define JWP_CHECKSUM_ENABLE			1

#define JWP_QUEUE_ENABLE			1
#define JWP_TX_QUEUE_ENABLE			1
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	1
#define JWP_RX_DATA_QUEUE_ENABLE	1

#define JWP_TIMER_ENABLE			0
#define JWP_TX_TIMER_ENABLE			0
#define JWP_TX_DATA_TIMER_ENABLE	0
#define JWP_TX_PKG_TIMER_ENABLE		0
#define JWP_RX_PKG_TIMER_ENABLE		0

#define JWP_TX_LOOP_ENABLE			1
#define JWP_RX_LOOP_ENABLE			1
#define JWP_RX_PKG_LOOP_ENABLE		1
#define JWP_TX_DATA_LOOP_ENABLE		1

#define JWP_TX_NOTIFY_ENABLE		0
#define JWP_RX_CMD_NOTIFY_ENABLE	0
#define JWP_RX_DATA_NOTIFY_ENABLE	0
#define JWP_QUEUE_NOTIFY_ENABLE		0

#define JWP_MTU						0xFF
#define JWP_POLL_TIME				10
#define JWP_TX_LATENCY				200
#define JWP_TX_RETRY				10
#define JWP_TX_TIMEOUT				2000
#define JWP_QUEUE_SIZE				(JWP_MTU * 3)

// ============================================================

#define jwp_msleep(msec) \
	Sleep(msec)

#define jwp_memcpy(dest, src, size) \
	memcpy(dest, src, size)

#define jwp_lock_init(lock) \
	do { \
		lock = CreateMutex(NULL, false, NULL); \
	} while (0)

#define jwp_lock_acquire(lock) \
	WaitForSingleObject(lock, INFINITE)

#define jwp_lock_release(lock) \
	ReleaseMutex(lock)

#define jwp_signal_init(signal) \
	jwp_lock_init((signal).handle)

#define jwp_signal_timedwait_locked(signal, lock, msec) \
	do { \
		(signal).waitting = true; \
		jwp_lock_release(lock); \
		WaitForSingleObject((signal).handle, msec); \
		jwp_lock_acquire(lock); \
		(signal).waitting = false; \
	} while (0)

#define jwp_signal_wait_locked(signal, lock) \
	jwp_signal_timedwait_locked(signal, lock, INFINITE)

#define jwp_signal_notify_locked(signal, lock) \
	do { \
		if ((signal).waitting) { \
			jwp_lock_release((signal).handle); \
		} \
	} while (0)

// ============================================================

typedef BYTE jwp_u8;
typedef WORD jwp_u16;
typedef DWORD jwp_u32;
typedef DWORD jwp_size_t;
typedef bool jwp_bool;
typedef HANDLE jwp_lock_t;

typedef struct
{
	HANDLE handle;
	jwp_bool waitting;
} jwp_signal_t;
