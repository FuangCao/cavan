/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed Mar 27 18:27:03 CST 2013
 */

#include <cavan.h>
#include <cavan/command.h>
#include <cavan/screen_mirror.h>

#define FILE_CREATE_DATE "2013-03-27 18:27:03"

static int screen_mirror_service_main(int argc, char *argv[])
{
	struct screen_mirror_service service;

	return screen_mirror_service_run(&service);
}

static int screen_mirror_client_main(int argc, char *argv[])
{
	int ret;
	char buff[1024];

	ret = scanf("%s", buff);
	println("ret = %d", ret);

	return screen_mirror_client(buff, text_len(buff));
}

static struct cavan_command_map map[] =
{
	{
		.name = "service",
		.main_func = screen_mirror_service_main
	},
	{
		.name = "client",
		.main_func = screen_mirror_client_main
	}
};

FIND_EXEC_COMMAND_MAIN(map, false);
