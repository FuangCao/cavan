package com.cavan.cavanutils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class CavanJavaUtils {

	public static String getEnv(String name) {
		return System.getenv(name);
	}

	public static int getEnvInt(String name, int defValue) {
		String text = getEnv(name);
		if (text == null) {
			return defValue;
		}

		try {
			return Integer.parseInt(text);
		} catch (NumberFormatException e) {
			e.printStackTrace();
		}

		return defValue;
	}

	public static int getEnvLines() {
		return getEnvInt("LINES", 0);
	}

	public static int getEnvColumns() {
		return getEnvInt("COLUMNS", 0);
	}

	public static Object invokeMethodTyped(Class<?> cls, Object object, String name, Class<?>[] types, Object[] values) {
		try {
			Method method = cls.getMethod(name, types);
			return method.invoke(object, values);
		} catch (NoSuchMethodException e) {
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			e.printStackTrace();
		}

		return null;
	}

	public static Object invokeMethodTyped(String className, Object object, String name, Class<?>[] types, Object[] values) {
		try {
			Class<?> cls = Class.forName(className);
			return invokeMethodTyped(cls, object, name, types, values);
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		}

		return null;
	}

	public static Object invokeStaticMethodTyped(Class<?> cls, String name, Class<?>[] types, Object[] values) {
		return invokeMethodTyped(cls, null, name, types, values);
	}

	public static Object invokeStaticMethodTyped(String className, String name, Class<?>[] types, Object[] values) {
		return invokeMethodTyped(className, null, name, types, values);
	}

	public static Class<?>[] buildTypeArray(Object[] values) {
		Class<?>[] types = new Class<?>[values.length];

		for (int i = types.length - 1; i >= 0; i--) {
			types[i] = values[i].getClass();
		}

		return types;
	}

	public static Object invokeMethod(Class<?> cls, Object object, String name, Object... parameters) {
		Class<?>[] types = buildTypeArray(parameters);
		return invokeMethodTyped(cls, object, name, types, parameters);
	}

	public static Object invokeMethod(Object object, String name, Object... parameters) {
		return invokeMethod(object.getClass(), object, name, parameters);
	}

	public static Object invokeStaticMethod(Class<?> cls, String name, Object... parameters) {
		return invokeMethod(cls, null, name, parameters);
	}

	public static Object invokeStaticMethod(String className, String name, Object... parameters) {
		Class<?>[] types = buildTypeArray(parameters);
		return invokeMethodTyped(className, null, name, types, parameters);
	}

	public static final StackTraceElement[] getStackTrace() {
		return new Throwable().getStackTrace();
	}

	protected static final StackTraceElement getCurrentStackTrace(int index) {
		StackTraceElement[] traces = getStackTrace();
		if (traces != null && traces.length > index) {
			return traces[index];
		}

		return null;
	}

	public static final StackTraceElement getCurrentStackTrace() {
		return getCurrentStackTrace(3);
	}

	public static int ArrayCopy(byte[] src, int srcOff, byte[] dest, int destOff, int count) {
		int srcEnd = srcOff + count;

		while (srcOff < srcEnd) {
			dest[destOff++] = src[srcOff++];
		}

		return destOff;
	}

	public static int ArrayCopy(byte[] src, byte[] dest, int count) {
		return ArrayCopy(src, 0, dest, 0, count);
	}

	public static int ArrayCopy(byte[] src, byte[] dest) {
		int length = Math.min(src.length, dest.length);
		return ArrayCopy(src, dest, length);
	}

	public static byte[] ArrayCopy(byte[] bytes, int start, int count) {
		byte[] newBytes = new byte[count];
		ArrayCopy(bytes, start, newBytes, 0, count);
		return newBytes;
	}

	public static int parseChar(byte c) {
		if (c >= '0' && c <= '9') {
			return c - '0';
		} else if (c >= 'a' && c <= 'z') {
			return c - 'a' + 10;
		} else if (c >= 'A' && c <= 'Z') {
			return c - 'A' + 10;
		} else {
			return 0;
		}
	}

	public static int parseHexText(byte[] bytes, byte[] text, int start, int end) {
		int count;

		for (count = 0; start <= end; start += 2, count++) {
			bytes[count] = (byte) (parseChar(text[start]) << 4 | parseChar(text[start + 1]));
		}

		return count;
	}

	public static byte[] parseHexText(byte[] text, int start, int count) {
		byte[] bytes = new byte[count / 2];
		parseHexText(bytes, text, start, count - 1);
		return bytes;
	}

	public static byte[] parseHexText(byte[] text, int count) {
		return parseHexText(text, 0, count);
	}

	public static byte[] parseHexText(byte[] text) {
		return parseHexText(text, 0, text.length);
	}
}
