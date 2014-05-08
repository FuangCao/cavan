/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu May 17 14:54:59 CST 2012
 */

#include <cavan.h>
#include <cavan/process.h>

ssize_t process_get_cmdline_simple1(pid_t pid, char *buff, size_t size)
{
	char filename[64];

	sprintf(filename, "/proc/%d/cmdline", pid);

	return file_read(filename, buff, size);
}

ssize_t process_get_cmdline_simple2(const char *pid, char *buff, size_t size)
{
	char filename[64];

	sprintf(filename, "/proc/%s/cmdline", pid);

	return file_read(filename, buff, size);
}

char *process_get_cmdline_base(const char *filename, char *buff, size_t size)
{
	ssize_t readlen;
	char readbuff[1024];
	char *p, *p_end, *buff_end;

	readlen = file_read(filename, readbuff, sizeof(readbuff));
	if (readlen < 0)
	{
		pr_red_info("read file %s failed", filename);
		return NULL;
	}

	for (p_end = readbuff + readlen - 1; *p_end == 0 && p_end > readbuff; p_end--);

	for (buff_end = buff + size - 1, p = readbuff; p <= p_end && buff < buff_end; buff++, p++)
	{
		if (*p == 0)
		{
			*buff = ' ';
		}
		else
		{
			*buff = *p;
		}
	}

	*buff = 0;

	return buff;
}

char *process_get_cmdline1(pid_t pid, char *buff, size_t size)
{
	char filename[64];

	sprintf(filename, "/proc/%d/cmdline", pid);

	return process_get_cmdline_base(filename, buff, size);
}

char *process_get_cmdline2(const char *pid, char *buff, size_t size)
{
	char filename[64];

	sprintf(filename, "/proc/%s/cmdline", pid);

	return process_get_cmdline_base(filename, buff, size);
}

pid_t process_find_by_cmdline(const char *proc_path, const char *cmdline)
{
	DIR *dir_proc;
	pid_t pid;
	struct dirent *en;
	char pathname[64], *p_name;

	if (proc_path == NULL)
	{
		proc_path = "/proc";
	}

	dir_proc = opendir(proc_path);
	if (dir_proc == NULL)
	{
		print_error("Open directroy %s failed", proc_path);
		return -EFAULT;
	}

	pid = -1;
	p_name = text_path_cat(pathname, sizeof(pathname), proc_path, NULL);

	while ((en = readdir(dir_proc)))
	{
		char buff[1024];
		ssize_t readlen;

		if (text_is_number(en->d_name) == 0)
		{
			continue;
		}

		sprintf(p_name, "%s/cmdline", en->d_name);

		readlen = file_read(pathname, buff, sizeof(buff));
		if (readlen < 0)
		{
			continue;
		}

		buff[readlen] = 0;

		if (text_lhcmp(cmdline, buff) == 0)
		{
			pid = text2value_unsigned(en->d_name, NULL, 10);
			break;
		}
	}

	closedir(dir_proc);

	return pid;
}

