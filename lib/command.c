/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:53:52 CST 2012
 */

#include <cavan.h>
#include <cavan/command.h>

void print_command_table(const struct cavan_command_map *p, size_t size)
{
	const struct cavan_command_map *p_end;
	char buff[MB(1)], *buff_p;

	if (size == 0)
	{
		return;
	}

	pr_bold_info("Available command is:");

	for (p_end = p + size, buff_p = buff; p < p_end; p++)
	{
		buff_p += sprintf(buff_p, "%s, ", p->name);
	}

	*(buff_p - 2) = 0;

	print_string(buff);
}

const struct cavan_command_map *find_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname, size_t size)
{
	while (p < p_end && text_ncmp(cmdname, p->name, size))
	{
		p++;
	}

	return p < p_end ? p : NULL;
}

void print_maybe_command(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	int size;
	const struct cavan_command_map *p_match;
	char buff[MB(1)], *buff_p;

	for (size = text_len(cmdname); size && ((p_match = find_command_by_name(p, p_end, cmdname, size))) == NULL; size--);

	if (size == 0)
	{
		pr_red_info("`%s' No such command", cmdname);
		return;
	}

	pr_bold_info("This command maybe:");

	buff_p = buff;

	do {
		buff_p += sprintf(buff_p, "%s, ", p_match->name);
	} while ((p_match = find_command_by_name(p_match + 1, p_end, cmdname, size)));

	*(buff_p - 2) = 0;

	print_string(buff);
}

const struct cavan_command_map *match_command_by_name(const struct cavan_command_map *p, const struct cavan_command_map *p_end, const char *cmdname)
{
	while (p < p_end && text_cmp(cmdname, p->name))
	{
		p++;
	}

	return p < p_end ? p : NULL;
}

int find_and_exec_command(const struct cavan_command_map *map, size_t count, int argc, char *argv[])
{
	const struct cavan_command_map *p;
	const char *pcmd;

	if (file_test(argv[0], "l") < 0)
	{
		if (argc < 2)
		{
			print_command_table(map, count);
			return -1;
		}

		pcmd = argv[1];
		argc--;
		argv++;
	}
	else
	{
		const char *pstart;

		for (pcmd = argv[0]; *pcmd; pcmd++);
		for (pstart = argv[0]; pcmd >= pstart && *pcmd != '/'; pcmd--);
		pcmd++;
	}

	p = match_command_by_name(map, map + count, pcmd);
	if (p)
	{
		return p->main_func(argc, argv);
	}

	print_maybe_command(map, map + count, pcmd);

	return -1;
}

