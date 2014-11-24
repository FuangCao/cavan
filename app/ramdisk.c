#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/image.h>

int main(int argc, char *argv[])
{
	assert(argc == 3);

	if (file_test(argv[1], "b") >= 0)
	{
		if (image_is(argv[2], "busybox.img"))
		{
			return get_uramdisk(argv[1], argv[2], 1);
		}
		else if (image_is(argv[2], "uramdisk.img"))
		{
			return get_uramdisk(argv[1], argv[2], 0);
		}
		else if (image_is(argv[2], "ramdisk.img"))
		{
			return get_ramdisk(argv[1], argv[2], 0);
		}
		else if (image_is(argv[2], "busybox"))
		{
			return get_dump_uramdisk(argv[1], argv[2], 1);
		}
		else
		{
			return get_dump_uramdisk(argv[1], argv[2], 0);
		}
	}
	else if (file_test(argv[1], "d") >= 0)
	{
		if (image_is_uramdisk(argv[2]))
		{
			return create_uramdisk(argv[1], argv[2]);
		}
		else
		{
			return create_ramdisk(argv[1], argv[2]);
		}
	}
	else if (image_is_uramdisk(argv[1]))
	{
		if (image_is(argv[2], "ramdisk.img"))
		{
			return uramdisk2ramdisk(argv[1], argv[2]);
		}
		else
		{
			return dump_uramdisk(argv[1], argv[2]);
		}
	}
	else if (image_is_uramdisk(argv[2]))
	{
		return ramdisk2uramdisk(argv[1], argv[2]);
	}
	else
	{
		return dump_ramdisk(argv[1], argv[2]);
	}

	return -1;
}
