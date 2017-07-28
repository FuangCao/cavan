#pragma once

#include <cavan/config.h>

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
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/syscall.h>
#include <poll.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <features.h>

extern int pipe2(int *, int);

#define ERROR_RETURN(en) \
	do { \
		errno = en; \
		return -errno; \
	} while (0)

#define ERRNO_NEED_RETRY() \
	(errno == EINTR || errno == EAGAIN)

#define ERRNO_NOT_RETRY() \
	(errno != EINTR && errno != EAGAIN)

#ifdef __GNUC__
#define GCC_VERSION				(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define	__GNUC_PREREQ__(x, y)	((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) || (__GNUC__ > (x)))
#else
#define GCC_VERSION				0
#define	__GNUC_PREREQ__(x, y)	0
#endif

#ifndef __maybe_unused
#ifdef __unused
#define __maybe_unused			__unused
#else
#if __GNUC_PREREQ__(2, 7)
#define	__maybe_unused				__attribute__((__unused__))
#else
#define	__maybe_unused	/* delete */
#endif
#endif
#endif

#ifndef __used
#if __GNUC_PREREQ__(3, 1)
#define	__used					__attribute__((__used__))
#else
#define	__used		/* delete */
#endif
#endif

#if __GNUC_PREREQ__(2, 7)
#define	__packed				__attribute__((__packed__))
#define	__aligned(x)			__attribute__((__aligned__(x)))
#define	__section(x)			__attribute__((__section__(x)))
#elif defined(__lint__)
#define	__packed	/* delete */
#define	__aligned(x)	/* delete */
#define	__section(x)	/* delete */
#else
#define	__packed				error: no __packed for this compiler
#define	__aligned(x)			error: no __aligned for this compiler
#define	__section(x)			error: no __section for this compiler
#endif

#if __GNUC_PREREQ__(2, 96)
#define	__predict_true(exp)		__builtin_expect((exp) != 0, 1)
#define	__predict_false(exp)	__builtin_expect((exp) != 0, 0)
#else
#define	__predict_true(exp)		(exp)
#define	__predict_false(exp)	(exp)
#endif

#if __GNUC_PREREQ__(2, 96)
#define __noreturn    			__attribute__((__noreturn__))
#define __mallocfunc  			__attribute__((malloc))
#else
#define __noreturn
#define __mallocfunc
#endif

#if __GNUC_PREREQ__(3, 4)
#define __must_check			__attribute__((warn_unused_result))
#else
#define __must_check
#endif

#if __GNUC_PREREQ__(2, 95)
#define	__insn_barrier()		__asm __volatile("":::"memory")
#else
#define	__insn_barrier()
#endif

#define likely(exp)				__predict_true(exp)
#define unlikely(exp)			__predict_false(exp)

#ifdef __cplusplus
#define	__BEGIN_DECLS			extern "C" {
#define	__END_DECLS				}
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif

#ifndef __static_cast
#ifdef __cplusplus
#define	__static_cast(x, y)		static_cast<x>(y)
#else
#define	__static_cast(x, y)		((x) (y))
#endif
#endif

#ifndef __UNCONST
#define __UNCONST(a)			((void *) (unsigned long) (const void *) (a))
#endif

#ifndef __read_mostly
#define __read_mostly			__section(".data..read_mostly")
#endif

__BEGIN_DECLS

extern int main(int argc, char *argv[]);

static inline void msleep(useconds_t msec)
{
	while (msec--) {
		usleep(1000);
	}
}

static inline void ssleep(useconds_t sec)
{
	while (sec--) {
		msleep(1000);
	}
}

__END_DECLS

#include <cavan/debug.h>
#include <cavan/types.h>
#include <cavan/stdio.h>
#include <cavan/text.h>
#include <cavan/path.h>
#include <cavan/memory.h>
#include <cavan/file.h>
#include <cavan/ctype.h>
