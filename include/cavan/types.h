#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef uint64_t u64, dword;
typedef int64_t s64;

typedef uint32_t u32, word;
typedef int32_t s32, sword;

typedef uint16_t u16, hword;
typedef int16_t s16;

typedef uint8_t u8, byte;
typedef int8_t s8;

typedef unsigned long int ulong;
typedef unsigned short int ushort;

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
