// Fuang.Cao <cavan.cfa@gmail.com> Wed Aug 24 17:42:02 CST 2011

#include <cavan.h>
#include <cavan/swan_vk.h>
#include <cavan/command.h>

CAVAN_COMMAND_MAP_START {
	{
		.name = "server",
		.main_func = swan_vk_server_main
	}, {
		.name = "client",
		.main_func = swan_vk_client_main
	}, {
		.name = "cmdline",
		.main_func = swan_vk_cmdline_main
	}, {
		.name = "cmd",
		.main_func = swan_vk_cmdline_main
	}, {
		.name = "command",
		.main_func = swan_vk_cmdline_main
	}, {
		.name = "line",
		.main_func = swan_vk_line_main
	}, {
		.name = "unlock",
		.main_func = swan_vk_unlock_main
	}
} CAVAN_COMMAND_MAP_END;
