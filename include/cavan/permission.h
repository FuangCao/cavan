#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>

int check_super_permission(bool def_choose, int timeout_ms);

static inline bool user_id_equal(uid_t uid)
{
	return uid == getuid();
}

static inline bool user_is_super(void)
{
	return user_id_equal(0);
}
