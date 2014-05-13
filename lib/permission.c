/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <cavan/permission.h>

int check_super_permission(bool def_choose, int timeout_ms)
{
	if (user_is_super())
	{
		return 0;
	}

	if (setuid(0) == 0 && setgid(0) == 0)
	{
		pr_green_info("Change to super user successfull");
		return 0;
	}

	pr_red_info("Require super user permission");

	if (cavan_get_choose_yesno("Do you want to run as general user", def_choose, timeout_ms))
	{
		return 0;
	}

	ERROR_RETURN(EPERM);
}

int cavan_permission_set(u32 permission)
{
	int ret;
	struct __user_cap_data_struct data =
	{
		.permitted = permission,
		.effective = permission
	};
	struct __user_cap_header_struct header =
	{
		.pid = 0,
		.version = _LINUX_CAPABILITY_VERSION,
	};

	ret = capset(&header, &data);
	if (ret < 0)
	{
		pr_error_info("capset");
		return ret;
	}

	return 0;
}

int cavan_permission_clear(u32 permission)
{
	int ret;
	struct __user_cap_data_struct data;
	struct __user_cap_header_struct header =
	{
		.pid = 0,
		.version = _LINUX_CAPABILITY_VERSION,
	};

	ret = capget(&header, &data);
	if (ret < 0)
	{
		pr_error_info("capget");
		return ret;
	}

	println("permitted = 0x%08x, effective = 0x%08x", data.permitted, data.effective);

	data.permitted &= ~permission;
	data.effective = data.permitted;

	ret = capset(&header, &data);
	if (ret < 0)
	{
		pr_error_info("capset");
		return ret;
	}

	return 0;
}
