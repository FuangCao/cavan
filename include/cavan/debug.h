#pragma once

#include <cavan.h>

#define RETRY(func, ret, condition, count) \
	do { \
		int i = count; \
		do { \
			ret = func; \
			i--; \
		} while (i && (condition)); \
	} while (0)

// ============================================================

__BEGIN_DECLS

char *dump_backtrace(char *buff, size_t size);
int dump_stack(void);
char *address_to_symbol(const void *addr, char *buff, size_t size);
int catch_sigsegv(void);

int cavan_get_build_time(struct tm *time);
const char *cavan_get_build_time_string(void);

__END_DECLS