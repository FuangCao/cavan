package com.cavan.cavanutils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.util.Log;

public class CavanUtils extends CavanNative {
	public static final String TAG = "Cavan";

	public static native boolean kill(String name);
	public static native boolean setEnv(String key, String value);

	public static String getEnv(String name) {
		return System.getenv(name);
	}

	public static boolean setPathEnv(String value) {
		return setEnv("PATH", value);
	}

	public static boolean appendPathEnv(String value) {
		String path = getEnv("PATH");
		if (path == null) {
			path = value;
		} else {
			path += ":" + value;
		}

		return setPathEnv(path);
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

	public static final int logE(String message) {
		return Log.e(TAG, message);
	}

	public static final int logE(String message, Throwable throwable) {
		return Log.e(TAG, message, throwable);
	}

	public static final int logW(String message) {
		return Log.w(TAG, message);
	}

	public static final int logW(Throwable throwable) {
		return Log.w(TAG, throwable);
	}

	public static final int logW(String message, Throwable throwable) {
		return Log.w(TAG, message, throwable);
	}

	public static final int logD(String message) {
		return Log.d(TAG, message);
	}

	public static final int logD(String message, Throwable throwable) {
		return Log.d(TAG, message, throwable);
	}

	public static final int dumpstack() {
		return logE("Dump Stack", new Throwable());
	}

	public static int doCommand(ICavanCommand command, String... args) {
		return command.main(args);
	}

	public static int doCommand(String name, String... args) {
		ICavanCommand command = sHashMap.get(name);
		if (command != null) {
			return doCommand(command, args);
		}

		return -1;
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
		return ArrayCopy(src, 0, dest, 0, src.length);
	}
}
