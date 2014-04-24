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
	struct fb_event *evdata = data;
	struct hua_ts_device *ts = container_of(notifier, struct hua_ts_device, fb_notifier);

	pr_bold_info("event = %ld", event);

	if (evdata && event == FB_EVENT_BLANK)
	{
		int *blank = evdata->data;
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
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_core *core = chip->core;
	struct input_dev *input = dev->input;
	const struct hua_ts_touch_key *key, *key_end;
	const char *name;

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
		for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
		{
			set_bit(key->code, input->keybit);
		}
	}

	set_bit(EV_ABS, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, ts->xmin, ts->xmax, dev->fuzz, dev->flat);
	input_set_abs_params(input, ABS_MT_POSITION_Y, ts->ymin, ts->ymax, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, ts->point_count - 1, 0, 0);

	input->open = hua_ts_device_open;
	dev->remove = hua_ts_device_remove;

	ts->touch_count = 0;

	pr_green_info("huamobile touch screen %s probe complete", dev->name);

	return 0;

out_kfree_name:
	kfree(name);
out_hua_input_remove_kobject:
	hua_input_remove_kobject(&core->prop_kobj);
	return ret;
}

int hua_ts_read_pending_firmware_name(char *buff, size_t size)
{
	int ret;
	struct file *fp;

	fp = filp_open("/data/property/persist.sys.tp.fw.pending", O_RDONLY, 0);
	if (IS_ERR(fp))
	{
		return -1;
	}

	ret = kernel_read(fp, 0, buff, size - 1);
	filp_close(fp, NULL);

	if (ret > 0)
	{
		buff[ret] = 0;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(hua_ts_read_pending_firmware_name);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile TouchScreen Subsystem");
MODULE_LICENSE("GPL");
