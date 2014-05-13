#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <linux/capability.h>

extern int capget(cap_user_header_t hdrp, cap_user_data_t datap);
extern int capset(cap_user_header_t hdrp, const cap_user_data_t datap);

int check_super_permission(bool def_choose, int timeout_ms);
int cavan_permission_set(u32 permission);
int cavan_permission_clear(u32 permission);

static inline bool user_id_equal(uid_t uid)
{
	return uid == getuid();
}

static inline bool user_is_super(void)
{
	return user_id_equal(0);
}
