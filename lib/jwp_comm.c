/*
 * File:		jwp_comm.c
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
#include <cavan/jwp_comm.h>

static jwp_size_t jwp_comm_hw_read(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	ssize_t rdlen;
	struct jwp_comm_desc *comm = (struct jwp_comm_desc *) jwp;

	rdlen = read(comm->fd, buff, size);
	if (rdlen < 0)
	{
		pr_error_info("read");
		return 0;
	}

	return rdlen;
}

static jwp_size_t jwp_comm_hw_write(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	ssize_t wrlen;
	struct jwp_comm_desc *comm = (struct jwp_comm_desc *) jwp;

	wrlen = write(comm->fd, buff, size);
	if (wrlen < 0)
	{
		pr_error_info("write");
		return 0;
	}

	return wrlen;
}

jwp_bool jwp_comm_init(struct jwp_comm_desc *comm, int fd, void *data)
{
	struct jwp_desc *jwp = &comm->jwp;

	comm->fd = fd;
	jwp->hw_read = jwp_comm_hw_read;
	jwp->hw_write = jwp_comm_hw_write;

	return jwp_linux_init(&comm->jwp_linux, data);
}

jwp_bool jwp_comm_init2(struct jwp_comm_desc *comm, const char *pathname, void *data)
{
	int fd;

	fd = open(pathname, O_RDWR);
	if (fd < 0)
	{
		pr_error_info("open %s", pathname);
		return fd;
	}

	return jwp_comm_init(comm, fd, data);
}

jwp_bool jwp_comm_start(struct jwp_comm_desc *comm)
{
	if (!jwp_linux_start(&comm->jwp_linux))
	{
		return false;
	}

	jwp_send_sync(&comm->jwp);

	return true;
}
