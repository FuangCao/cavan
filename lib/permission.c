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
