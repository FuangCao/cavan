package com.cavan.cavanutils;

import android.content.Context;
import android.util.Log;
import android.widget.Toast;

public class CavanUtils extends CavanJavaUtils {
	public static final String TAG = "Cavan";

	public static final int EVENT_CLEAR_TOAST = 1;

	private static Toast sToast;
	private static final Object sToastLock = new Object();

	public static void logE(String message) {
		Log.e(TAG, message);
	}

	public static void logE(String message, Throwable throwable) {
		Log.e(TAG, message, throwable);
	}

	public static void logW(String message) {
		Log.w(TAG, message);
	}

	public static void logW(Throwable throwable) {
		Log.w(TAG, throwable);
	}

	public static void logW(String message, Throwable throwable) {
		Log.w(TAG, message, throwable);
	}

	public static void logD(String message) {
		Log.d(TAG, message);
	}

	public static int logD(String message, Throwable throwable) {
		return Log.d(TAG, message, throwable);
	}

	public static void logP() {
		StackTraceElement trace = getCurrentStackTrace(3);
		if (trace == null) {
			return;
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

		logE(builder.toString());
	}

	public static void logP(String message) {
		StackTraceElement trace = getCurrentStackTrace(3);
		if (trace == null) {
			return;
		}

		StringBuilder builder = new StringBuilder();

		builder.append(trace.getMethodName());
		builder.append("[");
		builder.append(trace.getLineNumber());
		builder.append("]: ");
		builder.append(message);

		logE(builder.toString());
	}

	public static void dumpstack(Throwable throwable) {
		logW(throwable);
	}

	public static void dumpstack() {
		logW(new Throwable());
	}

	public static void printSep() {
		String text;
		int columns = getEnvColumns();
		if (columns > 0) {
			char[] chars = new char[columns];
			for (int i = 0; i < columns; i++) {
				chars[i] = '=';
			}

			text = new String(chars);
		} else {
			text = "============================================================";
		}

		logD(text);
	}

	public static void cancelToastLocked() {
		if (sToast != null) {
			sToast.cancel();
			sToast = null;
		}
	}

	public static void cancelToast() {
		synchronized (sToastLock) {
			cancelToastLocked();
		}
	}

	public static void showToast(Context context, String text, int duration) {
		logD(text);

		Toast toast = Toast.makeText(context, text, duration);
		synchronized (sToastLock) {
			cancelToastLocked();

			sToast = toast;
			toast.show();
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
