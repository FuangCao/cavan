#pragma once

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#define _FILE_OFFSET_BITS	64
#define __USE_FILE_OFFSET64

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE		700
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE			1
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE		1
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE			500
#endif

#ifdef ANDROID
#ifndef CONFIG_BUILD_FOR_ANDROID
#define CONFIG_BUILD_FOR_ANDROID	1
#endif

#ifndef CAVAN_ARCH_ARM
#define CAVAN_ARCH_ARM				1
#endif
#endif
