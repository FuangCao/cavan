#pragma once

/*
 * File:			jwp-csr101x.h
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

#include <mem.h>
#include <timer.h>
#include <buf_utils.h>

#define JWP_ARCH_NAME				"csr101x"

#define JWP_DEBUG					0
#define JWP_DEBUG_MEMBER			0
#define JWP_SHOW_ERROR				1
#define JWP_PRINTF_ENABLE			1
#define JWP_WRITE_LOG_ENABLE		1

#define JWP_POLL_ENABLE				0
#define JWP_SLEEP_ENABLE			0
#define JWP_CHECKSUM_ENABLE			1

#define JWP_QUEUE_ENABLE			1
#define JWP_TX_QUEUE_ENABLE			1
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	1
#define JWP_RX_DATA_QUEUE_ENABLE	1

#define JWP_TIMER_ENABLE			1
#define JWP_TX_TIMER_ENABLE			1
#define JWP_TX_DATA_TIMER_ENABLE	1
#define JWP_TX_PKG_TIMER_ENABLE		1
#define JWP_RX_PKG_TIMER_ENABLE		1

#define JWP_TX_LOOP_ENABLE			0
#define JWP_RX_LOOP_ENABLE			0
#define JWP_RX_PKG_LOOP_ENABLE		0
#define JWP_TX_DATA_LOOP_ENABLE		0

#define JWP_TX_NOTIFY_ENABLE		0
#define JWP_RX_CMD_NOTIFY_ENABLE	0
#define JWP_RX_DATA_NOTIFY_ENABLE	0
#define JWP_QUEUE_NOTIFY_ENABLE		0

#define JWP_MTU						128
#define JWP_POLL_TIME				10
#define JWP_TX_LATENCY				200
#define JWP_TX_RETRY_COUNT			10
#define JWP_TX_TIMEOUT				2000
#define JWP_QUEUE_SIZE				(JWP_MTU + 1)

// ============================================================

#define jwp_msleep(msec)

#define jwp_memcpy(dest, src, size) \
	MemCopy(dest, src, size)

#define jwp_lock_init(lock)

#define jwp_lock_acquire(lock)

#define jwp_lock_release(lock)

#define jwp_signal_init(signal)

#define jwp_signal_wait_locked(signal, lock)

#define jwp_signal_timedwait_locked(signal, lock, msec)

#define jwp_signal_notify_locked(signal, lock)

// ============================================================

typedef unsigned char jwp_u8;
typedef unsigned long jwp_u16;
typedef unsigned long jwp_u32;
typedef unsigned long jwp_size_t;

typedef enum
{
    false = 0,
    true
} jwp_bool;

typedef jwp_u8 jwp_signal_t;
typedef jwp_u8 jwp_lock_t;

#include "jwp.h"

#define JWP_CSR_CMD_MAX_LEN			64

typedef enum
{
	JWP_CSR_CMD_SET_STATE,
	JWP_CSR_CMD_SET_FACTORY_SCAN,
	JWP_CSR_CMD_SET_WHITE_LIST,
	JWP_CSR_CMD_SET_WHITE_LIST_ENABLE,
	JWP_CSR_CMD_GET_STATE,
	JWP_CSR_CMD_GET_FIRMWARE_INFO,
	JWP_CSR_CMD_RM_PAIR,
	JWP_CSR_RESPONSE,
	JWP_CSR_EVENT_STATE,
	JWP_CSR_EVENT_FIRMWARE_INFO,
} jwp_csr_command_t;

struct jwp_csr_header
{
	jwp_u8 type;
};

struct jwp_csr_command_set_state
{
	struct jwp_csr_header header;
	jwp_u8 state;
};

struct jwp_csr_command_set_white_list
{
	struct jwp_csr_header header;
	jwp_u8 addr_list[5][6];
};

struct jwp_csr_command_set_white_list_enable
{
	struct jwp_csr_header header;
	jwp_u8 enable;
};

struct jwp_csr_command_set_factory_scan
{
	struct jwp_csr_header header;
	jwp_u8 mac_addr[6];
};

struct jwp_csr_response_package
{
	struct jwp_csr_header header;
	jwp_u8 success;
};

struct jwp_csr_event_state
{
	struct jwp_csr_header header;
	jwp_u8 state;
	jwp_u8 bonded;
	jwp_u8 bonded_addr[6];
};

struct jwp_csr_event_firmware_info
{
	struct jwp_csr_header header;
	jwp_u8 firmware_version;
	jwp_u8 mac_addr[6];
};

jwp_bool jwp_csr_init(struct jwp_desc *jwp);
jwp_size_t jwp_csr_build_response(struct jwp_csr_response_package *rsp, jwp_u8 success);
jwp_size_t jwp_csr_build_state_response(struct jwp_csr_event_state *rsp);

static inline jwp_bool jwp_csr_send_command(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	return jwp_send_command(jwp, command, size);
}

static inline jwp_bool jwp_csr_send_empty_command(struct jwp_desc *jwp, jwp_u8 type)
{
	return jwp_csr_send_command(jwp, &type, 1);
}
