package com.cavan.android;

import com.cavan.java.CavanJava;

import android.content.Context;
import android.util.Log;
import android.widget.Toast;

public class CavanAndroid extends CavanJava {
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
		logE(buildPosMessage());
	}

	public static void logP(String message) {
		logE(buildPosMessage(message));
	}

	public static void dumpstack(Throwable throwable) {
		logW(throwable);
	}

	public static void dumpstack() {
		logW(new Throwable());
	}

	public static void printSep() {
		logD(buildPrintSep());
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
