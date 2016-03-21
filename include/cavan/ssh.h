#pragma once

/*
 * File:		ssh.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-03-11 10:55:33
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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

#define CAVAN_SSH_VERSION	0x20160311

typedef enum {
	CAVAN_SSH_REQ_NOOP,
	CAVAN_SSH_REQ_EXEC,
	CAVAN_SSH_REQ_FILE_STAT,
	CAVAN_SSH_REQ_FILE_READ,
	CAVAN_SSH_REQ_FILE_WRITE,
} cavan_ssh_req_t;

#pragma pack(1)

struct cavan_ssh_package {
	u32 version;
	u32 flags;
	u8 type;
	u8 type_invert;
	u16 body_size;
	u8 body[0];
};

#pragma pack()
