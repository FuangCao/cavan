#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/progress.h>
#include <cavan/file.h>
#include <cavan/text.h>
#include <cavan/device.h>

#define MAX_BUFFER_SIZE		MB(1)

int fcavan_dd(int fd_in, int fd_out, off_t offset_in, off_t offset_out, off_t length, off_t size)
{
	int ret;
	struct stat st;

	println("size = %s", size2text(size));

	ret = fstat(fd_in, &st);
	if (ret < 0)
	{
		print_error("fstat input file");
		return ret;
	}

	if (length == 0)
	{
		length = st.st_size;
		if (length <= offset_in)
		{
			warning_msg("no data to be burn");
			return 0;
		}
		length -= offset_in;
	}

	if (size && length > size)
	{
		error_msg("No space left");
		ERROR_RETURN(ENOMEDIUM);
	}

	if (offset_in > 0)
	{
		ret = lseek(fd_in, offset_in, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek input file");
			return ret;
		}
	}

	if (offset_out > 0)
	{
		ret = lseek(fd_out, offset_out, SEEK_SET);
		if (ret < 0)
		{
			print_error("seek output file");
			return ret;
		}
	}

	println("skip = %s", size2text(offset_in));
	println("seek = %s", size2text(offset_out));
	println("length = %s", size2text(length));

	ret = ffile_ncopy(fd_in, fd_out, length);
	if (ret < 0)
	{
		error_msg("file_ncopy");
	}

	return ret;
}

int cavan_dd_base(const char *file_in, const char *file_out, off_t offset_in, off_t offset_out, off_t length, int flags)
{
	int fd_in, fd_out;
	int ret;
	off_t size;

	println("%s [dd]-> %s", file_in, file_out);

	ret = get_first_partition_offset(file_out, &size);
	if (ret < 0)
	{
		size = 0;
	}

	fd_in = open(file_in, O_RDONLY | O_BINARY);
	if (fd_in < 0)
	{
		print_error("open input file");
		return -1;
	}

	fd_out = open(file_out, O_WRONLY | O_CREAT | O_SYNC | O_BINARY | flags, 0777);
	if (fd_out < 0)
	{
		print_error("open output file");
		ret = -1;
		goto out_close_fd_in;
	}

	ret = fcavan_dd(fd_in, fd_out, offset_in, offset_out, length, size);
	if (ret < 0)
	{
		error_msg("fcavan_dd");
		goto out_close_fd_out;
	}

	ret = 0;

out_close_fd_out:
	close(fd_out);
out_close_fd_in:
	close(fd_in);

	return ret;
}

int cavan_dds(struct dd_desc *descs, size_t count)
{
	while (count)
	{
		int ret;

		ret = cavan_dd2(descs);
		if (ret < 0)
		{
			return ret;
		}

		descs++;
	}

	return 0;
}
