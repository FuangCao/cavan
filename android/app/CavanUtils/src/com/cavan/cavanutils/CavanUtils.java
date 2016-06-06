package com.cavan.cavanutils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

public class CavanUtils {
	public static final String TAG = "Cavan";

	static {
		try {
			Looper.prepare();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static final int EVENT_CLEAR_TOAST = 1;

	private static Toast sToast;
	private static final Object sToastLock = new Object();

	private static final Handler HANDLER = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_CLEAR_TOAST:
				cancelToast();
				break;
			}
		}
	};


	public static String getEnv(String name) {
		return System.getenv(name);
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

	public static void cancelToastLocked() {
		if (sToast != null) {
			sToast.cancel();
			sToast = null;
		}

		HANDLER.removeMessages(EVENT_CLEAR_TOAST);
	}

	public static void cancelToast() {
		synchronized (sToastLock) {
			cancelToastLocked();
		}
	}

	public static void showToast(Context context, String text, int duration) {
		Toast toast = Toast.makeText(context, text, duration);
		synchronized (sToastLock) {
			cancelToastLocked();

			sToast = toast;
			toast.show();

			HANDLER.sendEmptyMessageDelayed(EVENT_CLEAR_TOAST, 10000);
		}
	}

	public static void showToast(Context context, String text) {
		showToast(context, text, Toast.LENGTH_SHORT);
	}

	public static void showToastLong(Context context, String text) {
		showToast(context, text, Toast.LENGTH_LONG);
	}

	public static void showToast(Context context, int resId, int duration) {
		String text = context.getResources().getString(resId);
		if (text != null) {
			showToast(context, text, duration);
		}
	}

	public static void showToast(Context context, int resId) {
		showToast(context, resId, Toast.LENGTH_SHORT);
	}

	public static void showToastLong(Context context, int resId) {
		showToast(context, resId, Toast.LENGTH_LONG);
	}
}
