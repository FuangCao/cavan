/*
 * File:		message_queue.c
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
#include <cavan/timer.h>
#include <cavan/message_queue.h>

int cavan_mq_writer_open(struct cavan_mq_writer *writer, struct cavan_mq_context *context)
{
	return 0;
}

int cavan_mq_writer_write(struct cavan_mq_writer *writer, u32 uid, u32 suid, const void *message, int length)
{
	struct cavan_mq_index index;
	int wrlen;

	index.time = clock_gettime_real_ss();
	index.uid = uid;
	index.suid = suid;
	index.length = length;
	index.offset = lseek(writer->data_fd, 0, SEEK_CUR);

	wrlen = write(writer->data_fd, message, length);
	if (wrlen < 0) {
		return wrlen;
	}

	wrlen = write(writer->index_fd, &index, sizeof(index));
	if (wrlen < 0) {
		return wrlen;
	}

	return 0;
}

int cavan_mq_reader_open(struct cavan_mq_reader *reader, struct cavan_mq_context *context)
{
	return 0;
}

int cavan_mq_reader_read(struct cavan_mq_reader *reader, struct cavan_mq_index *index, char *buff, int size)
{
	int ret;

	ret = cavan_file_fill(reader->index_fd, index, sizeof(*index));
	if (ret < 0) {
		return ret;
	}

	if (lseek(reader->data_fd, index->offset, SEEK_SET) < 0) {
		return -EFAULT;
	}

	if (size > (int) index->length) {
		size = index->length;
	}

	ret = cavan_file_fill(reader->data_fd, buff, size);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
