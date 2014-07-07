// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 18 15:04:58 CST 2011

#include <cavan.h>
#include <cavan/rle.h>
#include <cavan/progress.h>
#include <cavan/file.h>

#define MAX_BUFFER_LEN	MB(1)

char *frle_data_to(const u8 *data, ssize_t size, int fd_out)
{
	int ret;
	const u8 *data_end = data + size;
	u8 tmp;
	u8 buff[MAX_BUFFER_LEN];
	const u8 *p;
	u8 *q, *end_q;
	struct progress_bar prg_bar;

	q = buff;
	end_q = buff + sizeof(buff) - 1;

	progress_bar_init(&prg_bar, size);

	while (data < data_end)
	{
		for (p = data, tmp = *p; tmp == *p && p < data_end; p++);

		if (q >= end_q)
		{
			ret = ffile_writeto(fd_out, buff, q - buff, 0);
			if (ret < 0)
			{
				error_msg("write file failed");
				return NULL;
			}

			q = buff;
		}

		*q++ = p - data;
		*q++ = tmp;

		progress_bar_add(&prg_bar, p - data);

		data = p;
	}

	progress_bar_finish(&prg_bar);

	if (q > buff)
	{
		ret = ffile_writeto(fd_out, buff, q - buff, 0);
		if (ret < 0)
		{
			error_msg("write file failed");
			return NULL;
		}
	}

	return (char *) data;
}

int frle_to(int fd_in, int fd_out)
{
	int ret;
	struct stat st;
	const u8 *data_in;

	ret = fstat(fd_in, &st);
	if (ret < 0)
	{
		print_error("get input file size failed");
		return ret;
	}

	data_in = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd_in, 0);
	if (data_in == MAP_FAILED)
	{
		print_error("mmap input file failed");
		return -1;
	}

	if (frle_data_to(data_in, st.st_size, fd_out) == NULL)
	{
		ret = -1;
	}

	munmap((void *)data_in, st.st_size);

	return ret;
}

int rle_to(const char *file_in, const char *file_out)
{
	int ret;
	int fd_in, fd_out;

	fd_in = file_open_ro(file_in);
	if (fd_in < 0)
	{
		print_error("open input file \"%s\" failed", file_in);
		return -1;
	}

	fd_out = file_open_wo(file_out);
	if (fd_out < 0)
	{
		print_error("open output file \"%s\" failed", file_out);
		ret = -1;
		goto out_close_in;
	}

	ret = frle_to(fd_in, fd_out);

	close(fd_out);
out_close_in:
	close(fd_in);

	return ret;
}

char *funrle_data_to(const u8 *data, ssize_t size, int fd_out)
{
	int ret;
	const u8 *data_end;
	u8 tmp;
	char buff[MAX_BUFFER_LEN];
	char *p, *end_p;
	struct progress_bar prg_bar;

	progress_bar_init(&prg_bar, size);
	p = buff;
	end_p = buff + sizeof(buff);

	for (data_end = data + size; data < data_end; data += 2)
	{
		tmp = *data;

		println("tmp = %d", tmp);

		while (p + tmp >= end_p)
		{
			memset(p, data[1], end_p - p);

			ret = ffile_writeto(fd_out, buff, MAX_BUFFER_LEN, 0);
			if (ret < 0)
			{
				error_msg("write file failed");
				return NULL;
			}

			tmp -= end_p - p;
			p = buff;
		}

		memset(p, data[1], tmp);
		p += tmp;

		progress_bar_add(&prg_bar, 2);
	}

	progress_bar_finish(&prg_bar);

	if (p > buff)
	{
		ret = ffile_writeto(fd_out, buff, p - buff, 0);
		if (ret < 0)
		{
			error_msg("write file failed");
			return NULL;
		}
	}

	return (char *) data;
}

int funrle_to(int fd_in, int fd_out)
{
	int ret;
	struct stat st;
	const u8 *data_in;

	ret = fstat(fd_in, &st);
	if (ret < 0)
	{
		print_error("get input file size failed");
		return ret;
	}

	data_in = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd_in, 0);
	if (data_in == MAP_FAILED)
	{
		print_error("mmap input file failed");
		return -1;
	}

	if (funrle_data_to(data_in, st.st_size, fd_out) == NULL)
	{
		ret = -1;
	}

	munmap((void *)data_in, st.st_size);

	return ret;
}

int unrle_to(const char *file_in, const char *file_out)
{
	int ret;
	int fd_in, fd_out;

	fd_in = file_open_ro(file_in);
	if (fd_in < 0)
	{
		print_error("open input file \"%s\" failed", file_in);
		return -1;
	}

	fd_out = file_open_wo(file_out);
	if (fd_out < 0)
	{
		print_error("open output file \"%s\" failed", file_out);
		ret = -1;
		goto out_close_in;
	}

	ret = funrle_to(fd_in, fd_out);

	close(fd_out);
out_close_in:
	close(fd_in);

	return ret;
}

