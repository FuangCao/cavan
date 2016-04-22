package com.cavan.cavanutils;

import android.util.Log;

public class CavanUtils extends CavanNative {
	public static final String TAG = "Cavan";

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
