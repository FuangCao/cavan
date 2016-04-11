#include <cavan.h>
#include <cavan/android.h>

#ifdef CONFIG_ANDROID_NDK
char *dump_backtrace(char *buff, size_t size)
{
	*buff = 0;
	return buff;
}

char *address_to_symbol(const void *addr, char *buff, size_t size)
{
	return buff;
}

int property_get(const char *key, char *value, const char *default_value)
{
	int length;

	length = __system_property_get(key, value);
	if (length < 0 && default_value) {
		return text_copy(value, default_value) - value;
	}

	return length;
}

int8_t property_get_bool(const char *key, int8_t default_value)
{
	return android_getprop_bool(key, default_value);
}

int64_t property_get_int64(const char *key, int64_t default_value)
{
	return android_getprop_int64(key, default_value);
}

int32_t property_get_int32(const char *key, int32_t default_value)
{
	return android_getprop_int32(key, default_value);
}

int property_set(const char *key, const char *value)
{
	return android_setprop_command(key, value);
}

int property_list(void (*propfn)(const char *key, const char *value, void *cookie), void *cookie)
{
	return 0;
}

#endif
