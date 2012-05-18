/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <cavan/permission.h>

int has_super_permission(const char *prompt)
{
	if (user_is_super())
	{
		return 0;
	}

	if (prompt)
	{
		pr_red_info("%s", prompt);
	}
	else
	{
		pr_red_info("Only super can do this");
	}

	ERROR_RETURN(EPERM);
}
