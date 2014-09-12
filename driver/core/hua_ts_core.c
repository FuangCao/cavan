#include <huamobile/hua_ts.h>
#include <linux/suspend.h>

static void hua_ts_suspend(struct hua_ts_device *ts)
{
	hua_input_device_set_enable_lock(&ts->dev, false);
	hua_ts_mt_touch_release(ts->dev.input);
}

static void hua_ts_resume(struct hua_ts_device *ts)
{
	hua_input_device_set_enable_lock(&ts->dev, true);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void hua_ts_early_suspend(struct early_suspend *h)
{
	hua_ts_suspend(container_of(h, struct hua_ts_device, early_suspend));
}

static void hua_ts_later_resume(struct early_suspend *h)
{
	hua_ts_resume(container_of(h, struct hua_ts_device, early_suspend));
}
#elif defined(CONFIG_FB) && defined(CONFIG_HUAMOBILE_USE_FB_NOTIFILER)
static int hua_ts_fb_notifier_call(struct notifier_block *notifier, unsigned long event, void *data)
{
	struct hua_ts_device *ts = container_of(notifier, struct hua_ts_device, fb_notifier);

	pr_bold_info("event = %ld", event);

	switch (event)
	{
	case FB_EVENT_BLANK:
		if (data)
		{
			int *blank = ((struct fb_event *) data)->data;

			if (blank)
			{
				if (*blank == FB_BLANK_UNBLANK)
				{
					hua_ts_resume(ts);
				}
				else if (*blank == FB_BLANK_POWERDOWN)
				{
					hua_ts_suspend(ts);
				}
			}
		}

		break;

	case FB_EVENT_SUSPEND:
		hua_ts_suspend(ts);
		break;

	case FB_EVENT_RESUME:
		hua_ts_resume(ts);
		break;
	}

	return 0;
}
#else
static int hua_ts_pm_notifier_call(struct notifier_block *notifier, unsigned long event, void *data)
{
	struct hua_ts_device *ts = container_of(notifier, struct hua_ts_device, pm_notifier);

	pr_bold_info("event = %ld", event);

	switch (event)
	{
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		hua_ts_suspend(ts);
		break;

	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		hua_ts_resume(ts);
		break;
	}

	return 0;
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
		if (p > buff)
		{
			*p++ = ':';
		}

		p += sprintf(p, "0x%02x:%d:%d:%d:%d:%d", EV_KEY, key->code, key->x, key->y, key->width, key->height);
	}

	return p - buff;
}

static struct hua_input_attribute hua_ts_board_properties_attr =
{
	.attr =
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
		.owner = THIS_MODULE,
#endif
		.mode = S_IRUGO
	},

	.show = hua_ts_board_properties_show,
};

static ssize_t hua_ts_device_attr_xrange_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct hua_misc_device *mdev = dev_get_drvdata(device);
	struct hua_ts_device *ts = (struct hua_ts_device *) hua_misc_device_get_data(mdev);

	return sprintf(buff, "(%d, %d)\n", ts->xmin, ts->xmax);
}

static ssize_t hua_ts_device_attr_yrange_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct hua_misc_device *mdev = dev_get_drvdata(device);
	struct hua_ts_device *ts = (struct hua_ts_device *) hua_misc_device_get_data(mdev);

	return sprintf(buff, "(%d, %d)\n", ts->ymin, ts->ymax);
}

static ssize_t hua_ts_device_attr_point_count_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct hua_misc_device *mdev = dev_get_drvdata(device);
	struct hua_ts_device *ts = (struct hua_ts_device *) hua_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", ts->point_count);
}

static ssize_t hua_ts_device_attr_keys_show(struct device *device, struct device_attribute *attr, char *buff)
{
	char *buff_bak = buff;
	const struct hua_ts_touch_key *key, *key_end;
	struct hua_misc_device *mdev = dev_get_drvdata(device);
	struct hua_ts_device *ts = (struct hua_ts_device *) hua_misc_device_get_data(mdev);

	if (ts->keys == NULL || ts->key_count == 0)
	{
		return 0;
	}

	for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
	{
		buff += sprintf(buff, "x = %d, y = %d, width = %d, height = %d, code = %d\n", key->x, key->y, key->width, key->height, key->code);
	}

	return buff - buff_bak;
}

static struct device_attribute hua_ts_device_attr_xrange = __ATTR(xrange, S_IRUGO, hua_ts_device_attr_xrange_show, NULL);
static struct device_attribute hua_ts_device_attr_yrange = __ATTR(yrange, S_IRUGO, hua_ts_device_attr_yrange_show, NULL);
static struct device_attribute hua_ts_device_attr_point_count = __ATTR(point_count, S_IRUGO, hua_ts_device_attr_point_count_show, NULL);
static struct device_attribute hua_ts_device_attr_keys = __ATTR(keys, S_IRUGO, hua_ts_device_attr_keys_show, NULL);

static const struct attribute *hua_ts_device_attributes[] =
{
	&hua_ts_device_attr_xrange.attr,
	&hua_ts_device_attr_yrange.attr,
	&hua_ts_device_attr_point_count.attr,
	&hua_ts_device_attr_keys.attr,
	NULL
};

static void hua_ts_device_remove(struct hua_input_device *dev)
{
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_core *core = chip->core;

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#elif defined(CONFIG_FB) && defined(CONFIG_HUAMOBILE_USE_FB_NOTIFILER)
	fb_unregister_client(&ts->fb_notifier);
#else
	unregister_pm_notifier(&ts->pm_notifier);
#endif

	sysfs_remove_files(&dev->misc_dev.dev->kobj, hua_ts_device_attributes);
	hua_input_remove_sysfs_files(&core->prop_kobj, &hua_ts_board_properties_attr, 1);
	kfree(hua_ts_board_properties_attr.attr.name);
	hua_input_remove_kobject(&core->prop_kobj);
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
	u32 range[2];
	const char *name;
	struct hua_ts_device *ts = (struct hua_ts_device *) dev;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_core *core = chip->core;
	struct input_dev *input = dev->input;
	struct hua_ts_touch_key *key, *key_end;
#ifdef CONFIG_OF
	struct device_node *of_node = chip->dev->of_node;
#endif

	ret = hua_input_add_kobject(&core->prop_kobj, "board_properties");
	if (ret < 0 && ret != -EEXIST)
	{
		pr_red_info("hua_input_add_kobject");
		return ret;
	}

	name = kasprintf(GFP_KERNEL, "virtualkeys.%s", input->name);
	if (name == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("kasprintf");
		goto out_hua_input_remove_kobject;
	}

	hua_ts_board_properties_attr.attr.name = name;

	ret = hua_input_create_sysfs_files(dev, &core->prop_kobj, &hua_ts_board_properties_attr, 1);
	if (ret < 0)
	{
		pr_red_info("hua_input_add_kobject");
		goto out_kfree_name;
	}

	ret = sysfs_create_files(&dev->misc_dev.dev->kobj, hua_ts_device_attributes);
	if (ret < 0)
	{
		pr_red_info("sysfs_create_files");
		goto out_hua_input_remove_sysfs_files;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = hua_ts_suspend;
	ts->early_suspend.resume = hua_ts_resume;
	register_early_suspend(&ts->early_suspend);
#elif defined(CONFIG_FB) && defined(CONFIG_HUAMOBILE_USE_FB_NOTIFILER)
	ts->fb_notifier.notifier_call = hua_ts_fb_notifier_call;
	ret = fb_register_client(&ts->fb_notifier);
	if (ret < 0)
	{
		pr_red_info("fb_register_client");
	}
#else
	ts->pm_notifier.notifier_call = hua_ts_pm_notifier_call;
	ret = register_pm_notifier(&ts->pm_notifier);
	if (ret < 0)
	{
		pr_red_info("register_pm_notifier");
	}
#endif

	set_bit(INPUT_PROP_DIRECT, input->propbit);

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_TOUCH, input->keybit);

	if (ts->keys && ts->key_count)
	{
#ifdef CONFIG_OF
		struct property *prop;

		prop = of_find_property(of_node, "key-code", NULL);
		if (prop)
		{
			const __be32 *value = prop->value;
			size_t count = prop->length / sizeof(*value);

			if (count > ts->key_count)
			{
				count = ts->key_count;
			}

			while (count > 0)
			{
				count--;
				ts->keys[count].code = be32_to_cpup(value + count);
			}
		}
#endif

		for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
		{
			set_bit(key->code, input->keybit);
		}
	}

#ifdef CONFIG_OF
	if (of_property_read_u32_array(of_node, "xrange", range, ARRAY_SIZE(range)) >= 0)
	{
		ts->xmin = range[0];
		ts->xmax = range[1];
	}

	if (of_property_read_u32_array(of_node, "yrange", range, ARRAY_SIZE(range)) >= 0)
	{
		ts->ymin = range[0];
		ts->ymax = range[1];
	}
#endif

	set_bit(EV_ABS, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, ts->xmin, ts->xmax, dev->fuzz, dev->flat);
	input_set_abs_params(input, ABS_MT_POSITION_Y, ts->ymin, ts->ymax, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, ts->point_count - 1, 0, 0);

	input->open = hua_ts_device_open;
	dev->remove = hua_ts_device_remove;

	ts->touch_count = 0;

	pr_green_info("huamobile touch screen %s probe complete", dev->name);

	return 0;

out_hua_input_remove_sysfs_files:
	hua_input_remove_sysfs_files(&core->prop_kobj, &hua_ts_board_properties_attr, 1);
out_kfree_name:
	kfree(name);
out_hua_input_remove_kobject:
	hua_input_remove_kobject(&core->prop_kobj);
	return ret;
}

EXPORT_SYMBOL_GPL(hua_ts_device_probe);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile TouchScreen Subsystem");
MODULE_LICENSE("GPL");
