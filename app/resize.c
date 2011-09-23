#include <cavan.h>
#include <cavan/text.h>
#include <cavan/image.h>

int main(int argc, char *argv[])
{
	int ret;
	off_t size;
	struct stat st;

	assert(argc > 1);

	if (argc == 2)
	{
		ret = image_shrink(argv[1]);
		if (ret < 0)
		{
			error_msg("image_shrink");
		}

		return ret;
	}

	if (stat(argv[1], &st) < 0)
	{
		print_error("get file size failed");
		return -1;
	}

	switch (argv[2][0])
	{
	case '+':
		size = st.st_size + text2size(argv[2] + 1);
		break;
	case '-':
		size = text2size(argv[2] + 1);
		if (size >= st.st_size)
		{
			size = 0;
		}
		else
		{
			size = st.st_size - size;
		}
		break;
	case '0' ... '9':
		size = text2size(argv[2]);
		break;
	default:
		error_msg("argument error");
		return -1;
	}

	if (size <= 0)
	{
		error_msg("dest size <= 0");
		return -1;
	}
	else if (size == st.st_size)
	{
		warning_msg("size is no change");
		return 0;
	}

	ret = image_resize(argv[1], size);
	if (ret < 0)
	{
		error_msg("image_resize");
	}

	return ret;
}
