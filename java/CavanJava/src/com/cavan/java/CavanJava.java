package com.cavan.java;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

import javax.crypto.Cipher;

public class CavanJava {

	public static CavanLogger sLogger = new CavanLogger();
	public static CavanAesCipher sAesCipher = new CavanAesCipher();

	public interface Closure {
		Object call(Object... args);
	}

	public interface ClosureVoid {
		void call(Object... args);
	}

	public static void setLogger(CavanLogger logger) {
		sLogger = logger;
	}

	public static void eLog(String message) {
		sLogger.eLog(message);
	}

	public static void eLog(Throwable throwable) {
		sLogger.eLog(throwable);
	}

	public static void eLog(Throwable throwable, String message) {
		sLogger.eLog(throwable, message);
	}

	public static void wLog(String message) {
		sLogger.wLog(message);
	}

	public static void wLog(Throwable throwable) {
		sLogger.wLog(throwable);
	}

	public static void wLog(Throwable throwable, String message) {
		sLogger.wLog(throwable, message);
	}

	public static void dLog(String message) {
		sLogger.dLog(message);
	}

	public static void efLog(String format, Object... args) {
		sLogger.efLog(format, args);
	}

	public static void dfLog(String format, Object... args) {
		sLogger.dfLog(format, args);
	}

	public static void pLog() {
		sLogger.pLog(5);
	}

	public static void pfLog(String format, Object... args) {
		sLogger.pfLog(5, format, args);
	}

	public static void sepLog(int length) {
		sLogger.sepLog(length);
	}

	public static void sepLog() {
		sLogger.sepLog();
	}

	public static void dumpstack(Throwable throwable) {
		sLogger.dumpstack(throwable);
	}

	public static void dumpstack() {
		sLogger.dumpstack();
	}

	public static String buildPosMessage(int index) {
		StackTraceElement trace = getCurrentStackTrace(index);
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

	public static String buildPosMessage() {
		return buildPosMessage(5);
	}

	public static String buildPosMessage(int index, String format, Object... args) {
		StackTraceElement trace = getCurrentStackTrace(index);
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

	public static String buildPosMessage(String format, Object... args) {
		return buildPosMessage(5, format, args);
	}

	public static String buildThrowableMessage(Throwable throwable, String message) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);

		if (message != null) {
			pw.println(message);
		}

		if (throwable != null) {
			throwable.printStackTrace(pw);
		}

		pw.flush();

		return sw.toString();
	}

	public static String buildSepMessage(int length) {
		if (length <= 0) {
			length = getEnvColumns();
			if (length <= 0) {
				length = 30;
			}
		}

		char[] chars = new char[length];
		for (int i = 0; i < length; i++) {
			chars[i] = '=';
		}

		return new String(chars);
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
		} catch (Exception e) {
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

	public static Method getMethod(Class<?> cls, String name, Class<?>... parameters) throws NoSuchMethodException {
		try {
			return cls.getDeclaredMethod(name, parameters);
		} catch (Exception e) {
			return cls.getMethod(name, parameters);
		}
	}

	public static Object invokeMethodTyped(Class<?> cls, Object object, String name, Class<?>[] types, Object... parameters) {
		try {
			Method method = getMethod(cls, name, types);
			return method.invoke(object, parameters);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static Object invokeMethodTyped(String className, Object object, String name, Class<?>[] types, Object... parameters) {
		try {
			Class<?> cls = Class.forName(className);
			return invokeMethodTyped(cls, object, name, types, parameters);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static Object invokeMethodTyped(Object object, String name, Class<?>[] types, Object... parameters) {
		return invokeMethodTyped(object.getClass(), object, name, types, parameters);
	}

	public static Object invokeStaticMethodTyped(Class<?> cls, String name, Class<?>[] types, Object... parameters) {
		return invokeMethodTyped(cls, null, name, types, parameters);
	}

	public static Object invokeStaticMethodTyped(String className, String name, Class<?>[] types, Object... parameters) {
		return invokeMethodTyped(className, null, name, types, parameters);
	}

	public static Class<?>[] buildTypeArray(Object[] values) {
		Class<?>[] types = new Class<?>[values.length];

		for (int i = 0; i < values.length; i++) {
			if (values[i] != null) {
				types[i] = values[i].getClass();
			} else if (i > 0) {
				types[i] = types[i - 1];
			} else {
				types[i] = Object.class;
			}
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

	public static short buildValue16(byte[] bytes, int offset) {
		return (short) ((bytes[offset] & 0xFF) | (bytes[offset + 1] & 0xFF) << 8);
	}

	public static int buildValueU16(byte[] bytes, int offset) {
		return buildValue16(bytes, offset) & 0xFFFF;
	}

	public static int buildValue32(byte[] bytes, int offset) {
		return (buildValue16(bytes, offset) & 0xFFFF) | (buildValue16(bytes, offset + 2) & 0xFFFF) << 16;
	}

	public static long buildValueU32(byte[] bytes, int offset) {
		return buildValue32(bytes, offset) & 0xFFFFFFFF;
	}

	public static short buildValueBe16(byte[] bytes, int offset) {
		return (short) ((bytes[offset + 1] & 0xFF) | (bytes[offset] & 0xFF) << 8);
	}

	public static int buildValueBeU16(byte[] bytes, int offset) {
		return buildValueBe16(bytes, offset) & 0xFFFF;
	}

	public static int buildValueBe32(byte[] bytes, int offset) {
		return (buildValue16(bytes, offset + 2) & 0xFFFF) | (buildValue16(bytes, offset) & 0xFFFF) << 16;
	}

	public static long buildValueBeU32(byte[] bytes, int offset) {
		return buildValueBe32(bytes, offset) & 0xFFFFFFFF;
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
		} catch (Exception e) {
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

	public static boolean notDigit(char c) {
		return c < '0' || c > '9';
	}

	public static boolean isDigit(char[] array) {
		for (char c : array) {
			if (notDigit(c)) {
				return false;
			}
		}

		return true;
	}

	public static boolean isDigit(CharSequence text) {
		for (int i = text.length() - 1; i >= 0; i--) {
			if (notDigit(text.charAt(i))) {
				return false;
			}
		}

		return true;
	}

	public static void msleep(long millis) {
		try {
			Thread.sleep(millis);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static void msleep(long millis, int nanos) {
		try {
			Thread.sleep(millis, nanos);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static void usleep(long usec) {
		long millis = usec / 1000;
		msleep(millis, (int) (usec * 1000 - millis * 1000000));
	}

	public static void nsleep(long nanos) {
		long millis = nanos / 1000000;
		msleep(millis, (int) (nanos - millis * 1000000));
	}

	public static byte[] AesCrypt(byte[] bytes, int opmode) {
		return sAesCipher.crypt(bytes, opmode);
	}

	public static byte[] AesEncrypt(byte[] bytes) {
		return AesCrypt(bytes, Cipher.ENCRYPT_MODE);
	}

	public static byte[] AesEncrypt(String text) {
		return AesEncrypt(text.getBytes());
	}

	public static byte[] AesDecrypt(byte[] bytes) {
		return AesCrypt(bytes, Cipher.DECRYPT_MODE);
	}

	public static int parseInt(String text) {
		try {
			return Integer.parseInt(text);
		} catch (Exception e) {
			return 0;
		}
	}

	public static short parseShort(String text) {
		try {
			return Short.parseShort(text);
		} catch (Exception e) {
			return 0;
		}
	}

	public static long parseLong(String text) {
		try {
			return Long.parseLong(text);
		} catch (Exception e) {
			return 0;
		}
	}

	public static float parseFloat(String text) {
		try {
			return Float.parseFloat(text);
		} catch (Exception e) {
			return 0;
		}
	}

	public static double parseDouble(String text) {
		try {
			return Double.parseDouble(text);
		} catch (Exception e) {
			return 0;
		}
	}

	public static int getShortBit(short value, int index, int base) {
		while (index > 0) {
			value /= base;
			index--;
		}

		return value % base;
	}

	public static int getIntBit(int value, int index, int base) {
		while (index > 0) {
			value /= base;
			index--;
		}

		return value % base;
	}

	public static int getLongBit(long value, int index, int base) {
		while (index > 0) {
			value /= base;
			index--;
		}

		return (int) (value % base);
	}

	public static byte[] getShortBits(short value, int base) {
		byte[] bytes = new byte[16];

		for (int i = 0; i < bytes.length; i++) {
			bytes[i] = (byte) (value % base);
			value /= base;
		}

		return bytes;
	}

	public static byte[] getIntBits(int value, int base) {
		byte[] bytes = new byte[32];

		for (int i = 0; i < bytes.length; i++) {
			bytes[i] = (byte) (value % base);
			value /= base;
		}

		return bytes;
	}

	public static byte[] getLongBits(long value, int base) {
		byte[] bytes = new byte[64];

		for (int i = 0; i < bytes.length; i++) {
			bytes[i] = (byte) (value % base);
			value /= base;
		}

		return bytes;
	}
}
