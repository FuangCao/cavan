#pragma once

/*
 * File:		message_queue.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2019-02-18 14:39:30
 *
 * Copyright (c) 2019 Fuang.Cao <cavan.cfa@gmail.com>
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

struct cavan_mq_index {
	u32 offset;
	u32 length;
	u32 time;
	u32 suid;
	u32 uid;
};

struct cavan_mq_account {
	u32 uid;
	char name[1024];
};

struct cavan_mq_writer {
	int index_fd;
	int data_fd;
	u32 offset;
};

struct cavan_mq_reader {
	int index_fd;
	int data_fd;
};

struct cavan_mq_context {
	const char *save_dir;
};

int cavan_mq_writer_open(struct cavan_mq_writer *writer, struct cavan_mq_context *context);
int cavan_mq_writer_write(struct cavan_mq_writer *writer, u32 uid, u32 suid, const void *message, int length);
int cavan_mq_reader_open(struct cavan_mq_reader *reader, struct cavan_mq_context *context);
int cavan_mq_reader_read(struct cavan_mq_reader *reader, struct cavan_mq_index *index, char *buff, int size);
