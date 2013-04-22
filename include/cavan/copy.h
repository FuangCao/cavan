#pragma once

#include <cavan.h>

extern int write_flags;

int symlink_copy(const char *src, const char *dest);
int file_copy_main(const char *src, const char *dest);
int directory_copy_only(const char *src, const char *dest);
int directory_copy_main(const char *src, const char *dest);
int copy_main(const char *src, const char *dest);

int move_auto(const char *srcpath, const char *destpath);

