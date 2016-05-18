package com.cavan.cavanutils;

public class SystemProperties {
	public static String TARGET_CLASS_NAME = "android.os.SystemProperties";

	public static Object invokeMethodTyped(String name, Class<?>[] types, Object[] values) {
		return CavanUtils.invokeStaticMethodTyped(TARGET_CLASS_NAME, name, types, values);
	}

	public static Object invokeMethod(String name, Object... parameters) {
		return CavanUtils.invokeStaticMethod(TARGET_CLASS_NAME, name, parameters);
	}

	public static String get(String key) {
		return (String) invokeMethod("get", key);
	}

	public static String get(String key, String def) {
		return (String) invokeMethod("get", key, def);
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
		try {
			String value = get(key);
			if (value != null) {
				return Boolean.parseBoolean(value);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return def;
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
}
