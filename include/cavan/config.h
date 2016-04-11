#pragma once

#ifndef __WORDSIZE
#define __WORDSIZE			32
#endif

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

#ifndef CONFIG_ANDROID_NDK
#ifndef _GNU_SOURCE
#define _GNU_SOURCE			500
#endif
#endif

#if defined(ANDROID) && !defined(CONFIG_ANDROID)
#define CONFIG_ANDROID	1
#endif

#if defined(CONFIG_ANDROID) && !defined(CAVAN_ARCH_ARM)
#define CAVAN_ARCH_ARM				1
#endif

#ifndef CONFIG_ANDROID_VERSION
#define CONFIG_ANDROID_VERSION		1
#endif

#define CONFIG_I2C_ROCKCHIP_COMPAT	1
