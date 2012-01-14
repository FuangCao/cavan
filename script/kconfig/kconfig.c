/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include "kconfig.h"

int main(int argc, char *argv[])
{
	int ret;
	struct kconfig_menu_descriptor desc =
	{
		.title = "Cavan Configure",
		.prompt =	"Arrow keys navigate the menu. <Enter> selects submenus --->. "
					"Highlighted letters are hotkeys.  Pressing <Y> includes, <N> excludes, <M> modularizes features.  "
					"Press <Esc><Esc> to exit, <?> for Help, </> for Search. "
					"Legend: [*] built-in  [ ] excluded  <M> module  < > module capable",
	};
	int i;
	struct kconfig_menu_item arch_item =
	{
		.state = KCONFIG_STATE_SELECTED,
		.text = "Architecture",
	};
	struct kconfig_menu_item arch_items[] =
	{
		{
			.state = KCONFIG_STATE_SELECTED,
			.text = "host",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "arm",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "x86",
		},
	};
	struct kconfig_menu_item toolchian_item =
	{
		.state = KCONFIG_STATE_SELECTED,
		.text = "Toolchian",
	};
	struct kconfig_menu_item toolchian_items[] =
	{
		{
			.state = KCONFIG_STATE_SELECTED,
			.text = "none",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "arm-linux",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "arm-cavan-linux-gnueabi",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "arm-none-linux-gnueabi",
		},
	};
	struct kconfig_menu_item type_item =
	{
		.state = KCONFIG_STATE_SELECTED,
		.text = "Build type",
	};
	struct kconfig_menu_item type_items[] =
	{
		{
			.state = KCONFIG_STATE_SELECTED,
			.text = "debug",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "release",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "static",
		},
	};
	struct kconfig_menu_item entry_item =
	{
		.state = KCONFIG_STATE_SELECTED,
		.text = "Build entry",
	};
	struct kconfig_menu_item entry_items[] =
	{
		{
			.state = KCONFIG_STATE_SELECTED,
			.text = "application",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "library",
		},
		{
			.state = KCONFIG_STATE_DESELED,
			.text = "cavan",
		},
	};

	kconfig_menu_init(&desc);
	kconfig_menu_add_item(&desc, &arch_item);
	kconfig_menu_add_item(&desc, &toolchian_item);
	kconfig_menu_add_item(&desc, &type_item);
	kconfig_menu_add_item(&desc, &entry_item);

	for (i = 0; i < NELEM(arch_items); i++)
	{
		kconfig_menu_add_child(&arch_item, arch_items + i);
	}

	for (i = 0; i < NELEM(toolchian_items); i++)
	{
		kconfig_menu_add_child(&toolchian_item, toolchian_items + i);
	}

	for (i = 0; i < NELEM(type_items); i++)
	{
		kconfig_menu_add_child(&type_item, type_items + i);
	}

	for (i = 0; i < NELEM(entry_items); i++)
	{
		kconfig_menu_add_child(&entry_item, entry_items + i);
	}

#if 1
	ret = ncurses_show_menu_box(&desc);
#else
	ret = ncurses_show_yes_no_dialog(60, 5, "88888888888888");
#endif

	endwin();

	return ret;
}
