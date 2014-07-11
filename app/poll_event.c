// Fuang.Cao <cavan.cfa@gmail.com> Mon Jan 24 16:15:11 CST 2011

#include <cavan.h>
#include <cavan/input.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *dev_path;
	char name[64];
	struct cavan_event_service service;

	assert(argc < 3);

	if (argc > 1)
	{
		if (file_test(argv[1], "c") < 0 && text_is_number(argv[1]))
		{
			sprintf(name, "/dev/input/event%s", argv[1]);
			dev_path = name;
		}
		else
		{
			dev_path = argv[1];
		}
	}
	else
	{
		dev_path = NULL;
	}

	cavan_event_service_init(&service, cavan_event_simple_matcher);
	ret = cavan_event_service_start(&service, (void *) dev_path);
	if (ret < 0)
	{
		pr_red_info("cavan_event_service_start");
		return ret;
	}

	cavan_event_service_join(&service);
	cavan_event_service_stop(&service);

	return ret;
}

