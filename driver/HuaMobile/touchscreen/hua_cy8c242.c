#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include "hua_ts_core.h"

#define CY8C242_XAXIS_MIN			0
#define CY8C242_XAXIS_MAX			320
#define CY8C242_YAXIS_MIN			0
#define CY8C242_YAXIS_MAX			480
#define CY8C242_POINT_COUNT			2
#define CY8C242_DEVICE_NAME			"cy8c242_ts"

#define CY8C242_REG_DEVICE_MODE		0x00
#define CY8C242_REG_GEST_ID			0x01
#define CY8C242_REG_TD_STATUS		0x02
#define CY8C242_REG_POINTS			0x03
#define CY8C242_REG_POWER_MODE		0xA5

#define CY8C242_MODE_ACTIVE			0x00
#define CY8C242_MODE_MONITOR		0x01
#define CY8C242_MODE_STANDBY		0x02
#define CY8C242_MODE_HIBERNATE		0x03

#define CY8C242_FIRMWARE_MAX_SIZE	(50 << 10)
#define CY8C242_FIRMWARE_BLOCK_SIZE	128

#define CY8C242_BUILD_AXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define CY8C242_EVENT_FLAG(xh) \
	((xh) >> 4)

#define CY8C242_TOUCH_ID(yh) \
	((yh) >> 4)

#pragma pack(1)
struct cy8c242_touch_point
{
	u8 xh;
	u8 xl;
	u8 yh;
	u8 yl;
	u16 reserved;
};

struct cy8c242_data_package
{
	u8 device_mode;
	u8 gest_id;
	u8 td_status;
	struct cy8c242_touch_point points[CY8C242_POINT_COUNT];
};
#pragma pack()

static inline ssize_t cy8c242_data_package(struct i2c_client *client, struct cy8c242_data_package *package)
{
	return huamobile_i2c_read_data(client, 0, package, sizeof(*package));
}

static const char *cy8c242_power_mode_tostring(int mode)
{
	switch (mode)
	{
	case CY8C242_MODE_ACTIVE:
		return "Active";
	case CY8C242_MODE_MONITOR:
		return "Monitor";
	case CY8C242_MODE_STANDBY:
		return "Standby";
	case CY8C242_MODE_HIBERNATE:
		return "Hibernate";
	default:
		return "Unknown";
	}
}

static int cy8c242_change_power_mode(struct i2c_client *client, u8 mode, int retry)
{
	pr_bold_info("Ft5216 change power mode => %s", cy8c242_power_mode_tostring(mode));

	while (huamobile_i2c_write_register(client, CY8C242_REG_POWER_MODE, mode) < 0 && retry--)
	{
		msleep(10);
	}

	if (retry < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	return retry;
}

static int cy8c242_event_loop(struct huamobile_ts_device *ts)
{
	int ret;
	int count;
	struct i2c_client *client = ts->client;
	struct input_dev *input = ts->input;
	struct completion *event_completion = &ts->event_completion;
	struct cy8c242_data_package package;
	struct cy8c242_touch_point *p, *p_end;

	while (1)
	{
		wait_for_completion(event_completion);
		if (unlikely(huamobile_ts_should_stop(ts)))
		{
			break;
		}

		ret = cy8c242_data_package(client, &package);
		if (ret < 0)
		{
			pr_red_info("cy8c242_read_data_package");
			continue;
		}

		count = package.td_status & 0x0F;
		if (count == 0)
		{
			huamobile_mt_touch_release(input);
			continue;
		}

		if (unlikely(count > CY8C242_POINT_COUNT))
		{
			// pr_red_info("Too much points = %d", count);
			count = CY8C242_POINT_COUNT;
		}

		for (p = package.points, p_end = p + count; p < p_end; p++)
		{
			huamobile_report_mt_data(input, CY8C242_BUILD_AXIS(p->xh, p->xl), \
				CY8C242_BUILD_AXIS(p->yh, p->yl));
		}

		input_sync(input);
	}

	return 0;
}

static int cy8c242_suspend_enable(struct huamobile_ts_device *ts, bool enable)
{
	if (enable)
	{
		return cy8c242_change_power_mode(ts->client, CY8C242_MODE_HIBERNATE, 2);
	}
	else
	{
		return cy8c242_change_power_mode(ts->client, CY8C242_MODE_ACTIVE, 10);
	}
}

static int cy8c242_power_enable(struct huamobile_ts_device *ts, bool enable)
{
	sprd_ts_power_enable(false);

	if (enable)
	{
		sprd_ts_reset_enable(true);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_reset_enable(false);
	}

	return 0;
}

static struct huamobile_touch_key cy8c242_touch_keys[] =
{
	{
		.code = KEY_MENU,
		.x = 40,
		.y = 550,
		.width = 120,
		.height = 120,
	},
	{
		.code = KEY_HOME,
		.x = 160,
		.y = 550,
		.width = 120,
		.height = 120,
	},
	{
		.code = KEY_BACK,
		.x = 280,
		.y = 550,
		.width = 120,
		.height = 120,
	}
};

static int cy8c242_readid(struct huamobile_ts_device *ts)
{
	int ret;
	char buff[18];
	struct i2c_client *client = ts->client;

	pr_pos_info();

	ret = huamobile_i2c_read_data(client, 0x00, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	huamobile_print_memory(buff, ret);

	ts->id_size = 2;
	ts->dev_id[0] = buff[0x10];
	ts->dev_id[1] = buff[0x11];

	return 0;
}

static int cy8c242_enter_upgrade_mode(struct i2c_client *client, int retry)
{
	while (retry--)
	{
		int ret;
		u8 value;

		sprd_ts_power_enable(false);
		sprd_ts_irq_output(0);
		msleep(10);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_irq_output(-1);

		ret = huamobile_i2c_read_register(client, 0x01, &value);
		if (ret < 0)
		{
			pr_red_info("huamobile_i2c_read_register");
			return ret;
		}

		pr_bold_info("value = 0x%02x", value);

		if (value == 0x11 || value == 0x10)
		{
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static void cy8c242_ts_reset(void)
{
	sprd_ts_reset_enable(true);
	msleep(100);
	sprd_ts_reset_enable(false);
}

static int cy8c242_exit_upgrade_mode(struct i2c_client *client, int retry)
{
	int ret;
	u8 value;

	msleep(200);

	while (retry--)
	{
		cy8c242_ts_reset();

		msleep(100);

		ret = huamobile_i2c_read_register(client, 0x01, &value);
		if (ret < 0)
		{
			pr_red_info("huamobile_i2c_read_register");
			return ret;
		}

		pr_bold_info("value = 0x%02x", value);

		if (value != 0x11 && value != 0x10)
		{
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static int cy8c242_byte2value(const char byte)
{
	switch (byte)
	{
	case '0' ... '9':
		return byte - '0';

	case 'a' ... 'f':
		return byte - 'a' + 10;

	case 'A' ... 'F':
		return byte - 'A' + 10;

	case 'p':
		return -2;

	default:
		return -1;
	}
}

static int cy8c242_firmware2data(const char *p, const char *end, u8 *datas)
{
	int ret;
	int count;

	for (count = 0; p < end; p++)
	{
		ret = cy8c242_byte2value(*p);
		if (ret < 0)
		{
			if (ret == -2)
			{
				break;
			}

			continue;
		}

		p++;
		datas[count++] = ret << 4 | cy8c242_byte2value(*p);
	}

	return count;
}

static int cy8c242_firmware_write_data(struct i2c_client *client, const char *buff, size_t size)
{
	int ret;
	u32 delay;
	const char *p, *end_line, *end_file;
	u8 datas[32];

	pr_pos_info();

	if (size < 100)
	{
		pr_red_info("file size too small");
		return 0;
	}

	for (p = buff, end_file = p + size; p < end_file; p = end_line + 1)
	{
		if (((p - buff) & 0x0F) == 0)
		{
			pr_bold_info("Remain %d byte", end_file - p);
		}

		while (BYTE_IS_SPACE(*p))
		{
			if (p < end_file)
			{
				p++;
			}
			else
			{
				return 0;
			}
		}

		for (end_line = p; end_line < end_file; end_line++)
		{
			if (BYTE_IS_LF(*end_line))
			{
				break;
			}
		}

		if (p == end_line)
		{
			continue;
		}

		switch (*p)
		{
		case 'w':
		case 'W':
			ret = cy8c242_firmware2data(p + 2, end_line, datas);
			ret = i2c_master_send(client, datas + 1, ret - 1);
			if (ret < 0)
			{
				pr_red_info("i2c_master_send");
				return ret;
			}
			break;

		case 'r':
		case 'R':
			ret = i2c_master_recv(client, datas, 3);
			if (ret < 0)
			{
				pr_red_info("i2c_master_recv");
				return ret;
			}

			if (datas[2] != 0x20)
			{
				pr_red_info("datas[2] = 0x%02x", datas[2]);
				return -EFAULT;
			}
			break;

		case '[':
			ret = sscanf(p, "[delay=%d]", &delay);
			if (ret == 1)
			{
				msleep(delay >> 1);
			}
			else
			{
				pr_bold_info("unknown delay");
			}
			break;

		case '#':
			pr_green_info("Comment Line");
			break;

		default:
			pr_red_info("unknown char 0x%02x at %d", *p, p - buff);
		}
	}

	return 0;
}

static int cy8c242_firmware_upgrade(struct huamobile_ts_device *ts, const void *buff, size_t size)
{
	int ret;

	pr_pos_info();

	pr_bold_info("buff size = %d", size);

	ret = cy8c242_enter_upgrade_mode(ts->client, 10);
	if (ret < 0)
	{
		pr_red_info("cy8c242_enter_upgrade_mode");
		return ret;
	}

	ret = cy8c242_firmware_write_data(ts->client, buff, size);
	if (ret < 0)
	{
		pr_red_info("cy8c242_firmware_write_data");
		return ret;
	}

	ret = cy8c242_exit_upgrade_mode(ts->client, 10);
	if (ret < 0)
	{
		pr_red_info("cy8c242_exit_upgrade_mode");
		return ret;
	}

	cy8c242_readid(ts);

	return 0;
}

static int cy8c242_calibration(struct huamobile_ts_device *ts)
{
	int ret;
	u8 value;
	int i;
	struct i2c_client *client = ts->client;
	char buff[2] = {0x1C, 0x01};

	pr_pos_info();

	for (i = 0; i < 5; i++)
	{
		cy8c242_ts_reset();

		msleep(100);

		ret = i2c_master_send(client, buff, sizeof(buff));
		if (ret < 0)
		{
			pr_red_info("i2c_master_send");
			return ret;
		}

		msleep(1000);

		ret = i2c_master_send(client, buff, 1);
		if (ret < 0)
		{
			pr_red_info("i2c_master_send");
			return ret;
		}

		ret = i2c_master_recv(client, &value, 1);
		if (ret < 0)
		{
			pr_red_info("i2c_master_recv");
			return ret;
		}

		pr_bold_info("value[%d] = %d", i, value);

		if (value == 0)
		{
			return 0;
		}
	}

	return -EFAULT;
}

static int cy8c242_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct huamobile_ts_device *ts;

	pr_pos_info();

	ts = kzalloc(sizeof(struct huamobile_ts_device), GFP_KERNEL);
	if (ts == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	ts->client = client;
	ts->irq_flags = IRQF_TRIGGER_FALLING;

	ts->name = "CY8C242";
	ts->xmin = CY8C242_XAXIS_MIN;
	ts->xmax = CY8C242_XAXIS_MAX;
	ts->ymin = CY8C242_YAXIS_MIN;
	ts->ymax = CY8C242_YAXIS_MAX;
	ts->point_count = CY8C242_POINT_COUNT;

	ts->key_count = ARRAY_SIZE(cy8c242_touch_keys);
	ts->keys = cy8c242_touch_keys;

	ts->event_loop = cy8c242_event_loop;
	ts->suspend_enable = cy8c242_suspend_enable;
	ts->power_enable = cy8c242_power_enable;
	ts->readid = cy8c242_readid;

	ts->firmware_size = KB(180);
	ts->firmware_upgrade = cy8c242_firmware_upgrade;
	ts->calibration = cy8c242_calibration;

	ret = huamobile_ts_register(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_register");
		goto out_kfree_ts;
	}

	pr_green_info("CY8C242 Probe Complete");
	return 0;

out_kfree_ts:
	kfree(ts);
	return ret;
}

static int cy8c242_ts_remove(struct i2c_client *client)
{
	struct huamobile_ts_device *ts = i2c_get_clientdata(client);

	huamobile_ts_unregister(ts);
	kfree(ts);

	return 0;
}

static const struct i2c_device_id cy8c242_ts_id_table[] =
{
	{CY8C242_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver cy8c242_ts_driver =
{
	.probe = cy8c242_ts_probe,
	.remove = cy8c242_ts_remove,

	.id_table = cy8c242_ts_id_table,
	.driver =
	{
		.name = CY8C242_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static int __init cy8c242_ts_init(void)
{
	return i2c_add_driver(&cy8c242_ts_driver);
}

static void __exit cy8c242_ts_exit(void)
{
	i2c_del_driver(&cy8c242_ts_driver);
}

module_init(cy8c242_ts_init);
module_exit(cy8c242_ts_exit);

MODULE_AUTHOR("<Fuang.Cao cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile CY8C242 TouchScreen Driver");
MODULE_LICENSE("GPL");
