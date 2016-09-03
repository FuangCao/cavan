package com.cavan.java;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

public class CavanJava {

	public static void eLog(String message) {
		System.err.println(message);
	}

	public static void eLog(Throwable throwable) {
		throwable.printStackTrace();
	}

	public static void eLog(Throwable throwable, String message) {
		eLog(message);
		eLog(throwable);
	}

	public static void dLog(String message) {
		System.out.println(message);
	}

	public static void efLog(String format, Object... args) {
		eLog(String.format(format, args));
	}

	public static void dfLog(String format, Object... args) {
		dLog(String.format(format, args));
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

	public static void sepLog() {
		dLog(buildPrintSep());
	}

	public static String buildPosMessage() {
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

	public static String buildPosMessage(String format, Object... args) {
		StackTraceElement trace = getCurrentStackTrace(4);
		if (trace == null) {
			return "unknown";
		}

		StringBuilder builder = new StringBuilder();

		builder.append(trace.getMethodName());
		builder.append("[");
		builder.append(trace.getLineNumber());
		builder.append("]: ");
		builder.append(String.format(format, args));

		return builder.toString();
	}

	public static void pLog() {
		eLog(buildPosMessage());
	}

	public static void pfLog(String format, Object... args) {
		eLog(buildPosMessage(format, args));
	}

	public static void dumpstack(Throwable throwable) {
		eLog(throwable);
	}

	public static void dumpstack() {
		eLog(new Throwable());
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

	public static byte[] ArrayCopy(byte[] bytes, int count) {
		return ArrayCopy(bytes, 0, count);
	}

	public static byte[] ArrayCopy(byte[] bytes) {
		return ArrayCopy(bytes, 0, bytes.length);
	}

	public static byte[] ArrayCopySkip(byte[] bytes, int skip) {
		return ArrayCopy(bytes, skip, bytes.length - skip);
	}

	public static int ArrayIndexOf(Object[] objects, Object object) {
		if (objects == null) {
			return -1;
		}

		for (int i = objects.length - 1; i >= 0; i--) {
			if (objects[i].equals(object)) {
				return i;
			}
		}

		return -1;
	}

	public static boolean ArrayContains(Object[] objects, Object object) {
		if (objects == null) {
			return false;
		}

		for (Object o : objects) {
			if (o.equals(object)) {
				return true;
			}
		}

		return false;
	}

	public static short buildValue16(byte[] bytes, int offset) {
		return (short) ((bytes[offset] & 0xFF) | (bytes[offset + 1] & 0xFF) << 8);
	}

	public static int buildValue32(byte[] bytes, int offset) {
		return (buildValue16(bytes, offset) & 0xFFFF) | (buildValue16(bytes, offset + 2) & 0xFFFF) << 16;
	}

	public static short buildValueBe16(byte[] bytes, int offset) {
		return (short) ((bytes[offset + 1] & 0xFF) | (bytes[offset] & 0xFF) << 8);
	}

	public static int buildValueBe32(byte[] bytes, int offset) {
		return (buildValue16(bytes, offset + 2) & 0xFFFF) | (buildValue16(bytes, offset) & 0xFFFF) << 16;
	}

	public static byte[] getValueBytes(byte value) {
		return new byte[] { value };
	}

	public static byte[] getValueBytes(short value) {
		return new byte[] { (byte) (value & 0xFF), (byte) ((value >> 8) & 0xFF) };
	}

	public static byte[] getValueBytes(int value) {
		return new byte[] { (byte) (value & 0xFF), (byte) ((value >> 8) & 0xFF), (byte) ((value >> 16) & 0xFF), (byte) ((value >> 24) & 0xFF) };
	}

	public static byte[] getValueBytesBe(short value) {
		return new byte[] { (byte) ((value >> 8) & 0xFF), (byte) (value & 0xFF) };
	}

	public static byte[] getValueBytesBe(int value) {
		return new byte[] { (byte) ((value >> 24) & 0xFF), (byte) ((value >> 16) & 0xFF), (byte) ((value >> 8) & 0xFF), (byte) (value & 0xFF) };
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

	public static int getBoolValueInt(boolean value) {
		return value ? 1 : 0;
	}

	public static short getBoolValueShort(boolean value) {
		return (short) getBoolValueInt(value);
	}

	public static byte getBoolValueByte(boolean value) {
		return (byte) getBoolValueInt(value);
	}

	public static long getBoolValueLong(boolean value) {
		return getBoolValueInt(value);
	}

	public static List<InetAddress> getIpAddressList() {
		List<InetAddress> addresses = new ArrayList<InetAddress>();

		Enumeration<NetworkInterface> enNetIf;
		try {
			enNetIf = NetworkInterface.getNetworkInterfaces();
			if (enNetIf == null) {
				return addresses;
			}
		} catch (SocketException e) {
			e.printStackTrace();
			return addresses;
		}

		while (enNetIf.hasMoreElements()) {
			Enumeration<InetAddress> enAddr = enNetIf.nextElement().getInetAddresses();
			while (enAddr.hasMoreElements()) {
				InetAddress addr = enAddr.nextElement();
				if (addr.isLoopbackAddress()) {
					continue;
				}

				if (addr.isLinkLocalAddress()) {
					continue;
				}

				addresses.add(addr);
			}
		}

		return addresses;
	}

	public static InetAddress getIpAddress() {
		List<InetAddress> addresses = getIpAddressList();
		if (addresses.size() > 0) {
			return addresses.get(0);
		}

		return null;
	}

	public static boolean isDigit(char c) {
		return c >= '0' && c <= '9';
	}

	public static boolean isNotDigit(char c) {
		return c < '0' || c > '9';
	}
}
