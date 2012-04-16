#pragma once

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS	64
#define __USE_FILE_OFFSET64

#include <linux/version.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <cavan/debug.h>
#include <cavan/types.h>
#include <cavan/stdio.h>
#include <cavan/text.h>
#include <cavan/memory.h>
#include <cavan/file.h>

#define msleep(msec)	usleep((msec) * 1000)
#define ssleep(sec)		msleep((sec) * 1000)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
#define NELEM(a)		(ARRAY_SIZE(a))
#define KB(a)			((a) << 10)
#define BK(a)			((a) >> 10)
#define MB(a)			((a) << 20)
#define BM(a)			((a) >> 20)
#define GB(a)			((a) << 30)
#define BG(a)			((a) >> 30)

#define POINTER_ASSIGN_CASE(dest, src) \
	((dest) = (typeof(dest))(src))

#define MEMBER_OFFSET(type, member) \
	((int)&((type *)0)->member)

#define MEMBER_STRUCT(addr, type, member) \
	((void *)(addr) - MEMBER_OFFSET(type, member))

#define MLC_TO(p) \
	(p) = malloc(sizeof(*p))

#define BYTES_WORD(h, l) \
	(((u16)(h)) << 8 | (l))

#define WORDS_DWORD(h, l) \
	(((u32)(h)) << 16 | (l))

#define BYTES_DWORD(b1, b2, b3, b4) \
	WORDS_DWORD(BYTES_WORD(b1, b2), BYTES_WORD(b3, b4))

#define ERROR_RETURN(en) \
	do { \
		errno = en; \
		return -en; \
	} while (0)

#define RETRY(func, ret, condition, count) \
	do { \
		int i = count; \
		do { \
			ret = func; \
			i--; \
		} while (i && (condition)); \
	} while (0)

