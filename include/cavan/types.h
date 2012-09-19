#pragma once

#include <asm-generic/int-ll64.h>

typedef __u64 u64, dword;
typedef __s64 s64;

typedef __u32 u32, word;
typedef __s32 s32, sword;

typedef __u16 u16, hword;
typedef __s16 s16;

typedef __u8 u8, byte;
typedef __s8 s8;

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef signed long int slong;
typedef signed short int sshort;
typedef signed int sint;
typedef signed char schar;

typedef char * pchar, text;

typedef enum
{
	false,
	true
} bool;

typedef enum
{
	False,
	True,
} Bool;

typedef enum
{
	FALSE,
	TRUE,
} BOOL;

typedef union
{
	void *type_void;

	char type_char;
	short type_short;
	int type_int;
	long type_long;

	unsigned char type_uchar;
	unsigned short type_ushort;
	unsigned int type_uint;
	unsigned long type_ulong;
} cavan_shared_data_t;
