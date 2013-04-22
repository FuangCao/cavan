#pragma once

#include <cavan.h>

struct dd_desc
{
	char in[1024];
	char out[1024];
	off_t seek;
	off_t skip;
	off_t count;
	off_t bs;
};

int fcavan_dd(int fd_in, int fd_out, off_t offset_in, off_t offset_out, off_t length, off_t size);
int cavan_dd_base(const char *file_in, const char *file_out, off_t offset_in, off_t offset_out, off_t length, int flags);
int cavan_dds(struct dd_desc *descs, size_t count);

static inline int cavan_dd(const char *file_in, const char *file_out, off_t offset_in, off_t offset_out, off_t length)
{
	return cavan_dd_base(file_in, file_out, offset_in, offset_out, length, 0);
}

static inline int cavan_dd2(struct dd_desc *desc)
{
	return cavan_dd(desc->in, desc->out, desc->skip * desc->bs, desc->seek * desc->bs, desc->count * desc->bs);
}

