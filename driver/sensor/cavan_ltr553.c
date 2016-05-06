#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <cavan/cavan_io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define LTR553_SUPPORT_IRQ			0
#define LTR553_DEVICE_NAME			"ltr553"

enum ltr553_register_map {
	REG_ALS_CTRL = 0x80,
	REG_PS_CTRL,
	REG_PS_LED,
	REG_PS_N_PULSES,
	REG_PS_MEAS_RATE,
	REG_ALS_MEAS_RATE,
	REG_PART_ID,
	REG_MANUFAC_ID,
	REG_ALS_DATA_CH1_0,
	REG_ALS_DATA_CH1_1,
	REG_ALS_DATA_CH0_0,
	REG_ALS_DATA_CH0_1,
	REG_ALS_PS_STATUS,
	REG_PS_DATA_0,
	REG_PS_DATA_1,
	REG_INTERRPUT,
	REG_PS_THRES_UP_0,
	REG_PS_THRES_UP_1,
	REG_PS_THRES_LOW_0,
	REG_PS_THRES_LOW_1,
	REG_PS_OFFSET_1,
	REG_PS_OFFSET_0,
	REG_ALS_THRES_UP_0,
	REG_ALS_THRES_UP_1,
	REG_ALS_THRES_LOW_0,
	REG_ALS_THRES_LOW_1,
	REG_INTERRUPT_PERSIST = 0x9E,
};

struct cavan_ltr553_device {
	struct cavan_sensor_device proxi;
	struct cavan_sensor_device light;
};

static struct cavan_sensor_rate_table_node ltr553_proximity_rate_table[] = {
	{ 0x0F, 10000 },
	{ 0x00, 50000 },
	{ 0x01, 70000 },
	{ 0x02, 100000 },
	{ 0x03, 200000 },
	{ 0x04, 500000 },
	{ 0x05, 1000000 },
	{ 0x06, 2000000 },
};

static struct cavan_sensor_rate_table_node ltr553_light_integration_rate_table[] = {
	{ 0x01, 50000 },
	{ 0x00, 100000 },
	{ 0x04, 150000 },
	{ 0x02, 200000 },
	{ 0x03, 400000 },
	{ 0x05, 250000 },
	{ 0x06, 300000 },
	{ 0x07, 350000 },
};

static struct cavan_sensor_rate_table_node ltr553_light_rate_table[] = {
	{ 0x00, 50000 },
	{ 0x01, 100000 },
	{ 0x02, 200000 },
	{ 0x03, 500000 },
	{ 0x04, 1000000 },
	{ 0x05, 2000000 },
};

static int ltr553_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 part_id;
	u8 manufac_id;

	ret = chip->read_register(chip, REG_PART_ID, &part_id);
	if (ret < 0) {
		pr_red_info("chip->read_data");
		return ret;
	}

	pr_func_info("PART_ID = 0x%02x\n", part_id);

	if (part_id != 0x92) {
		pr_red_info("Invalid Part ID!");
		return -EINVAL;
	}

	ret = chip->read_register(chip, REG_MANUFAC_ID, &manufac_id);
	if (ret < 0) {
		pr_red_info("chip->read_data");
		return ret;
	}

	pr_func_info("MANUFAC_ID = 0x%02x\n", manufac_id);

	if (manufac_id != 0x05) {
		pr_red_info("Invalid Manufacturer ID!");
		return -EINVAL;
	}

	return 0;
}

static int ltr553_proximity_set_enable(struct cavan_input_device *dev, bool enable)
{
	return cavan_io_update_bits8(dev->chip, REG_PS_CTRL, enable ? 0x03 : 0x00, 0x03);
}

static int ltr553_proximity_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	struct cavan_input_chip *chip = dev->chip;
	const struct cavan_sensor_rate_table_node *node = cavan_sensor_find_rate_value(ltr553_proximity_rate_table, ARRAY_SIZE(ltr553_proximity_rate_table), delay * 1000);

	return cavan_io_update_bits8(chip, REG_PS_MEAS_RATE, node->value, 0x0F);
}

static int ltr553_proximity_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	u16 value;

	ret = chip->read_register16(chip, REG_PS_DATA_0, &value);
	if (ret < 0) {
		return ret;
	}

	cavan_sensor_report_value(dev->input, value < 750);

	return 0;
}

static int ltr553_light_set_enable(struct cavan_input_device *dev, bool enable)
{
	return cavan_io_update_bits8(dev->chip, REG_ALS_CTRL, enable ? 0x01 : 0x00, 0x01);
}

static int ltr553_light_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	u8 value;
	struct cavan_input_chip *chip = dev->chip;
	const struct cavan_sensor_rate_table_node *node = cavan_sensor_find_rate_value(ltr553_light_rate_table, ARRAY_SIZE(ltr553_light_rate_table), delay * 1000);
	const struct cavan_sensor_rate_table_node *node_integration = cavan_sensor_find_rate_value(ltr553_light_integration_rate_table, ARRAY_SIZE(ltr553_light_integration_rate_table), node->delay_us);

	value = node_integration->value << 3 | node->value;

	return cavan_io_update_bits8(chip, REG_ALS_MEAS_RATE, value, 0x3F);
}

static int ltr553_light_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	u16 value;

	ret = chip->read_register16(chip, REG_ALS_DATA_CH0_0, &value);
	if (ret < 0) {
		return ret;
	}

	cavan_sensor_report_value(dev->input, value);

	return 0;
}

static int ltr553_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct cavan_input_device *dev;
	struct cavan_sensor_device *sensor;
	struct cavan_ltr553_device *ltr553;

	pr_pos_info();

	ltr553 = kzalloc(sizeof(*ltr553), GFP_KERNEL);
	if (ltr553 == NULL) {
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, ltr553);

	sensor = &ltr553->proxi;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "LTR553 Proximity";
	dev->fuzz = 0;
	dev->flat = 0;

#if LTR553_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = CAVAN_INPUT_DEVICE_TYPE_PROXIMITY;
	dev->min_delay = 10;
	dev->poll_delay = 200;

	sensor->max_range = 1;
	sensor->resolution = 1;

	dev->set_enable = ltr553_proximity_set_enable;
	dev->set_delay = ltr553_proximity_set_delay;
	dev->event_handler = ltr553_proximity_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_kfree_sensor;
	}

	sensor = &ltr553->light;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "LTR553 Light";
	dev->fuzz = 0;
	dev->flat = 0;

#if LTR553_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = CAVAN_INPUT_DEVICE_TYPE_LIGHT;
	dev->min_delay = 50;
	dev->poll_delay = 200;
	sensor->max_range = 64000;
	sensor->resolution = 64000;
	dev->set_enable = ltr553_light_set_enable;
	dev->set_delay = ltr553_light_set_delay;
	dev->event_handler = ltr553_light_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_cavan_input_device_unregister_proxi;
	}

	return 0;

out_cavan_input_device_unregister_proxi:
	cavan_input_device_unregister(chip, &ltr553->proxi.dev);
out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void ltr553_input_chip_remove(struct cavan_input_chip *chip)
{
	struct cavan_ltr553_device *ltr553 = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &ltr553->light.dev);
	cavan_input_device_unregister(chip, &ltr553->proxi.dev);
	kfree(ltr553);
}

static int ltr553_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cavan_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL) {
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	cavan_input_chip_set_bus_data(chip, client);

#if LTR553_SUPPORT_IRQ
	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
#else
	chip->irq = -1;
#endif

	pr_bold_info("chip->irq = %d", chip->irq);

	chip->name = "LTR553";
	chip->flags = CAVAN_INPUT_CHIP_FLAG_POWERON_INIT;
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_PROXIMITY | 1 << CAVAN_INPUT_DEVICE_TYPE_LIGHT;
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c_single;
	chip->readid = ltr553_sensor_chip_readid;

	chip->probe = ltr553_input_chip_probe;
	chip->remove = ltr553_input_chip_remove;

	ret = cavan_input_chip_register(chip, &client->dev);
	if (ret < 0) {
		pr_red_info("cavan_input_chip_register");
		goto out_kfree_chip;
	}

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int ltr553_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id ltr553_id[] = {
	{ LTR553_DEVICE_NAME, 0 },
	{ LTR553_DEVICE_NAME "-left", 0 },
	{ LTR553_DEVICE_NAME "-right", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, ltr553_id);

static struct i2c_driver ltr553_driver = {
	.driver = {
		.name = LTR553_DEVICE_NAME,
		.owner = THIS_MODULE,
	},

	.probe = ltr553_i2c_probe,
	.remove = ltr553_i2c_remove,
	.id_table = ltr553_id,
};

static int __init cavan_ltr553_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&ltr553_driver);
}

static void __exit cavan_ltr553_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&ltr553_driver);
}

module_init(cavan_ltr553_init);
module_exit(cavan_ltr553_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan LTR553 Proximity and Light Sensor Driver");
MODULE_LICENSE("GPL");
