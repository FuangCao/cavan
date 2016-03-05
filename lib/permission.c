/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <cavan/permission.h>

int check_super_permission(bool def_choose, int timeout_ms)
{
	if (user_is_super()) {
		return 0;
	}

	if (setuid(0) == 0 && setgid(0) == 0) {
		pr_green_info("Change to super user successfull");
		return 0;
	}

	pr_red_info("Require super user permission");

	if (cavan_get_choose_yesno("Do you want to run as general user", def_choose, timeout_ms)) {
		return 0;
	}

	ERROR_RETURN(EPERM);
}

int cavan_permission_set(u32 permission)
{
	int ret;
	struct __user_cap_data_struct data = {
		.permitted = permission,
		.effective = permission
	};
	struct __user_cap_header_struct header = {
		.pid = 0,
		.version = _LINUX_CAPABILITY_VERSION,
	};

	ret = capset(&header, &data);
	if (ret < 0) {
		pr_error_info("capset");
		return ret;
	}

	return 0;
}

int cavan_permission_clear(u32 permission)
{
	int ret;
	struct __user_cap_data_struct data;
	struct __user_cap_header_struct header = {
		.pid = 0,
		.version = _LINUX_CAPABILITY_VERSION,
	};

	ret = capget(&header, &data);
	if (ret < 0) {
		pr_error_info("capget");
		return ret;
	}

	println("permitted = 0x%08x, effective = 0x%08x", data.permitted, data.effective);

	data.permitted &= ~permission;
	data.effective = data.permitted;

	ret = capset(&header, &data);
	if (ret < 0) {
		pr_error_info("capset");
		return ret;
	}

	return 0;
}

void cavan_parse_user_text(char *text, const char **user, const char **group)
{
	char **pp;
	char *pu, *pg;

	pp = &pu;
	pu = text;
	pg = NULL;

	while (1) {
		switch (*text) {
		case 0:
			if (*pu) {
				*user = pu;
			} else {
				*user = NULL;
			}

			if (pg && *pg) {
				*group = pg;
			} else {
				*group = NULL;
			}
			return;

		case '.':
		case ',':
		case ':':
		case '@':
			*text = 0;
			pp = &pg;
			pg = text + 1;
			break;

		case ' ':
		case '\t':
			*text = 0;
			if (text == *pp) {
				*pp = text + 1;
			}
			break;
		}

		text++;
	}
}

uid_t cavan_user_name_to_uid(const char *name)
{
	struct passwd *pw;

	if (cavan_isdigit_text(name)) {
		return text2value_unsigned(name, NULL, 10);
	}

	pw = getpwnam(name);
	if (pw == NULL) {
		return CAVAN_UID_INVALID;
	}

	println("user: name = %s, uid = %d", pw->pw_name, pw->pw_uid);

	return pw->pw_uid;
}

gid_t cavan_group_name_to_gid(const char *name)
{
	struct group *gr;

	if (cavan_isdigit_text(name)) {
		return text2value_unsigned(name, NULL, 10);
	}

	gr = getgrnam(name);
	if (gr == NULL) {
		return CAVAN_GID_INVALID;
	}

	println("group: name = %s, gid = %d", gr->gr_name, gr->gr_gid);

	return gr->gr_gid;
}

char *cavan_user_uid_to_name(uid_t uid, char *buff, size_t size)
{
	struct passwd *pw;

	pw = getpwuid(uid);
	if (pw == NULL) {
		return NULL;
	}

	println("user: name = %s, uid = %d", pw->pw_name, pw->pw_uid);

	return text_ncopy(buff, pw->pw_name, size);
}

char *cavan_group_gid_to_name(gid_t gid, char *buff, size_t size)
{
	struct group *gr;

	gr = getgrgid(gid);
	if (gr == NULL) {
		return NULL;
	}

	println("group: name = %s, gid = %d", gr->gr_name, gr->gr_gid);

	return text_ncopy(buff, gr->gr_name, size);
}

struct passwd *cavan_user_get_passwd(const char *name)
{
	struct passwd *pw;

	if (name == NULL) {
		return getpwuid(getuid());
	}

	pw = getpwnam(name);
	if (pw) {
		return pw;
	}

	if (cavan_isdigit_text(name)) {
		return getpwuid(text2value_unsigned(name, NULL, 10));
	}

	return NULL;
}

struct group *cavan_group_get_group(const char *name)
{
	struct group *gr;

	if (name == NULL) {
		return getgrgid(getgid());
	}

	gr = getgrnam(name);
	if (gr) {
		return gr;
	}

	if (cavan_isdigit_text(name)) {
		return getgrgid(text2value_unsigned(name, NULL, 10));
	}

	return NULL;
}
