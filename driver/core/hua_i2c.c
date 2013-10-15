#include <huamobile/hua_i2c.h>

ssize_t hua_input_read_data_i2c(struct hua_input_chip *chip, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *)&addr
		},
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *)buff
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_read_data_i2c);

ssize_t hua_input_write_data_i2c(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *)&addr
		},
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = size,
			.buf = (__u8 *)buff
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_write_data_i2c);

int hua_input_read_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 *value)
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

EXPORT_SYMBOL_GPL(hua_input_read_register_i2c_smbus);

int hua_input_write_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 value)
{
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	data.byte = value;

	return i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA, &data);
}

EXPORT_SYMBOL_GPL(hua_input_write_register_i2c_smbus);

int hua_input_master_recv_i2c(struct hua_input_chip *chip, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msg =
	{
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = size,
		.buf = buff
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret == 1)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_master_recv_i2c);

int hua_input_master_send_i2c(struct hua_input_chip *chip, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
	struct i2c_msg msg =
	{
		.addr = client->addr,
		.flags = client->flags & I2C_M_TEN,
		.len = size,
		.buf = (__u8 *)buff
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret == 1)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_master_send_i2c);

int hua_input_test_i2c(struct i2c_client *client)
{
	struct i2c_msg msg =
	{
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = 0,
		.buf = NULL
	};

	if (i2c_transfer(client->adapter, &msg, 1) == 1)
	{
		return 0;
	}

	return -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_test_i2c);

int hua_input_detect_i2c(struct i2c_client *client, u8 start, u8 end)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg =
	{
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = 0,
		.buf = NULL
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

EXPORT_SYMBOL_GPL(hua_input_detect_i2c);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile I2C");
MODULE_LICENSE("GPL");
