#pragma once

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
#include <poll.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static inline void msleep(useconds_t msec)
{
	while (msec--)
	{
		usleep(1000);
	}
}

static inline void ssleep(useconds_t sec)
{
	while (sec--)
	{
		msleep(1000);
	}
}

#include <cavan/debug.h>
#include <cavan/types.h>
#include <cavan/stdio.h>
#include <cavan/text.h>
#include <cavan/memory.h>
#include <cavan/file.h>

#define likely(exp)		(__builtin_expect(!!(exp), true))
#define unlikely(exp)	(__builtin_expect(!!(exp), false))
