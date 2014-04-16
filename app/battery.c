// Fuang.Cao: Tue Dec 28 23:50:12 CST 2010

#include <cavan.h>
#include <cavan/bmp.h>
#include <cavan/file.h>
#include <cavan/text.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int i;
	int ret;
	int fd_in;
	int fd_out;
	struct bmp_file_header file_hdr;

	assert(argc == 3);

	fd_in = open(argv[1], O_RDONLY | O_BINARY);
	if (fd_in < 0)
	{
		print_error("open file \"%s\"", argv[1]);
		return -1;
	}

	if (file_test(argv[2], "d") < 0)
	{
		cavan_mkdir(argv[2]);
	}

	for (i = 0; i < 10; i++)
	{
		ret = bmp_read_file_header(fd_in, &file_hdr);
		if (ret < 0)
		{
			break;
		}

		fd_out = open(format_text("%s/%d.bmp", argv[2], i), O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0777);
		if (fd_out < 0)
		{
			print_error("open target file");
			break;
		}

		ret = ffile_write(fd_out, &file_hdr, sizeof(file_hdr));
		if (ret < (int)sizeof(file_hdr))
		{
			print_error("write file header");
			goto out_close_out;
		}

		ret = ffile_ncopy(fd_in, fd_out, file_hdr.size - sizeof(file_hdr));
		if (ret < 0)
		{
			error_msg("write file");
			goto out_close_out;
		}

		close(fd_out);
	}

	ret = 0;
	goto out_close_in;
out_close_out:
	close(fd_out);
out_close_in:
	close(fd_in);

	return ret;
}
