#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

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

static struct input_dev *vk_input;

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

static void swan_vk_touch(struct input_dev *input, unsigned int code)
{
	input_event(input, EV_KEY, code, 1);
	input_event(input, EV_KEY, code, 0);
	input_sync(input);
}

static ssize_t swan_virtual_keypad_command_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	const struct swan_virtual_key *p, *end_p;
	static unsigned int old_code;

	pr_pos_info();

	for (p = swan_vk_table, end_p = swan_vk_table + ARRAY_SIZE(swan_vk_table); p < end_p && strlhcmp(p->name, buff); p++);

	if (p < end_p)
	{
		old_code = p->code;
	}

	pr_green_info("keycode = %d", old_code);
	swan_vk_touch(vk_input, old_code);

	return count;
}

static ssize_t swan_virtual_keypad_keycode_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	unsigned int code;
	static unsigned old_code;

	code = simple_strtoul(buff, NULL, 10);
	if (code)
	{
		old_code = code;
	}

	swan_vk_touch(vk_input, old_code);

	return count;
}

static ssize_t swan_virtual_keypad_event_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	int type, code, value;

	if (sscanf(buff, "%d,%d,%d", &type, &code, &value) == 3)
	{
		input_event(vk_input, type, code, value);
	}

	return count;
}

static ssize_t swan_virtual_keypad_data_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	struct input_event *p, *end_p;

	for (p = (struct input_event *)buff, end_p = p + (count / sizeof(*p)); p < end_p; p++)
	{
		input_event(vk_input, p->type, p->code, p->value);
	}

	return count;
}

static ssize_t swan_virtual_keypad_value_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	swan_vk_touch(vk_input, *(u32 *)buff);

	return count;
}

struct device_attribute vk_attrs[] =
{
	{
		.attr =
		{
			.name = "command",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_command_store,
	},
	{
		.attr =
		{
			.name = "keycode",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_keycode_store,
	},
	{
		.attr =
		{
			.name = "event",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_event_store,
	},
	{
		.attr =
		{
			.name = "data",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_data_store,
	},
	{
		.attr =
		{
			.name = "value",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_keypad_value_store,
	}
};

static int swan_vk_misc_open(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

static int swan_vk_misc_release(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

static ssize_t swan_vk_misc_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	pr_pos_info();

	return 0;
}

static ssize_t swan_vk_misc_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct input_event *p, *end_p;

	for (p = (struct input_event *)buff, end_p = p + (size / sizeof(*p)); p < end_p; p++)
	{
		input_event(vk_input, p->type, p->code, p->value);
	}

	return size;
}

static const struct file_operations swan_vk_misc_fops =
{
	.owner = THIS_MODULE,
	.open = swan_vk_misc_open,
	.read = swan_vk_misc_read,
	.write = swan_vk_misc_write,
	.release = swan_vk_misc_release
};

static struct miscdevice swan_vk_misc =
{
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "swan_vk",
	.fops	= &swan_vk_misc_fops,
};

static int swan_virtual_keypad_probe(struct platform_device *pdev)
{
	int ret;
	struct device_attribute *p;

	pr_pos_info();

	vk_input = input_allocate_device();
	if (vk_input == NULL)
	{
		pr_red_info("input_allocate_device");
		return -ENOMEM;
	}

	vk_input->name = SWAN_VK_DEVICE_NAME;
	swan_virtual_keypad_setup_events(vk_input);

	ret = input_register_device(vk_input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_input_free_device;
	}

	ret = misc_register(&swan_vk_misc);
	if (ret < 0)
	{
		pr_red_info("misc_register failed");
		goto out_input_unregister_device;
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

			goto out_misc_deregister;
		}
	}

	pr_green_info("Swan Virtual Keypad is OK");

	return 0;

out_misc_deregister:
	misc_deregister(&swan_vk_misc);
out_input_unregister_device:
	input_unregister_device(vk_input);
out_input_free_device:
	input_free_device(vk_input);

	return ret;
}

static int swan_virtual_keypad_remove(struct platform_device *pdev)
{
	struct device_attribute *p;

	for (p = vk_attrs + ARRAY_SIZE(vk_attrs) - 1; p >= vk_attrs; p--)
	{
		device_remove_file(&pdev->dev, p);
	}

	misc_deregister(&swan_vk_misc);
	input_unregister_device(vk_input);
	input_free_device(vk_input);
	vk_input = NULL;

	return 0;
}

static void swan_vk_platform_device_release(struct device *dev)
{
	pr_pos_info();
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
	.dev =
	{
		.release = swan_vk_platform_device_release,
	},
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
