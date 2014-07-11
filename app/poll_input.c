/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 19 15:31:40 CST 2012
 */

#include <cavan.h>
#include <cavan/input.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *dev_path;
	char name[64];
	struct cavan_input_service service;

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

	cavan_input_service_init(&service, cavan_event_simple_matcher);
	service.lcd_width = service.lcd_height = 100;
	ret = cavan_input_service_start(&service, (void *) dev_path);
	if (ret < 0)
	{
		pr_error_info("cavan_input_service_start");
		return ret;
	}

	cavan_input_service_join(&service);
	cavan_input_service_stop(&service);

	return 0;
}
