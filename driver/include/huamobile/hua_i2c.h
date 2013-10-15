#pragma once

#include <huamobile/hua_input.h>

ssize_t hua_input_read_data_i2c(struct hua_input_chip *chip, u8 addr, void *buff, size_t size);
ssize_t hua_input_write_data_i2c(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size);
int hua_input_read_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 *value);
int hua_input_write_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 value);
int hua_input_master_recv_i2c(struct hua_input_chip *chip, void *buff, size_t size);
int hua_input_master_send_i2c(struct hua_input_chip *chip, const void *buff, size_t size);
int hua_input_test_i2c(struct i2c_client *client);
int hua_input_detect_i2c(struct i2c_client *client, u8 start, u8 end);
