/*
 * Copyright (C) 2015 JWAOO, Inc.
 * drivers/gpio/gpio-gpio_keypad.c
 * author: cavan.cfa@gmail.com
 * create date: 2015-05-14
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/input.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <dt-bindings/gpio/gpio.h>

#define GPIO_KEYPAD_DEBUG					0
#define GPIO_KEYPAD_MAX_KEYS				32
#define GPIO_KEYPAD_KEYPAD_DEBOUNCE			(HZ * 60 / 1000)

#define gpio_keypad_pr_info(fmt, args ...) \
	 pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define gpio_keypad_pr_pos_info() \
	 pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

struct gpio_keypad_device;

struct gpio_key {
	int irq;
	int code;
	int gpio;
	int state;
	u32 mask;
	int active_value;
	const char *name;
	struct gpio_keypad_device *keypad;
};

struct gpio_keypad_device {
	struct platform_device *pdev;
	struct input_dev *input;
	int key_count;
	u32 pending_mask;
	struct gpio_key *keys[GPIO_KEYPAD_MAX_KEYS];
	struct delayed_work work;
	struct workqueue_struct *wq;
};

// ============================================================

static void gpio_keypad_work(struct work_struct *work)
{
	struct gpio_keypad_device *keypad = container_of(work, struct gpio_keypad_device, work.work);
	struct input_dev *input = keypad->input;
	u32 mask = keypad->pending_mask;
	int i;

	dev_info(&keypad->pdev->dev, "mask = 0x%08x", mask);

	for (i = 0; i < keypad->key_count; i++) {
		int state;
		struct gpio_key *key = keypad->keys[i];

		if ((mask & key->mask) == 0) {
			continue;
		}

		state = gpio_get_value_cansleep(key->gpio);
		dev_info(&keypad->pdev->dev, "key = %s, state = %d", key->name, state);
		if (state == key->state) {
			continue;
		}

		key->state = state;
		input_report_key(input, key->code, state == key->active_value);
	}

	input_sync(input);

	keypad->pending_mask &= ~mask;
	if (keypad->pending_mask) {
		queue_delayed_work(keypad->wq, &keypad->work, GPIO_KEYPAD_KEYPAD_DEBOUNCE);
	}
}


static irqreturn_t gpio_keypad_irq_handler(int irq, void *dev_id)
{
	struct gpio_key *key = dev_id;
	struct gpio_keypad_device *keypad = key->keypad;

	keypad->pending_mask |= key->mask;
	mod_delayed_work(keypad->wq, &keypad->work, GPIO_KEYPAD_KEYPAD_DEBOUNCE);

	return IRQ_HANDLED;
}

static void gpio_key_deinit(struct gpio_key *key)
{
	free_irq(key->irq, key);
	gpio_free(key->gpio);
	kfree(key);
}

#ifdef CONFIG_OF
static struct gpio_key *gpio_key_init(struct gpio_keypad_device *keypad, struct device_node *key_node)
{
	int ret;
	int irq;
	int gpio;
	u32 code;
	const char *name;
	struct gpio_key *key;
	enum of_gpio_flags flags;

	gpio = of_get_gpio_flags(key_node, 0, &flags);
	if (!gpio_is_valid(gpio)) {
		return NULL;
	}

	if (of_property_read_u32(key_node, "code", &code) < 0) {
		return NULL;
	}

	name = of_get_property(key_node, "name", NULL);
	if (name == NULL) {
		name = "gpio-key";
	}

	gpio_keypad_pr_pos_info();

	ret = gpio_request(gpio, name);
	if (ret < 0) {
		dev_err(&keypad->pdev->dev, "Failed to request gpio %d: %d\n", gpio, ret);
		return NULL;
	}

	irq = gpio_to_irq(gpio);
	if (irq < 0) {
		dev_err(&keypad->pdev->dev, "Failed to gpio_to_irq %d: %d\n", gpio, irq);
		return NULL;
	}

	key = kzalloc(sizeof(struct gpio_key), GFP_KERNEL);
	if (key == NULL) {
		dev_err(&keypad->pdev->dev, "Failed to kzalloc key\n");
		return NULL;
	}

	key->irq = irq;
	key->gpio = gpio;
	key->code = code;
	key->name = name;
	key->keypad = keypad;
	key->active_value = !(flags & GPIO_ACTIVE_LOW);

	ret = request_irq(irq, gpio_keypad_irq_handler, IRQ_TYPE_EDGE_BOTH, name, key);
	if (ret < 0) {
		dev_err(&keypad->pdev->dev, "Failed to request_irq: %d\n", ret);
		goto out_kfree_key;
	}

	return key;

out_kfree_key:
	kfree(key);
	return NULL;
}

static int gpio_keypad_platform_probe(struct platform_device *pdev)
{
	int ret;
	int count;
	struct input_dev *input;
	struct device_node *key_node;
	struct gpio_keypad_device *keypad;

	gpio_keypad_pr_pos_info();

	keypad = kzalloc(sizeof(struct gpio_keypad_device), GFP_KERNEL);
	if (keypad == NULL) {
		dev_err(&pdev->dev, "Failed to kzalloc\n");
		return -ENOMEM;
	}

	keypad->pdev = pdev;
	platform_set_drvdata(pdev, keypad);

	input = input_allocate_device();
	if (input == NULL) {
		dev_err(&pdev->dev, "Failed to input_allocate_device");
		ret = -ENOMEM;
		goto out_kfree_keypad;
	}

	keypad->input = input;

	input->name = "gpio-keypad";

	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to input_register_device: %d\n", ret);
		goto out_input_free_device;
	}

	keypad->wq = create_singlethread_workqueue("gpio-keypad-wq");
	if (keypad->wq == NULL) {
		dev_err(&pdev->dev, "Failed to create_singlethread_workqueue\n");
		ret = -ENOMEM;
		goto out_input_unregister_device;
	}

	INIT_DELAYED_WORK(&keypad->work, gpio_keypad_work);

	count = 0;

	for_each_child_of_node(pdev->dev.of_node, key_node) {
		struct gpio_key *key = gpio_key_init(keypad, key_node);

		if (key == NULL) {
			continue;
		}

		input_set_capability(input, EV_KEY, key->code);

		key->mask = 1 << count;
		keypad->keys[count] = key;

		dev_info(&pdev->dev, "%d. name = %s, gpio = %d, code = %d", count, key->name, key->gpio, key->code);

		count++;
	}

	if (count == 0) {
		ret = -ENOENT;
		dev_info(&pdev->dev, "no key found\n");
		goto out_destroy_workqueue;
	}

	keypad->key_count = count;

	return 0;

out_destroy_workqueue:
	destroy_workqueue(keypad->wq);
out_input_unregister_device:
	input_unregister_device(input);
out_input_free_device:
	input_free_device(keypad->input);
out_kfree_keypad:
	kfree(keypad);
	return ret;
}
#else
static int gpio_keypad_platform_probe(struct platform_device *pdev)
{
	dev_err(pdev->dev, "this driver need dts support");

	return -EINVAL;
}
#endif

static int gpio_keypad_platform_remove(struct platform_device *pdev)
{
	int i;
	struct gpio_keypad_device *keypad = platform_get_drvdata(pdev);

	for (i = 0; i < keypad->key_count; i++) {
		gpio_key_deinit(keypad->keys[i]);
	}

	destroy_workqueue(keypad->wq);

	input_unregister_device(keypad->input);
	input_free_device(keypad->input);

	kfree(keypad);

	return 0;
}

static const struct of_device_id gpio_keypad_match[] = {
	{ .compatible = "gpio-keypad", .data = NULL},
	{},
};

MODULE_DEVICE_TABLE(of, rk_key_match);

static struct platform_driver gpio_keypad_driver = {
	.driver = {
		.name = "gpio-keypad",
		.owner = THIS_MODULE,
		.of_match_table = gpio_keypad_match,
	},
	.probe = gpio_keypad_platform_probe,
	.remove   = gpio_keypad_platform_remove,
};

module_platform_driver(gpio_keypad_driver);

MODULE_DESCRIPTION("GPIO_KEYPAD Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
