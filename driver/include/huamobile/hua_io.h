#pragma once

#include <huamobile/hua_input.h>

int hua_io_write_register_masked(struct hua_input_chip *chip, u8 addr, u8 value, u8 mask);
int hua_io_write_register16_masked(struct hua_input_chip *chip, u8 addr, u16 value, u16 mask);
