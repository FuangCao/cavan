/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed May 16 10:15:01 CST 2012
 */

#include <cavan.h>
#include <cavan/permission.h>

int is_super_user(const char *prompt)
{
	if (getuid() == 0)
	{
		return 0;
	}

	if (prompt)
	{
		pr_red_info("%s", prompt);
	}
	else
	{
		pr_red_info("Only super can run this program!");
	}

	return -EPERM;
}
