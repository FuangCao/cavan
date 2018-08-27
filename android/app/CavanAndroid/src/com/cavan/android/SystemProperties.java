package com.cavan.android;

import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class SystemProperties {

	public static Class<?> SystemPropertiesClass;

	public static Class<?> getSystemPropertiesClass() throws ClassNotFoundException {
		if (SystemPropertiesClass == null) {
			SystemPropertiesClass = Class.forName("android.os.SystemProperties");
		}

		return SystemPropertiesClass;
	}

	public static Object invokeMethodTyped(String name, Class<?>[] types, Object... values) {
		try {
			return CavanJava.invokeStaticMethodTyped(getSystemPropertiesClass(), name, types, values);
		} catch (Exception e) {
			return null;
		}
	}

	public static Object invokeMethod(String name, Object... parameters) {
		try {
			return CavanJava.invokeStaticMethod(getSystemPropertiesClass(), name, parameters);
		} catch (Exception e) {
			return null;
		}
	}

	public static String get(String key) {
		return (String) invokeMethod("get", key);
	}

	public static String get(String key, String def) {
		return (String) invokeMethod("get", key, def);
	}

	public static String getByKeys(String... keys) {
		for (String key : keys) {
			String value = get(key);
			if (value != null && value.length() > 0) {
				return value;
			}
		}

		return null;
	}

	public static int getInt(String key, int def) {
		try {
			String value = get(key);
			if (value != null) {
				return Integer.parseInt(value);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return def;
	}

	public static long getLong(String key, long def) {
		try {
			String value = get(key);
			if (value != null) {
				return Long.parseLong(value);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return def;
	}

	public static boolean getBoolean(String key, boolean def) {
		String value = get(key);
		if (value == null) {
			return def;
		}

		return CavanString.parseBoolean(value);
	}

	public static float getFloat(String key, float def) {
		try {
			String value = get(key);
			if (value != null) {
				return Float.parseFloat(value);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return def;
	}

	public static double getDouble(String key, double def) {
		try {
			String value = get(key);
			if (value != null) {
				return Double.parseDouble(value);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return def;
	}

	public static void set(String key, String value) {
		invokeMethod("set", key, value);
	}

	public static void setInt(String key, int value) {
		set(key, Integer.toString(value));
	}

	public static void setLong(String key, long value) {
		set(key, Long.toString(value));
	}

	public static void setBoolean(String key, boolean value) {
		set(key, Boolean.toString(value));
	}

	public static void setFloat(String key, float value) {
		set(key, Float.toString(value));
	}

	public static void setDouble(String key, double value) {
		set(key, Double.toString(value));
	}

	public static void addChangeCallback(Runnable callback) {
		Class<?> types[] = { Runnable.class };
		Object values[] = { callback };
		invokeMethodTyped("addChangeCallback", types, values);
	}

	public static String getClientIdBase() {
		return get("ro.com.google.clientidbase");
	}
}