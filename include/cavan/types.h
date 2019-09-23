#pragma once

#include <cavan.h>
#include <stdint.h>
#include <sys/types.h>

#define SIZEOF_INT		__SIZEOF_INT__
#define SIZEOF_LONG		__SIZEOF_LONG__
#define SIZEOF_SIZE_T	__SIZEOF_SIZE_T__

#define std_sp			std::shared_ptr
#define std_make_sp		std::make_shared

#ifndef null
#define null			NULL
#endif

#ifndef nil
#define nil				NULL
#endif

#ifndef PATH_MAX
#define PATH_MAX		1024
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN	PATH_MAX
#endif

#ifndef PATH_SIZE
#define PATH_SIZE		PATH_MAX
#endif

#ifndef BIT
#define BIT(offset) \
	(1UL << (offset))
#endif

#define BIT8(offset) \
	(((u8) 1) << (offset))

#define BIT16(offset) \
	(((u16) 1) << (offset))

#define BIT32(offset) \
	(((u32) 1) << (offset))

#define BIT64(offset) \
	(((u64) 1) << (offset))

#define TEXT_PAIR(text) \
	text, (sizeof(text) - 1)

#define ARRAY_PAIR(array) \
	array, NELEM(array)

#define CAST(type, value)		((type) (value))
#define CAST_PV(type, value)	(*CAST(type *, value))

#define CAST_CHAR(value)		CAST(int, (value))
#define CAST_UCHAR(value)		CAST(uchar, (value))

#define CAST_SHORT(value)		CAST(short, (value))
#define CAST_USHORT(value)		CAST(ushort, (value))

#define CAST_INT(value)			CAST(int, (value))
#define CAST_UINT(value)		CAST(uint, (value))

#define CAST_LONG(value)		CAST(long, (value))
#define CAST_ULONG(value)		CAST(ulong, (value))

#define CAST_FLOAT(value)		CAST(float, (value))
#define CAST_DOUBLE(value)		CAST(double, (value))

#define CAST_S8(value)			CAST(s8, (value))
#define CAST_U8(value)			CAST(u8, (value))

#define CAST_S16(value)			CAST(s16, (value))
#define CAST_U16(value)			CAST(u16, (value))

#define CAST_S32(value)			CAST(s32, (value))
#define CAST_U32(value)			CAST(u32, (value))

#define CAST_S64(value)			CAST(s64, (value))
#define CAST_U64(value)			CAST(u64, (value))

typedef uint64_t u64, dword;
typedef int64_t s64;

typedef uint32_t u4, u32, word;
typedef int32_t s4, s32, sword;

typedef uint16_t u2, u16, hword;
typedef int16_t s2, s16;

typedef uint8_t u1, u8, byte;
typedef int8_t s1, s8;

typedef int sint;
typedef char schar;
typedef short sshort;
typedef long slong;
typedef long long slonglong, sllong;

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned long long ulonglong, ullong;

typedef char * charptr, text;
typedef short * shortptr;
typedef int * intptr;
typedef long * longptr;
typedef void * voidptr;

typedef u32 size32_t;
typedef s32 ssize32_t;
typedef u64 size64_t;
typedef s64 ssize64_t;

#ifndef __cplusplus
#ifndef bool
typedef enum {
	false,
	true
} bool;
#endif
#endif

typedef enum {
	False,
	True,
} Bool;

typedef enum {
	FALSE,
	TRUE,
} BOOL;

typedef bool boolean;
typedef Bool Boolean;
typedef BOOL BOOLEAN;

typedef enum {
	WALKER_ACTION_CONTINUE,
	WALKER_ACTION_COMPLETE,
	WALKER_ACTION_EOF,
	WALKER_ACTION_STOP
} walker_action_t;

typedef union {
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

typedef struct {
	int x;
	int y;
} cavan_point_t;

#define ARRAY_SIZE(a)				(sizeof(a) / sizeof(*(a)))
#define NELEM(a)					((int) ARRAY_SIZE(a))
#define LAST_ELEM(a)				((a) + NELEM(a) - 1)
#define MOFS(type, member)			MEMBER_OFFSET(type, member)

#define BUILD_MASK(type, count) \
	((((type) 1) << (count)) - 1)

#define BYTE_IS_LF(b) \
	((b) == '\r' || (b) == '\n')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t')

#define POINTER_ASSIGN_CASE(dest, src) \
	((dest) = (typeof(dest)) (src))

#define MEMBER_OFFSET(type, member) \
	(((byte *) &((type *) 0)->member) - ((byte *) 0))

#define OFFSETOF(type, member) \
	MEMBER_OFFSET(type, member)

#ifndef offsetof
#define offsetof(type, member) \
	MEMBER_OFFSET(type, member)
#endif

#define MEMBER_TO_STRUCT(addr, type, member) \
	((type *) ((byte *) (addr) - MEMBER_OFFSET(type, member)))

#define POINTER_ADD(pointer, offset) \
	((void *) (((byte *) (pointer)) + (offset)))

#define POINTER_SUB(pointer, offset) \
	((void *) (((byte *) (pointer)) - (offset)))

#define ADDR_ADD(addr, offset) \
	POINTER_ADD(addr, offset)

#define ADDR_SUB(addr, offset) \
	POINTER_SUB(addr, offset)

#define ADDR_SUB2(addr1, addr2) \
	((byte *) (addr1) - (byte *) (addr2))

#define ADDR_OFFSET(addr1, addr2) \
	(((byte *) (addr1) > (byte *) (addr2)) ? ADDR_SUB2(addr1, addr2) : ADDR_SUB2(addr2, addr1))

#define ADDR_IS_ALIGN(addr, bits) \
	((((long) (addr)) & ((bits) - 1)) == 0)

#define ADDR_IS_LEFT_ALIGN(addr, bits) \
	ADDR_IS_ALIGN(ADDR_SUB(addr, (bits) - 1), bits)

#define MLC_TO(p) \
	(p) = malloc(sizeof(*p))

#define BYTES_WORD(h, l) \
	(((u16) (h)) << 8 | (l))

#define WORDS_DWORD(h, l) \
	(((u32) (h)) << 16 | (l))

#define BYTES_DWORD(b1, b2, b3, b4) \
	WORDS_DWORD(BYTES_WORD(b1, b2), BYTES_WORD(b3, b4))

#define BYTE_ARRAY_DWORD(a) \
	BYTES_DWORD((a)[3], (a)[2], (a)[1], (a)[0])

#define BUILD_U16(h, l) \
	(((u16) (h)) << 8 | (l))

#define BUILD_U32(h, l) \
	(((u32) (h)) << 16 | (l))

#define BUILD_U64(h, l) \
	(((u64) (h)) << 32 | (l))

#define SWAP16(value) \
	((value) >> 8 | (value) << 8)

#define SWAP32(value) \
	((value) >> 24 | (value) << 24 | ((value) & 0x00FF0000) >> 8 | ((value) & 0x0000FF00) << 8)

#define VALUE_DIFF(a, b) \
	((a) > (b) ? (a) - (b) : (b) - (a))

#define mb()	__sync_synchronize()
#define rmb()	mb()
#define wmb() 	mb()

#define cavan_atomic_get(addr) \
	({ __sync_synchronize(); *(addr); })

#define cavan_atomic_set(addr, value) \
	({ *(addr) = (value); __sync_synchronize(); })

#define cavan_atomic_cmpxchg(addr, oldval, newval) \
	__sync_val_compare_and_swap((addr), (oldval), (newval))

#define cavan_atomic_cmpxchg_bool(addr, oldval, newval) \
	__sync_bool_compare_and_swap((addr), (oldval), (newval))

#define cavan_atomic_add(addr, value) \
	__sync_add_and_fetch((addr), (value))

#define cavan_atomic_sub(addr, value) \
	__sync_sub_and_fetch((addr), (value))

#define cavan_atomic_and(addr, value) \
	__sync_and_and_fetch((addr), (value))

#define cavan_atomic_or(addr, value) \
	__sync_or_and_fetch((addr), (value))

#define cavan_atomic_xor(addr, value) \
	__sync_xor_and_fetch((addr), (value))

#define cavan_atomic_inc(addr) \
	cavan_atomic_add((addr), 1)

#define cavan_atomic_dec(addr) \
	cavan_atomic_sub((addr), 1)

#define cavan_atomic_lock_fetch_set(addr, value) \
	__sync_lock_test_and_set((addr), (value))

#define cavan_atomic_lock_release(addr) \
	__sync_lock_release(addr)

// ================================================================================

#define cavan_atomic_fetch_set(addr, value) \
	__sync_lock_test_and_set((addr), (value))

#define cavan_atomic_fetch_add(addr, value) \
	__sync_fetch_and_add((addr), (value))

#define cavan_atomic_fetch_sub(addr, value) \
	__sync_fetch_and_sub((addr), (value))

#define cavan_atomic_fetch_and(addr, value) \
	__sync_fetch_and_and((addr), (value))

#define cavan_atomic_fetch_or(addr, value) \
	__sync_fetch_and_or((addr), (value))

#define cavan_atomic_fetch_xor(addr, value) \
	__sync_fetch_and_xor((addr), (value))

#define cavan_atomic_fetch_inc(addr) \
	cavan_atomic_fetch_add((addr), 1)

#define cavan_atomic_fetch_dec(addr) \
	cavan_atomic_fetch_sub((addr), 1)

typedef volatile uint32_t cavan_spinlock_t;

#define cavan_spinlock_lock(lock) \
	while(cavan_atomic_lock_fetch_set(lock, 1)) { \
		sched_yield(); \
	}

#define cavan_spinlock_unlock(lock) \
	cavan_atomic_set(lock, 0)

#define cavan_malloc_type(type) \
	(type *) malloc(sizeof(type))

static inline u64 SWAP64(u64 value)
{
	u32 H = value >> 32;
	u32 L = value & BUILD_MASK(u64, 32);

	return ((u64) SWAP32(L)) << 32 | SWAP32(H);
}
