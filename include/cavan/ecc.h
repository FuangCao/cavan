#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-23 16:39:57

int cavan_ecc_polarity_row(u8 data);
u8 cavan_ecc_polarity_column(u8 data);
void cavan_ecc_show_ecc_table(const u8 *table, size_t count);
u8 *cavan_ecc_build_ecc_table(u8 *table, size_t count);
u32 cavan_ecc_check(void *buff, size_t size);
