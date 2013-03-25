#include <linux/input/hua_ts.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
static void hua_ts_suspend(struct early_suspend *h)
{
	struct hua_ts_device *ts = container_of(h, struct hua_ts_device, early_suspend);

	hua_input_device_set_enable_lock(&ts->dev, false);
}

static void hua_ts_resume(struct early_suspend *h)
{
	struct hua_ts_device *ts = container_of(h, struct hua_ts_device, early_suspend);

	hua_input_device_set_enable_lock(&ts->dev, true);
}
#endif

static ssize_t hua_ts_board_properties_show(struct hua_input_device *dev, struct hua_input_attribute *attr, char *buff)
{
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	const struct hua_ts_touch_key *key, *key_end;
	char *p;

	if (ts->keys == NULL || ts->key_count == 0)
	{
		return 0;
	}

	for (p = buff, key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
	{
		p += sprintf(p, "0x%02x:%d:%d:%d:%d:%d:", EV_KEY, key->code, key->x, key->y, key->width, key->height);
	}

	return p - buff;
}

static struct hua_input_attribute hua_ts_board_properties_attr =
{
	.attr =
	{
		.owner = THIS_MODULE,
		.mode = S_IRUGO
	},

	.show = hua_ts_board_properties_show,
};

static void hua_ts_device_remove(struct hua_input_device *dev)
{
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_core *core = chip->core;

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	hua_input_remove_sysfs_files(&core->prop_kobj, &hua_ts_board_properties_attr, 1);
	kfree(hua_ts_board_properties_attr.attr.name);
}

static int hua_ts_device_open(struct input_dev *dev)
{
	int ret;
	struct hua_input_device *idev = input_get_drvdata(dev);

	pr_pos_info();

	ret = hua_input_device_set_enable_lock(idev, true);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_set_enable_lock");
		return ret;
	}

	return 0;
}

int hua_ts_device_probe(struct hua_input_device *dev)
{
	int ret;
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_core *core = chip->core;
	struct input_dev *input = dev->input;
	const struct hua_ts_touch_key *key, *key_end;
	const char *name;

	name = kasprintf(GFP_KERNEL, "virtualkeys.%s", input->name);
	if (name == NULL)
	{
		pr_red_info("kasprintf");
		return -ENOMEM;
	}

	hua_ts_board_properties_attr.attr.name = name;

	ret = hua_input_create_sysfs_files(dev, &core->prop_kobj, &hua_ts_board_properties_attr, 1);
	if (ret < 0)
	{
		pr_red_info("hua_input_add_kobject");
		kfree(name);
		return ret;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = hua_ts_suspend;
	ts->early_suspend.resume = hua_ts_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	if (ts->keys && ts->key_count)
	{
		set_bit(EV_KEY, input->evbit);

		for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
		{
			set_bit(key->code, input->keybit);
		}
	}

	set_bit(EV_ABS, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, ts->xmin, ts->xmax, dev->fuzz, dev->flat);
	input_set_abs_params(input, ABS_MT_POSITION_Y, ts->ymin, ts->ymax, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, ts->pmin, ts->pmax, dev->fuzz, dev->flat);
	input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, ts->pmin, ts->pmax, dev->fuzz, dev->flat);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, ts->point_count - 1, 0, 0);

	input->open = hua_ts_device_open;
	dev->remove = hua_ts_device_remove;

	pr_green_info("huamobile touch screen %s probe complete", dev->name);

	return 0;
}

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile TouchScreen Subsystem");
MODULE_LICENSE("GPL");
