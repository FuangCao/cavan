#define CAVAN_CMD_NAME swan_vk

// Fuang.Cao <cavan.cfa@gmail.com> Wed Aug 24 17:42:02 CST 2011

#include <cavan.h>
#include <cavan/swan_vk.h>
#include <cavan/command.h>

CAVAN_COMMAND_MAP_START {
	{ "server", swan_vk_server_main },
	{ "client", swan_vk_client_main },
	{ "cmdline", swan_vk_cmdline_main },
	{ "cmd", swan_vk_cmdline_main },
	{ "command", swan_vk_cmdline_main },
	{ "line", swan_vk_line_main },
	{ "unlock", swan_vk_unlock_main }
} CAVAN_COMMAND_MAP_END;
