#pragma once

#include <cavan.h>
#include <stdint.h>
#include <sys/types.h>

typedef uint64_t u64, dword;
typedef int64_t s64;

typedef uint32_t u4, u32, word;
typedef int32_t s4, s32, sword;

typedef uint16_t u2, u16, hword;
typedef int16_t s2, s16;

typedef uint8_t u1, u8, byte;
typedef int8_t s1, s8;

typedef unsigned long int ulong;
typedef unsigned short int ushort;

typedef unsigned char uchar;

typedef signed long int slong;
typedef signed short int sshort;
typedef signed int sint;
typedef signed char schar;

typedef char * pchar, text;

#ifndef __cplusplus
typedef enum
{
	false,
	true
} bool;
#endif

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

typedef struct
{
	int x;
	int y;
} cavan_point_t;

#define ARRAY_SIZE(a)				(sizeof(a) / sizeof((a)[0]))
#define NELEM(a)					((int)ARRAY_SIZE(a))
#define MOFS(type, member)			MEMBER_OFFSET(type, member)

#define BYTE_IS_LF(b) \
	((b) == '\r' || (b) == '\n')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t')

#define POINTER_ASSIGN_CASE(dest, src) \
	((dest) = (typeof(dest))(src))

#define MEMBER_OFFSET(type, member) \
	((long)&((type *)0)->member)

#define OFFSETOF(type, member) \
	MEMBER_OFFSET(type, member)

#define MEMBER_TO_STRUCT(addr, type, member) \
	((type *)((byte *)(addr) - MEMBER_OFFSET(type, member)))

#define POINTER_ADD(pointer, offset) \
	((void *)(((byte *)(pointer)) + (offset)))

#define POINTER_SUB(pointer, offset) \
	((void *)(((byte *)(pointer)) - (offset)))

#define ADDR_ADD(addr, offset) \
	POINTER_ADD(addr, offset)

#define ADDR_SUB(addr, offset) \
	POINTER_SUB(addr, offset)

#define ADDR_SUB2(addr1, addr2) \
	((byte *)(addr1) - (byte *)(addr2))

#define MLC_TO(p) \
	(p) = malloc(sizeof(*p))

#define BYTES_WORD(h, l) \
	(((u16)(h)) << 8 | (l))

#define WORDS_DWORD(h, l) \
	(((u32)(h)) << 16 | (l))

#define BYTES_DWORD(b1, b2, b3, b4) \
	WORDS_DWORD(BYTES_WORD(b1, b2), BYTES_WORD(b3, b4))

#define SWAP16(value)(value) \
	((value) >> 8 | (value) << 8)

#define SWAP32(value) \
	((value) >> 24 | (value) << 24 | ((value) & 0x00FF0000) >> 8 | ((value) & 0x0000FF00) << 8)

#define VALUE_DIFF(a, b) \
	((a) > (b) ? (a) - (b) : (b) - (a))
