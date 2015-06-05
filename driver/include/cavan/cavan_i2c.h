#pragma once

#include <cavan/cavan_input.h>

#define CAVAN_INPUT_I2C_RATE			(100 * 1000)

ssize_t cavan_input_read_data_i2c(struct cavan_input_chip *chip, u8 addr, void *buff, size_t size);
ssize_t cavan_input_write_data_i2c(struct cavan_input_chip *chip, u8 addr, const void *buff, size_t size);
int cavan_input_read_register_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u8 *value);
int cavan_input_write_register_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u8 value);
int cavan_input_read_register16_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u16 *value);
int cavan_input_write_register16_i2c_smbus(struct cavan_input_chip *chip, u8 addr, u16 value);
int cavan_input_master_recv_from_i2c(struct i2c_client *client, short addr, void *buff, size_t size);
int cavan_input_master_send_to_i2c(struct i2c_client *client, short addr, const void *buff, size_t size);
int cavan_input_master_recv_i2c(struct cavan_input_chip *chip, void *buff, size_t size);
int cavan_input_master_send_i2c(struct cavan_input_chip *chip, const void *buff, size_t size);
int cavan_input_test_i2c(struct i2c_client *client);
int cavan_input_detect_i2c(struct i2c_client *client, u8 start, u8 end);
