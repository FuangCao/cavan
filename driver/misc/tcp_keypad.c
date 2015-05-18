#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define TCP_KEYPAD_DEVICE_NAME "tcp_keypad"

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

struct cavan_input_event
{
	__u16 type;
	__u16 code;
	__s32 value;
};

static struct input_dev *vk_input;

static void tcp_keypad_setup_events(struct input_dev *vk_input)
{
	int i;

	__set_bit(EV_KEY, vk_input->evbit);

	for (i = 1; i < KEY_CNT; i++)
	{
		__set_bit(i, vk_input->keybit);
	}

	__set_bit(EV_REL, vk_input->evbit);
	__set_bit(REL_X, vk_input->relbit);
	__set_bit(REL_Y, vk_input->relbit);
	__set_bit(REL_WHEEL, vk_input->relbit);
}

static int tcp_keypad_misc_open(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

static int tcp_keypad_misc_release(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

static ssize_t tcp_keypad_misc_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	pr_pos_info();

	return 0;
}

static ssize_t tcp_keypad_misc_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct cavan_input_event *p, *end_p;

	for (p = (struct cavan_input_event *)buff, end_p = p + (size / sizeof(*p)); p < end_p; p++)
	{
		input_event(vk_input, p->type, p->code, p->value);
	}

	return size;
}

static const struct file_operations tcp_keypad_misc_fops =
{
	.owner = THIS_MODULE,
	.open = tcp_keypad_misc_open,
	.read = tcp_keypad_misc_read,
	.write = tcp_keypad_misc_write,
	.release = tcp_keypad_misc_release
};

static struct miscdevice tcp_keypad_misc =
{
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "tcp_keypad",
	.fops	= &tcp_keypad_misc_fops,
};

static int tcp_keypad_probe(struct platform_device *pdev)
{
	int ret;

	pr_pos_info();

	vk_input = input_allocate_device();
	if (vk_input == NULL)
	{
		pr_red_info("input_allocate_device");
		return -ENOMEM;
	}

	vk_input->name = TCP_KEYPAD_DEVICE_NAME;
	tcp_keypad_setup_events(vk_input);

	ret = input_register_device(vk_input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_input_free_device;
	}

	ret = misc_register(&tcp_keypad_misc);
	if (ret < 0)
	{
		pr_red_info("misc_register failed");
		goto out_input_unregister_device;
	}

	pr_green_info("Swan Virtual Keypad is OK");

	return 0;

out_input_unregister_device:
	input_unregister_device(vk_input);
out_input_free_device:
	input_free_device(vk_input);

	return ret;
}

static int tcp_keypad_remove(struct platform_device *pdev)
{
	misc_deregister(&tcp_keypad_misc);
	input_unregister_device(vk_input);
	input_free_device(vk_input);
	vk_input = NULL;

	return 0;
}

static void tcp_keypad_platform_device_release(struct device *dev)
{
	pr_pos_info();
}

static struct platform_driver tcp_keypad_driver =
{
	.driver =
	{
		.name = TCP_KEYPAD_DEVICE_NAME,
	},

	.probe = tcp_keypad_probe,
	.remove = tcp_keypad_remove,
};

static struct platform_device tcp_keypad_device =
{
	.name = TCP_KEYPAD_DEVICE_NAME,
	.dev =
	{
		.release = tcp_keypad_platform_device_release,
	},
};

static int __init tcp_keypad_init(void)
{
	int ret;

	pr_pos_info();

	ret = platform_device_register(&tcp_keypad_device);
	if (ret < 0)
	{
		pr_red_info("platform_device_register");
		return ret;
	}

	ret = platform_driver_register(&tcp_keypad_driver);
	if (ret < 0)
	{
		pr_red_info("platform_driver_register");
		goto out_unregister_device;
	}

	return 0;

out_unregister_device:
	platform_device_unregister(&tcp_keypad_device);

	return ret;
}

static void __exit tcp_keypad_exit(void)
{
	pr_pos_info();

	platform_device_unregister(&tcp_keypad_device);
	platform_driver_unregister(&tcp_keypad_driver);
}

module_init(tcp_keypad_init);
module_exit(tcp_keypad_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("TCP Keypad Driver");
MODULE_LICENSE("GPL");
