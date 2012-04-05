/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:35:30 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_ts.h>
#include <cavan/command.h>

static struct cavan_command_map cmd_map[] =
{
	{
		.name = "calibration",
		.main_func = swan_ts_do_calication,
	},
};

int main(int argc, char *argv[])
{
	return find_and_exec_command(cmd_map, ARRAY_SIZE(cmd_map), argc, argv);
}
