// Fuang.Cao <cavan.cfa@gmail.com> Mon Jan 24 16:15:11 CST 2011

#include <cavan.h>
#include <cavan/input.h>
#include <cavan/file.h>
#include <cavan/event.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *dev_path;
	struct event_desc desc;
	char name[64];

	assert(argc == 2);

	if (file_test(argv[1], "c") < 0)
	{
		sprintf(name, "/dev/input/event%s", argv[1]);
		dev_path = name;
	}
	else
	{
		dev_path = argv[1];
	}

	println("dev_path = %s", dev_path);

	ret = event_init_by_path(&desc, dev_path);
	if (ret < 0)
	{
		error_msg("event_init_by_path");
		return ret;
	}

	println("Input device name = %s", desc.dev_name);

	while (1)
	{
		struct input_event event;
		char print_buff[1024];

		ret = read_events(&desc, &event, 1);
		if (ret < 0)
		{
			error_msg("read_events");
			goto out_event_uninit;
		}

		if (ret)
		{
			print_string(event_to_text(&event, print_buff));
		}
	}

out_event_uninit:
	event_uninit(&desc);

	return ret;
}

