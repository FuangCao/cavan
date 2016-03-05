#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <linux/capability.h>

#define CAVAN_UID_INVALID		((uid_t) -1)
#define CAVAN_GID_INVALID		((gid_t) -1)

extern int capget(cap_user_header_t hdrp, cap_user_data_t datap);
extern int capset(cap_user_header_t hdrp, const cap_user_data_t datap);

int check_super_permission(bool def_choose, int timeout_ms);
int cavan_permission_set(u32 permission);
int cavan_permission_clear(u32 permission);
void cavan_parse_user_text(char *text, const char **user, const char **group);
uid_t cavan_user_name_to_uid(const char *name);
gid_t cavan_group_name_to_gid(const char *name);
char *cavan_user_uid_to_name(uid_t uid, char *buff, size_t size);
char *cavan_group_gid_to_name(gid_t gid, char *buff, size_t size);

static inline bool user_id_equal(uid_t uid)
{
	return uid == getuid();
}

static inline bool user_is_super(void)
{
	return user_id_equal(0);
}
