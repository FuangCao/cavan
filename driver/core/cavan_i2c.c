#include <cavan/cavan_i2c.h>

ssize_t cavan_input_read_data_i2c(struct cavan_input_chip *chip, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *) &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = chip->i2c_rate,
#endif
		},
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = chip->i2c_rate,
#endif
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(cavan_input_read_data_i2c);

ssize_t cavan_input_write_data_i2c(struct cavan_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *) &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = chip->i2c_rate,
#endif
		},
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = size,
			.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = chip->i2c_rate,
#endif
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(cavan_input_write_data_i2c);

int cavan_input_read_register_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u8 *value)
{
	int ret;
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	ret = i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_READ, addr, I2C_SMBUS_BYTE_DATA, &data);
	if (ret < 0)
	{
		return ret;
	}

	*value = data.byte;

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_read_register_i2c_smbus);

int cavan_input_write_register_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u8 value)
{
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	data.byte = value;

	return i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA, &data);
}

EXPORT_SYMBOL_GPL(cavan_input_write_register_i2c_smbus);

int cavan_input_read_register16_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u16 *value)
{
	int ret;
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	ret = i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_READ, addr, I2C_SMBUS_WORD_DATA, &data);
	if (ret < 0)
	{
		return ret;
	}

	*value = data.word;

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_read_register16_i2c_smbus);

int cavan_input_write_register16_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u16 value)
{
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	data.word = value;

	return i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_WRITE, addr, I2C_SMBUS_WORD_DATA, &data);
}

EXPORT_SYMBOL_GPL(cavan_input_write_register16_i2c_smbus);

int cavan_input_master_recv_from_i2c(struct i2c_client *client, short addr, void *buff, size_t size)
{
	int ret;
	struct i2c_msg msg =
	{
		.addr = addr,
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = size,
		.buf = buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = CAVAN_INPUT_I2C_RATE,
#endif
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret == 1)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(cavan_input_master_recv_from_i2c);

int cavan_input_master_recv_i2c(struct cavan_input_chip *chip, void *buff, size_t size)
{
	struct i2c_client *client = cavan_input_chip_get_bus_data(chip);

	return cavan_input_master_recv_from_i2c(client, client->addr, buff, size);
}

EXPORT_SYMBOL_GPL(cavan_input_master_recv_i2c);

int cavan_input_master_send_to_i2c(struct i2c_client *client, short addr, const void *buff, size_t size)
{
	int ret;
	struct i2c_msg msg =
	{
		.addr = addr,
		.flags = client->flags & I2C_M_TEN,
		.len = size,
		.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = CAVAN_INPUT_I2C_RATE,
#endif
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret == 1)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(cavan_input_master_send_to_i2c);

int cavan_input_master_send_i2c(struct cavan_input_chip *chip, const void *buff, size_t size)
{
	struct i2c_client *client = cavan_input_chip_get_bus_data(chip);

	return cavan_input_master_send_to_i2c(client, client->addr, buff, size);
}

EXPORT_SYMBOL_GPL(cavan_input_master_send_i2c);

int cavan_input_test_i2c(struct i2c_client *client)
{
	struct i2c_msg msg =
	{
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = 0,
		.buf = NULL,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = CAVAN_INPUT_I2C_RATE,
#endif
	};

	if (i2c_transfer(client->adapter, &msg, 1) == 1)
	{
		return 0;
	}

	return -EFAULT;
}

EXPORT_SYMBOL_GPL(cavan_input_test_i2c);

int cavan_input_detect_i2c(struct i2c_client *client, u8 start, u8 end)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg =
	{
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = 0,
		.buf = NULL,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = CAVAN_INPUT_I2C_RATE,
#endif
	};

	if (start == 0)
	{
		start = 1;
	}

	if (end == 0)
	{
		end = 0x7F;
	}

	while (start <= end)
	{
		msg.addr = start;

		if (i2c_transfer(adapter, &msg, 1) == 1)
		{
			return start;
		}

		start++;
	}

	return -ENOENT;
}

EXPORT_SYMBOL_GPL(cavan_input_detect_i2c);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan I2C");
MODULE_LICENSE("GPL");
