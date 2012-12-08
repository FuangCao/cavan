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

#define FT5216_XAXIS_MIN			0
#define FT5216_XAXIS_MAX			320
#define FT5216_YAXIS_MIN			0
#define FT5216_YAXIS_MAX			480
#define FT5216_POINT_COUNT			5
#define FT5216_DEVICE_NAME			"ft5216_ts"

#define FT5216_REG_DEVICE_MODE		0x00
#define FT5216_REG_GEST_ID			0x01
#define FT5216_REG_TD_STATUS		0x02
#define FT5216_REG_POINTS			0x03
#define FT5216_REG_POWER_MODE		0xA5

#define FT5216_MODE_ACTIVE			0x00
#define FT5216_MODE_MONITOR			0x01
#define FT5216_MODE_STANDBY			0x02
#define FT5216_MODE_HIBERNATE		0x03

#define FT5216_FIRMWARE_MAX_SIZE	(50 << 10)
#define FT5216_FIRMWARE_BLOCK_SIZE	128

#define FT5216_BUILD_AXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define FT5216_EVENT_FLAG(xh) \
	((xh) >> 4)

#define FT5216_TOUCH_ID(yh) \
	((yh) >> 4)

#pragma pack(1)
struct ft5216_touch_point
{
	u8 xh;
	u8 xl;
	u8 yh;
	u8 yl;
	u16 reserved;
};

struct ft5216_data_package
{
	u8 device_mode;
	u8 gest_id;
	u8 td_status;
	struct ft5216_touch_point points[FT5216_POINT_COUNT];
};
#pragma pack()

static inline ssize_t ft5216_data_package(struct i2c_client *client, struct ft5216_data_package *package)
{
	return huamobile_i2c_read_data(client, 0, package, sizeof(*package));
}

static const char *ft5216_power_mode_tostring(int mode)
{
	switch (mode)
	{
	case FT5216_MODE_ACTIVE:
		return "Active";
	case FT5216_MODE_MONITOR:
		return "Monitor";
	case FT5216_MODE_STANDBY:
		return "Standby";
	case FT5216_MODE_HIBERNATE:
		return "Hibernate";
	default:
		return "Unknown";
	}
}

static int ft5216_change_power_mode(struct i2c_client *client, u8 mode, int retry)
{
	pr_bold_info("Ft5216 change power mode => %s", ft5216_power_mode_tostring(mode));

	while (huamobile_i2c_write_register(client, FT5216_REG_POWER_MODE, mode) < 0 && retry--)
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

static int ft5216_event_loop(struct huamobile_ts_device *ts)
{
	int ret;
	int count;
	struct i2c_client *client = ts->client;
	struct input_dev *input = ts->input;
	struct completion *event_completion = &ts->event_completion;
	struct ft5216_data_package package;
	struct ft5216_touch_point *p, *p_end;

	while (1)
	{
		wait_for_completion(event_completion);
		if (unlikely(huamobile_ts_should_stop(ts)))
		{
			break;
		}

		ret = ft5216_data_package(client, &package);
		if (ret < 0)
		{
			pr_red_info("ft5216_read_data_package");
			continue;
		}

		count = package.td_status & 0x0F;
		if (count == 0)
		{
			huamobile_mt_touch_release(input);
			continue;
		}

		if (unlikely(count > FT5216_POINT_COUNT))
		{
			pr_red_info("Too much points = %d", count);
			count = FT5216_POINT_COUNT;
		}

		for (p = package.points, p_end = p + count; p < p_end; p++)
		{
			huamobile_report_mt_data2(input, FT5216_TOUCH_ID(p->yh), \
				FT5216_BUILD_AXIS(p->xh, p->xl), FT5216_BUILD_AXIS(p->yh, p->yl));
		}

		input_sync(input);
	}

	return 0;
}

static int ft5216_suspend_enable(struct huamobile_ts_device *ts, bool enable)
{
	if (enable)
	{
		return ft5216_change_power_mode(ts->client, FT5216_MODE_HIBERNATE, 2);
	}
	else
	{
		return ft5216_change_power_mode(ts->client, FT5216_MODE_ACTIVE, 10);
	}
}

static int ft5216_power_enable(struct huamobile_ts_device *ts, bool enable)
{
	sprd_ts_power_enable(false);

	if (enable)
	{
		sprd_ts_reset_enable(false);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_reset_enable(true);
	}

	return 0;
}

static int ft5216_readid(struct huamobile_ts_device *ts)
{
	int ret;
	char buff[94];
	const u8 *p, *p_end;
	struct i2c_client *client = ts->client;

	pr_pos_info();

	ret = huamobile_i2c_read_data(client, 0x22, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	huamobile_print_memory(buff, ret);

	for (p = buff, p_end = p + 29; p < p_end; p++)
	{
		if (*p != 0xFF)
		{
			return -EINVAL;
		}
	}

	for (p_end = buff + sizeof(buff); p < p_end; p++)
	{
		if (*p != 0)
		{
			return -EINVAL;
		}
	}

	return 0;
}

static struct huamobile_touch_key ft5216_touch_keys[] =
{
	{
		.code = KEY_MENU,
		.x = 50,
		.y = 513,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_HOME,
		.x = 170,
		.y = 513,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_BACK,
		.x = 290,
		.y = 513,
		.width = 120,
		.height = 60,
	}
};

static int ft5216_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	ts->name = "FT5216";
	ts->xmin = FT5216_XAXIS_MIN;
	ts->xmax = FT5216_XAXIS_MAX;
	ts->ymin = FT5216_YAXIS_MIN;
	ts->ymax = FT5216_YAXIS_MAX;
	ts->point_count = FT5216_POINT_COUNT;

	ts->key_count = ARRAY_SIZE(ft5216_touch_keys);
	ts->keys = ft5216_touch_keys;

	ts->event_loop = ft5216_event_loop;
	ts->suspend_enable = ft5216_suspend_enable;
	ts->power_enable = ft5216_power_enable;
	ts->readid = ft5216_readid;

	ret = huamobile_ts_register(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_register");
		goto out_kfree_ts;
	}

	pr_green_info("FT5216 Probe Complete");
	return 0;

out_kfree_ts:
	kfree(ts);
	return ret;
}

static int ft5216_ts_remove(struct i2c_client *client)
{
	struct huamobile_ts_device *ts = i2c_get_clientdata(client);

	huamobile_ts_unregister(ts);
	kfree(ts);

	return 0;
}

static const struct i2c_device_id ft5216_ts_id_table[] =
{
	{FT5216_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver ft5216_ts_driver =
{
	.probe = ft5216_ts_probe,
	.remove = ft5216_ts_remove,

	.id_table = ft5216_ts_id_table,
	.driver =
	{
		.name = FT5216_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static int __init ft5216_ts_init(void)
{
	return i2c_add_driver(&ft5216_ts_driver);
}

static void __exit ft5216_ts_exit(void)
{
	i2c_del_driver(&ft5216_ts_driver);
}

module_init(ft5216_ts_init);
module_exit(ft5216_ts_exit);

MODULE_AUTHOR("<Fuang.Cao cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile FocalTech FT5216 TouchScreen Driver");
MODULE_LICENSE("GPL");
