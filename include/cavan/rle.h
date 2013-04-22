#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 18 15:04:58 CST 2011

#include <cavan.h>

char *frle_data_to(const u8 *data, ssize_t size, int fd_out);
int frle_to(int fd_in, int fd_out);
int rle_to(const char *file_in, const char *file_out);

char *funrle_data_to(const u8 *data, ssize_t size, int fd_out);
int funrle_to(int fd_in, int fd_out);
int unrle_to(const char *file_in, const char *file_out);
