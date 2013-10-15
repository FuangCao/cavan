#include <huamobile/hua_ts.h>
#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>

#if defined CONFIG_HUAMOBILE_D58 || defined CONFIG_HUAMOBILE_H2 || defined CONFIG_HUAMOBILE_H4
#define TOUCHSCREEN_CHIP_IS_FT5616	1
#define SUPPORT_PROXIMITY_SENSOR	1
#else
#define TOUCHSCREEN_CHIP_IS_FT5616	0
#define SUPPORT_PROXIMITY_SENSOR	0
#endif

#define FT5216_XAXIS_MIN			0
#define FT5216_XAXIS_MAX			(CONFIG_HUAMOBILE_LCD_WIDTH + 2)
#define FT5216_YAXIS_MIN			0
#define FT5216_YAXIS_MAX			CONFIG_HUAMOBILE_LCD_HEIGHT
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

#define AUTO_UPDATA_FRIMWARE 0

#if AUTO_UPDATA_FRIMWARE
static unsigned char frimwar_data[] = {
	#include "D58_FT6306_YESH_app.i"
};
#endif

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

struct ft5216_firmware_block
{
	u16 head;
	u16 addr;
	u16 size;
	char data[FT5216_FIRMWARE_BLOCK_SIZE];
};
#pragma pack()

struct hua_ft5216_driver_data
{
	struct hua_ts_device ts;
#if SUPPORT_PROXIMITY_SENSOR
	struct hua_sensor_device prox;
#endif
};

static inline ssize_t ft5216_read_data_package(struct hua_input_chip *chip, struct ft5216_data_package *package)
{
	return chip->read_data(chip, 0, package, sizeof(*package));
}

static int ft5216_send_command(struct hua_input_chip *chip, u8 *command, size_t wrlen, size_t rdlen)
{
	int ret;

	ret = chip->master_send(chip, command, wrlen);
	if (ret < 0) {
		pr_red_info("master_send");
		return ret;
	}

	if (rdlen > 0) {
		return chip->master_recv(chip, command, rdlen);
	}

	return 0;
}

static int ft5216_send_command_simple(struct hua_input_chip *chip, char command)
{
	return ft5216_send_command(chip, &command, 1, 0);
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

static int ft5216_change_power_mode(struct hua_input_chip *chip, u8 mode, int retry)
{
	pr_bold_info("Ft5216 change power mode => %s", ft5216_power_mode_tostring(mode));

	while (chip->write_register(chip, FT5216_REG_POWER_MODE, mode) < 0 && retry--)
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

static int ft5216_ts_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	int count;
	struct input_dev *input = dev->input;
	struct ft5216_data_package package;
	struct ft5216_touch_point *p, *p_end;

	ret = ft5216_read_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("ft5216_read_data_package");
		hua_input_chip_recovery(chip, false);
		return ret;
	}

	count = package.td_status & 0x07;
	if (count == 0)
	{
		hua_ts_mt_touch_release(input);
		return 0;
	}

	if (unlikely(count > FT5216_POINT_COUNT))
	{
		// pr_red_info("Too much points = %d", count);
		count = FT5216_POINT_COUNT;
	}

	for (p = package.points, p_end = p + count; p < p_end; p++)
	{
		hua_ts_report_mt_data2(input, FT5216_TOUCH_ID(p->yh), \
			FT5216_BUILD_AXIS(p->xh, p->xl), FT5216_BUILD_AXIS(p->yh, p->yl));
	}

	input_sync(input);

	return 0;
}

#if SUPPORT_PROXIMITY_SENSOR
static int ft5216_proximity_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, 0x01, &value);
	if (ret < 0)
	{
		pr_red_info("dev->read_register");
		hua_input_chip_recovery(chip, false);
		return ret;
	}

	switch (value)
	{
	case 0xC0:
		hua_sensor_report_value(dev->input, 0);
		break;

	case 0xE0:
		hua_sensor_report_value(dev->input, 1);
		break;
	}

	return 0;
}

static int ft5216_proximity_set_enable(struct hua_input_device *dev, bool enable)
{
	struct hua_input_chip *chip = dev->chip;

	return chip->write_register(chip, 0xB0, enable ? 1 : 0);
}
#endif

static int ft5216_set_power(struct hua_input_chip *chip, bool enable)
{
	if (enable)
	{
#if TOUCHSCREEN_CHIP_IS_FT5616
		sprd_ts_reset_enable(false);
		msleep(5);
		sprd_ts_power_enable(true);
		msleep(5);
		sprd_ts_reset_enable(true);
		msleep(300);
#else
		sprd_ts_reset_enable(false);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_reset_enable(true);
		msleep(200);
#endif
	}
	else
	{
		sprd_ts_power_enable(false);
	}

	return 0;
}

static int ft5216_set_active(struct hua_input_chip *chip, bool enable)
{
	u8 mode;
	int retry;

	if (enable)
	{
		mode = FT5216_MODE_ACTIVE;
		retry = 10;
	}
	else
	{
		mode = FT5216_MODE_HIBERNATE;
		retry = 2;
	}

	return ft5216_change_power_mode(chip, mode, retry);
}

static int ft5216_readid(struct hua_input_chip *chip)
{
	int ret;
	char buff[94];
	const u8 *p, *p_end;

	pr_pos_info();

	ret = chip->read_data(chip, 0x22, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	hua_input_print_memory(buff, ret);

	for (p = buff, p_end = p + 29; p < p_end; p++)
	{
#if TOUCHSCREEN_CHIP_IS_FT5616
		if (*p != 0)
#else
		if (*p != 0xFF)
#endif
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

static int ft5216_read_firmware_id(struct hua_input_chip *chip)
{
	int ret;
	u8 command = 0xa6;

	ret = ft5216_send_command(chip, &command, 1, 1);
	if (ret < 0) {
		pr_red_info("ft5216_send_command");
		return ret;
	}

	return command;
}

static int ft5216_read_vendor_id(struct hua_input_chip *chip)
{
	int ret;
	u8 command = 0xa8;

	ret = ft5216_send_command(chip, &command, 1, 1);
	if (ret < 0) {
		pr_red_info("ft5216_send_command");
		return ret;
	}

	return command;
}

static int ft5216_firmware_write_block(struct hua_input_chip *chip, struct ft5216_firmware_block *block, u16 addr, u16 size)
{
	block->addr = __cpu_to_be16(addr);
	block->size = __cpu_to_be16(size);

	return chip->master_send(chip, block, size + 6);
}

static int ft5216_upgrade_prepare(struct hua_input_chip *chip)
{
	int ret;
	unsigned char command[2];

	pr_pos_info();

	command[0] = 0xbc;
	command[1] = 0xaa;
	ret = ft5216_send_command(chip, command, 2, 0);
	if (ret < 0) {
		pr_red_info("ft5216_send_command");
		return ret;
	}

	msleep(100);

	command[0] = 0xbc;
	command[1] = 0x55;
	ret = ft5216_send_command(chip, command, 2, 0);
	if (ret < 0) {
		pr_red_info("ft5216_send_command");
		return ret;
	}

    msleep(30);

	return 0;
}

static int ft5216_enter_upgrade_mode(struct hua_input_chip *chip, int retry)
{
	unsigned char command[] = {0x55, 0xaa};

	pr_pos_info();

	while (retry--) {
		int ret = ft5216_send_command(chip, command, sizeof(command), 0);
		if (ret < 0) {
			msleep(5);
		} else {
			msleep(10);
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static int ft5216_check_hardware_id(struct hua_input_chip *chip)
{
	int ret;
	unsigned char command[] = {0x90, 0x00, 0x00, 0x00};

	pr_pos_info();

	ret = ft5216_send_command(chip, command, sizeof(command), 2);
	if (ret < 0) {
		pr_red_info("ft5216_send_command");
		return ret;
	}

	if (command[0] == 0x79 && command[1] == 0x08)
	{
		return 0;
	}

	pr_red_info("Invalid ID = 0x%02x 0x%02x", command[0], command[1]);

	return -EINVAL;
}

static int ft5216_firmware_erase_app(struct hua_input_chip *chip)
{
	pr_pos_info();

	return ft5216_send_command_simple(chip, 0x61);
}

static int ft5216_firmware_read_ecc(struct hua_input_chip *chip, unsigned char *ecc)
{
	*ecc = 0xcc;

	pr_pos_info();

	return ft5216_send_command(chip, ecc, 1, 1);
}

static int ft5216_firmware_reset(struct hua_input_chip *chip)
{
	int ret;

	pr_pos_info();

	ret = ft5216_send_command_simple(chip, 0x07);
	if (ret < 0) {
		pr_red_info("ft5216_send_command_simple");
		return ret;
	}

	ft5216_set_power(chip, false);
	ft5216_set_power(chip, true);

	return 0;
}

static u8 ft5216_firmware_calculate_ecc(u8 ecc, const u8 *buff, size_t size)
{
	const u8 *buff_end = buff + size;

	while (buff < buff_end) {
		ecc ^= *buff++;
	}

	return ecc;
}

static int ft5216_firmware_upgrade(struct hua_input_chip *chip, struct hua_firmware *fw)
{
	int ret;
	ssize_t rdlen;
	u16 addr;
	u8 ecc, ecc_read;
	struct ft5216_firmware_block block;

	pr_pos_info();

	pr_bold_info("firmware old version = 0x%02x\n", ft5216_read_firmware_id(chip));

	ret = ft5216_upgrade_prepare(chip);
	if (ret < 0) {
		pr_red_info("ft5216_upgrade_prepare");
		return ret;
	}

	ret = ft5216_enter_upgrade_mode(chip, 10);
	if (ret < 0) {
		pr_red_info("ft5216_enter_upgrade_mode");
		return ret;
	}

	ret = ft5216_check_hardware_id(chip);
	if (ret < 0) {
		pr_red_info("ft5216_check_hardware_id");
		return ret;
	}

	ret = ft5216_firmware_erase_app(chip);
	if (ret < 0) {
		pr_red_info("ft5216_firmware_erase_app");
		return ret;
	}

	msleep(1500);

	ecc = 0;
	addr = 0;
	block.head = 0xbf;

	pr_green_info("Start write block data");

	while (1)
    {
		rdlen = hua_firmware_fill(fw, block.data, sizeof(block.data), 8, 5000);
		if (rdlen < 0) {
			pr_red_info("hua_firmware_fill");
			return rdlen;
		}

		if (rdlen == 0) {
			break;
		}

		ret = ft5216_firmware_write_block(chip, &block, addr, rdlen);
		if (ret < 0)
		{
			pr_red_info("i2c_master_recv Step 5:1");
			return ret;
		}

		ecc = ft5216_firmware_calculate_ecc(ecc, block.data, rdlen);

		msleep(rdlen / 6 + 1);
		addr += rdlen;
    }

	pr_green_info("Write block data complete");
	pr_green_info("Start write last 6 byte");

	for (addr = 0x6ffa; addr < 0x6ffa + 6; addr++) {
		rdlen = hua_firmware_read(fw, block.data, 1, 0, 5000);
		if (rdlen < 1) {
			pr_red_info("hua_firmware_fill");
			return rdlen < 0 ? rdlen : -EFAULT;
		}

		ret = ft5216_firmware_write_block(chip, &block, addr, 1);
		if (ret < 0) {
			pr_red_info("ft5216_firmware_write_data");
			return ret;
		}

		ecc ^= block.data[0];

		msleep(20);
	}

	pr_green_info("Write last 6 byte complete");

	ret = ft5216_firmware_read_ecc(chip, &ecc_read);
	if (ret < 0) {
		pr_red_info("ft5216_firmware_read_ecc");
		return ret;
	}

	pr_bold_info("ecc = 0x%02x, ecc_read = 0x%02x", ecc, ecc_read);

	if (ecc != ecc_read) {
		pr_red_info("ecc not match");
		return -EINVAL;
	}

	ret = ft5216_firmware_reset(chip);
	if (ret < 0) {
		pr_red_info("ft5216_firmware_reset");
		return ret;
	}

	ft5216_readid(chip);

	pr_bold_info("firmware new version = 0x%02x\n", ft5216_read_firmware_id(chip));

	return 0;
}

static int ft5216_calibration(struct hua_input_chip *chip, const void *buff, size_t size)
{
	int ret;
	u8 value;
	int i;
	struct i2c_client *client = hua_input_chip_get_bus_data(chip);
	char data[2] = {0x1C, 0x01};

	pr_pos_info();

	for (i = 0; i < 5; i++)
	{
		ft5216_set_power(chip, false);
		ft5216_set_power(chip, true);

		msleep(100);

		ret = i2c_master_send(client, data, sizeof(data));
		if (ret < 0)
		{
			pr_red_info("i2c_master_send");
			return ret;
		}

		msleep(1000);

		ret = i2c_master_send(client, data, 1);
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

static struct hua_ts_touch_key ft5216_touch_keys[] =
{
#if CONFIG_HUAMOBILE_LCD_WIDTH == 480
	{
		.code = KEY_MENU,
		.x = 80,
		.y = 900,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_HOME,
		.x = 240,
		.y = 900,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_BACK,
		.x = 400,
		.y = 900,
		.width = 120,
		.height = 60,
	}
#else
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
#endif
};

static ssize_t ft5216_firmware_id_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	int ret;
	u8 vendor, version;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);

	ret = hua_input_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_set_power_lock");
		goto out_mutex_unlock;
	}

	ret = ft5216_read_vendor_id(chip);
	if (ret < 0)
	{
		pr_red_info("read_register");
		goto out_mutex_unlock;
	}

	vendor = ret;

	ret = ft5216_read_firmware_id(chip);
	if (ret < 0)
	{
		pr_red_info("read_register");
		goto out_mutex_unlock;
	}

	version = ret;

	ret = sprintf(buff, "%02x%02x\n", vendor, version);

out_mutex_unlock:
	mutex_unlock(&chip->lock);
	return ret;
}

static DEVICE_ATTR(firmware_id, 0664, ft5216_firmware_id_show, NULL);

static int ft5216_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_ft5216_driver_data *data;
	struct hua_ts_device *ts;
#if SUPPORT_PROXIMITY_SENSOR
	struct hua_sensor_device *prox;
#endif
	struct hua_input_device *dev;

	data = kzalloc(sizeof(struct hua_ft5216_driver_data), GFP_KERNEL);
	if (data == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, data);

	ts = &data->ts;
	ts->xmin = FT5216_XAXIS_MIN;
	ts->xmax = FT5216_XAXIS_MAX;
	ts->ymin = FT5216_YAXIS_MIN;
	ts->ymax = FT5216_YAXIS_MAX;
	ts->point_count = FT5216_POINT_COUNT;
	ts->key_count = ARRAY_SIZE(ft5216_touch_keys);
	ts->keys = ft5216_touch_keys;

	dev = &ts->dev;
	dev->type = HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
	dev->use_irq = true;
	dev->event_handler = ft5216_ts_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_kfree_data;
	}

#if SUPPORT_PROXIMITY_SENSOR
	prox = &data->prox;
	prox->min_delay = 20;
	prox->max_range = 1;
	prox->resolution = 1;
	prox->power_consume = 0;

	dev = &prox->dev;
	dev->type = HUA_INPUT_DEVICE_TYPE_PROXIMITY;
	dev->poll_delay = 200;
	dev->set_enable = ft5216_proximity_set_enable;
	dev->event_handler = ft5216_proximity_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_hua_input_device_unregister_ts;
	}
#endif

	ret = device_create_file(&((struct i2c_client *)chip->bus_data)->dev, &dev_attr_firmware_id);
	if (ret < 0)
	{
		pr_red_info("device_create_file");
		goto out_hua_input_device_unregister_proxy;
	}

	return 0;

out_hua_input_device_unregister_proxy:
#if SUPPORT_PROXIMITY_SENSOR
	hua_input_device_unregister(chip, &prox->dev);
out_hua_input_device_unregister_ts:
#endif
	hua_input_device_unregister(chip, &ts->dev);
out_kfree_data:
	kfree(data);
	return 0;
}

static void ft5216_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_ft5216_driver_data *data = hua_input_chip_get_dev_data(chip);

	device_remove_file(&((struct i2c_client *)chip->bus_data)->dev, &dev_attr_firmware_id);
#if SUPPORT_PROXIMITY_SENSOR
	hua_input_device_unregister(chip, &data->prox.dev);
#endif
	hua_input_device_unregister(chip, &data->ts.dev);
	kfree(data);
}

static int ft5216_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(struct hua_input_chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
	chip->name = "FT5216";
	chip->vendor = "FocalTech";

#if SUPPORT_PROXIMITY_SENSOR
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN | 1 << HUA_INPUT_DEVICE_TYPE_PROXIMITY;
#else
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
#endif

	chip->probe = ft5216_input_chip_probe;
	chip->remove = ft5216_input_chip_remove;
	chip->set_power = ft5216_set_power;
	chip->set_active = ft5216_set_active;
	chip->readid = ft5216_readid;
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->read_register = hua_input_read_register_i2c_smbus;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->firmware_upgrade = ft5216_firmware_upgrade;
	chip->calibration = ft5216_calibration;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_kfree_chip;
	}

	pr_green_info("FT5216 Probe Complete");

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int ft5216_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip= i2c_get_clientdata(client);

	hua_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id ft5216_ts_id_table[] =
{
	{FT5216_DEVICE_NAME, 0}, {}
};

static struct i2c_driver ft5216_ts_driver =
{
	.probe = ft5216_i2c_probe,
	.remove = ft5216_i2c_remove,

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

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile FocalTech FT5216 TouchScreen Driver");
MODULE_LICENSE("GPL");
