#pragma once

/*
 * File:			jwp-kl2x.h
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

#include "stdio.h"
#include "mqxlite.h"
#include "string.h"

#define JWP_ARCH_NAME				"kl2x"

#define JWP_DEBUG					0
#define JWP_DEBUG_MEMBER			0
#define JWP_SHOW_ERROR				0
#define JWP_PRINTF_ENABLE			0
#define JWP_WRITE_LOG_ENABLE		0

#define JWP_POLL_ENABLE				1
#define JWP_SLEEP_ENABLE			1
#define JWP_CHECKSUM_ENABLE			1

#define JWP_TX_INTERRUPT_ENABLE		1
#define JWP_RX_IMTERRUPT_ENABLE		1

#define JWP_TX_HW_QUEUE_ENABLE		1
#define JWP_TX_QUEUE_ENABLE			0
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	0
#define JWP_RX_DATA_QUEUE_ENABLE	1

#define JWP_TX_TIMER_ENABLE			0
#define JWP_TX_DATA_TIMER_ENABLE	0
#define JWP_TX_PKG_TIMER_ENABLE		0
#define JWP_RX_PKG_TIMER_ENABLE		0

#define JWP_TX_LOOP_ENABLE			0
#define JWP_RX_LOOP_ENABLE			0
#define JWP_RX_PKG_LOOP_ENABLE		1
#define JWP_TX_DATA_LOOP_ENABLE		0

#define JWP_TX_NOTIFY_ENABLE		1
#define JWP_RX_CMD_NOTIFY_ENABLE	1
#define JWP_RX_DATA_NOTIFY_ENABLE	1
#define JWP_QUEUE_NOTIFY_ENABLE		1

#define JWP_MTU						64
#define JWP_POLL_TIME				2
#define JWP_TX_LATENCY				50
#define JWP_TX_RETRY_COUNT			20
#define JWP_TX_TIMEOUT				200
#define JWP_QUEUE_SIZE				(JWP_MTU * 2)

// ============================================================

#define jwp_msleep(msec) \
	_time_delay_ticks(msec)

#define jwp_memcpy(dest, src, size) \
	memcpy(dest, src, size)

#define jwp_lock_init(lock) \
	_lwsem_create(&(lock), 1)

#define jwp_lock_acquire(lock) \
	_lwsem_wait(&(lock))

#define jwp_lock_release(lock) \
	_lwsem_post(&(lock))

#define jwp_signal_init(signal, available) \
	do { \
		(signal).wait_count = 0; \
		_lwsem_create(&(signal).sem, 0); \
	} while (0)

#define jwp_signal_wait_locked(signal, lock) \
	jwp_kl2x_signal_timedwait(&(signal), &(lock), 0)

#define jwp_signal_timedwait_locked(signal, lock, msec) \
	jwp_kl2x_signal_timedwait(&(signal), &(lock), msec)

#define jwp_signal_notify_locked(signal, lock) \
	do { \
		if ((signal).wait_count > 0) { \
			_lwsem_post(&(signal).sem); \
		} \
	} while (0)

#define jwp_irq_enable() \
	_int_enable()

#define jwp_irq_disable() \
	_int_disable()

// ============================================================

typedef unsigned char jwp_u8;
typedef unsigned short jwp_u16;
typedef unsigned long jwp_u32;
typedef jwp_u32 jwp_size_t;
typedef LWSEM_STRUCT jwp_lock_t;

typedef enum {
	false = 0,
	true = 1,
} jwp_bool;

typedef struct {
	LWSEM_STRUCT sem;
	jwp_u32 wait_count;
} jwp_signal_t;

void jwp_kl2x_signal_timedwait(jwp_signal_t *signal, jwp_lock_t *lock, jwp_u32 msec);

#include "jwp.h"

// ============================================================

#define BT_UART_NUM				2

extern struct jwp_mcu_desc kl2x_jwp_mcu;

jwp_bool kl2x_jwp_init(void);
