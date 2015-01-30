#pragma once

/*
 * File:		jwp_comm.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-30 10:51:22
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
#include <cavan/jwp-linux.h>

struct jwp_comm_desc
{
	struct jwp_desc jwp;

	int fd;
};

int jwp_comm_init(struct jwp_comm_desc *jwp_comm, int fd);
