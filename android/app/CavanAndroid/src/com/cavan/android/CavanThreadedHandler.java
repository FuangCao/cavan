package com.cavan.android;

import android.os.Handler;
import android.os.Looper;
import android.os.Process;

public class CavanThreadedHandler extends Handler {

	public CavanThreadedHandler(Callback callback, Class<?> cls, int index, int priority) {
		super(CavanHandlerThread.getLooper(cls, index, priority), callback);
	}

	public CavanThreadedHandler(Class<?> cls, int index, int priority) {
		this(null, cls, index, priority);
	}

	public CavanThreadedHandler(Callback callback, Class<?> cls, int index) {
		this(callback, cls, index, Process.THREAD_PRIORITY_DEFAULT);
	}

	public CavanThreadedHandler(Class<?> cls, int index) {
		this(null, cls, index);
	}

	public CavanThreadedHandler(Class<?> cls) {
		this(cls, 0);
	}

	public CavanThreadedHandler(Callback callback, Class<?> cls) {
		this(callback, cls, 0);
	}

	public boolean quit() {
		Looper looper = getLooper();
		if (looper == null) {
			return false;
		}

		looper.quit();

		return true;
	}

	public boolean quitSafely() {
		Looper looper = getLooper();
		if (looper == null) {
			return false;
		}

		looper.quitSafely();

		return true;
	}
}
