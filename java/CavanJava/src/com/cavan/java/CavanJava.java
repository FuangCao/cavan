package com.cavan.java;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class CavanJava {

	public static void logE(String message) {
		System.err.println(message);
	}

	public static void logE(Throwable throwable) {
		throwable.printStackTrace();
	}

	public static void logE(String message, Throwable throwable) {
		logE(message);
		logE(throwable);
	}

	public static void logD(String message) {
		System.out.println(message);
	}

	protected static String buildPrintSep() {
		int columns = getEnvColumns();
		if (columns > 0) {
			char[] chars = new char[columns];
			for (int i = 0; i < columns; i++) {
				chars[i] = '=';
			}

			return new String(chars);
		} else {
			return "============================================================";
		}
	}

	public static void printSep() {
		logD(buildPrintSep());
	}

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

	protected static String buildLogp() {
		StackTraceElement trace = getCurrentStackTrace(4);
		if (trace == null) {
			return "unknown";
		}

		StringBuilder builder = new StringBuilder();

		builder.append(trace.getClassName());
		builder.append(".");
		builder.append(trace.getMethodName());
		builder.append("(");
		builder.append(trace.getFileName());
		builder.append(":");
		builder.append(trace.getLineNumber());
		builder.append(")");

		return builder.toString();
	}

	protected static String buildLogp(String message) {
		StackTraceElement trace = getCurrentStackTrace(4);
		if (trace == null) {
			return "unknown";
		}

		StringBuilder builder = new StringBuilder();

		builder.append(trace.getMethodName());
		builder.append("[");
		builder.append(trace.getLineNumber());
		builder.append("]: ");
		builder.append(message);

		return builder.toString();
	}

	public static void logP() {
		logE(buildLogp());
	}

	public static void logP(String message) {
		logE(buildLogp(message));
	}

	public static void dumpstack(Throwable throwable) {
		logE(throwable);
	}

	public static void dumpstack() {
		logE(new Throwable());
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

	public static boolean isChineseChar(char c) {
		return c >= 0x4E00 && c <= 0x9FA5;
	}

	public static boolean hasChineseChar(String text) {
		for (int i = text.length() - 1; i >= 0; i--) {
			if (isChineseChar(text.charAt(i))) {
				return true;
			}
		}

		return false;
	}
}
