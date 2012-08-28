#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#define X_AXIS_MAX		1024
#define X_AXIS_MIN		0
#define Y_AXIS_MAX		600
#define Y_AXIS_MIN		0
#define PRESSURE_MAX	255
#define PRESSURE_MIN	0

#define SWAN_VK_DEVICE_NAME "swan_vk"

#define pr_red_info(fmt, args ...) \
	printk(KERN_INFO "\033[31m" fmt "\033[0m\n", ##args)

#define pr_green_info(fmt, args ...) \
	printk(KERN_INFO "\033[32m" fmt "\033[0m\n", ##args)

#define pr_blue_info(fmt, args ...) \
	printk(KERN_INFO "\033[34m" fmt "\033[0m\n", ##args)

#define pr_bold_info(fmt, args ...) \
	printk(KERN_INFO "\033[1m" fmt "\033[0m\n", ##args)

#define pr_pos_info() \
	pr_green_info("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

struct swan_virtual_key
{
	char *name;
	unsigned int code;
};

static struct swan_virtual_key swan_vk_table[] =
{
	{"search", 217},
	{"back", 158},
	{"menu", 229},
	{"home", 102},
	{"center", 232},
	{"up", 103},
	{"down", 108},
	{"left", 105},
	{"right", 106},
	{"space", 57},
	{"tab", 15},
	{"enter", 28},
	{"power", 116},
	{"call", 61},
	{"endcall", 62},
	{"grave", 399},
	{"pound", 228},
	{"star", 227},
	{"vup", 115},
	{"vdown", 114},
	{"camera", 212},
	{"backslash", 43},
	{"del", 14}
};

struct swan_virtual_keypad
{
	struct input_dev *input;
};

static int strlhcmp(const char *str1, const char *str2)
{
	while (*str1)
	{
		if (*str1 != *str2)
		{
			return *str1 - *str2;
		}

		str1++;
		str2++;
	}

	return 0;
}

static void swan_virtual_keypad_setup_events(struct input_dev *vk_input)
{
	int i;

	__set_bit(EV_KEY, vk_input->evbit);

	for (i = 1; i < KEY_CNT; i++)
	{
		__set_bit(i, vk_input->keybit);
	}

	__set_bit(EV_ABS, vk_input->evbit);
	input_set_abs_params(vk_input, ABS_X, X_AXIS_MIN, X_AXIS_MAX, 0, 0);
	input_set_abs_params(vk_input, ABS_Y, Y_AXIS_MIN, Y_AXIS_MAX, 0, 0);
	input_set_abs_params(vk_input, ABS_PRESSURE, PRESSURE_MIN, PRESSURE_MAX, 0, 0);

	__set_bit(EV_REL, vk_input->evbit);
	__set_bit(REL_X, vk_input->relbit);
	__set_bit(REL_Y, vk_input->relbit);
	__set_bit(REL_WHEEL, vk_input->relbit);
}

static ssize_t swan_virtual_keypad_command_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	const struct swan_virtual_key *p, *end_p;
	struct swan_virtual_keypad *vk = dev_get_drvdata(dev);
	static unsigned int old_code;

	for (p = swan_vk_table, end_p = swan_vk_table + ARRAY_SIZE(swan_vk_table); p < end_p && strlhcmp(p->name, buff); p++);

	if (p < end_p)
	{
		old_code = p->code;
	}

	input_event(vk->input, EV_KEY, old_code, 1);
	input_event(vk->input, EV_KEY, old_code, 0);

	return count;
}

static ssize_t swan_virtual_keypad_keycode_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	unsigned int code;
	static unsigned old_code;
	struct swan_virtual_keypad *vk = dev_get_drvdata(dev);
	struct input_dev *vk_input = vk->input;

	code = simple_strtoul(buff, NULL, 10);
	if (code)
	{
		old_code = code;
	}

	input_event(vk_input, EV_KEY, old_code, 1);
	input_event(vk_input, EV_KEY, old_code, 0);

	return count;
}

static ssize_t swan_virtual_keypad_event_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	int type, code, value;
	struct swan_virtual_keypad *vk = dev_get_drvdata(dev);

	if (sscanf(buff, "%d,%d,%d", &type, &code, &value) == 3)
	{
		input_event(vk->input, type, code, value);
	}

	return count;
}

static ssize_t swan_virtual_keypad_data_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	struct input_event *p, *end_p;
	struct swan_virtual_keypad *vk = dev_get_drvdata(dev);

	for (p = (struct input_event *)buff, end_p = p + (count / sizeof(*p)); p < end_p; p++)
	{
		input_event(vk->input, p->type, p->code, p->value);
	}

	return count;
}

static ssize_t swan_virtual_keypad_value_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	struct swan_virtual_keypad *vk = dev_get_drvdata(dev);

	input_event(vk->input, EV_KEY, *(u32 *)buff, 1);
	input_event(vk->input, EV_KEY, *(u32 *)buff, 0);

	return count;
}

struct device_attribute vk_attrs[] =
{
	{
		.attr =
		{
			.name = "command",
//			.owner = THIS_MODULE,
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_command_store,
	},
	{
		.attr =
		{
			.name = "keycode",
//			.owner = THIS_MODULE,
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_keycode_store,
	},
	{
		.attr =
		{
			.name = "event",
//			.owner = THIS_MODULE,
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_event_store,
	},
	{
		.attr =
		{
			.name = "data",
//			.owner = THIS_MODULE,
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_data_store,
	},
	{
		.attr =
		{
			.name = "value",
//			.owner = THIS_MODULE,
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_value_store,
	}
};

static int swan_virtual_keypad_probe(struct platform_device *pdev)
{
	int ret;
	struct swan_virtual_keypad *vk;
	struct input_dev *vk_input;
	struct device_attribute *p;

	pr_pos_info();

	vk = kzalloc(sizeof(struct swan_virtual_keypad), GFP_KERNEL);
	if (vk == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, vk);

	vk_input = input_allocate_device();
	if (vk_input == NULL)
	{
		ret = -ENOMEM;
		pr_err("input_allocate_device");
		goto out_free_vk;
	}

	vk->input = vk_input;
	input_set_drvdata(vk_input, vk);

	vk_input->name = SWAN_VK_DEVICE_NAME;
	swan_virtual_keypad_setup_events(vk_input);

	ret = input_register_device(vk_input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_input_free_device;
	}

	for (p = vk_attrs + ARRAY_SIZE(vk_attrs) - 1; p >= vk_attrs; p--)
	{
		ret = device_create_file(&pdev->dev, p);
		if (ret < 0)
		{
			pr_red_info("device_create_file");

			while (--p >= vk_attrs)
			{
				device_remove_file(&pdev->dev, p);
			}

			goto out_input_unregister_device;
		}
	}

	pr_green_info("Swan Virtual Keypad is OK");

	return 0;

out_input_unregister_device:
	input_unregister_device(vk_input);
out_input_free_device:
	input_free_device(vk_input);
out_free_vk:
	kfree(vk);

	return ret;
}

static int swan_virtual_keypad_remove(struct platform_device *pdev)
{
	struct device_attribute *p;
	struct swan_virtual_keypad *vk = platform_get_drvdata(pdev);

	for (p = vk_attrs + ARRAY_SIZE(vk_attrs) - 1; p >= vk_attrs; p--)
	{
		device_remove_file(&pdev->dev, p);
	}

	input_unregister_device(vk->input);
	input_free_device(vk->input);
	kfree(vk);

	return 0;
}

static struct platform_driver swan_virtual_keypad_driver =
{
	.driver =
	{
		.name = SWAN_VK_DEVICE_NAME,
	},

	.probe = swan_virtual_keypad_probe,
	.remove = swan_virtual_keypad_remove,
};

static struct platform_device swan_virtual_keypad_device =
{
	.name = SWAN_VK_DEVICE_NAME,
};

static int __init swan_virtual_keypad_init(void)
{
	int ret;

	pr_pos_info();

	ret = platform_device_register(&swan_virtual_keypad_device);
	if (ret < 0)
	{
		pr_red_info("platform_device_register");
		return ret;
	}

	ret = platform_driver_register(&swan_virtual_keypad_driver);
	if (ret < 0)
	{
		pr_red_info("platform_driver_register");
		goto out_unregister_device;
	}

	return 0;

out_unregister_device:
	platform_device_unregister(&swan_virtual_keypad_device);

	return ret;
}

static void __exit swan_virtual_keypad_exit(void)
{
	pr_pos_info();

	platform_device_unregister(&swan_virtual_keypad_device);
	platform_driver_unregister(&swan_virtual_keypad_driver);
}

module_init(swan_virtual_keypad_init);
module_exit(swan_virtual_keypad_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Eavoo Swan Virtual Keypad Driver");
MODULE_LICENSE("GPL");
