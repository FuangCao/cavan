package com.cavan.android;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;

public class CavanThreadedHandler extends Handler {

	public static class CavanHandlerThread extends HandlerThread {

		public CavanHandlerThread(String name, int priority) {
			super(name, priority);
			super.start();
		}

		public CavanHandlerThread(String name) {
			this(name, Process.THREAD_PRIORITY_DEFAULT);
		}

		public CavanHandlerThread() {
			this(CavanHandlerThread.class.getCanonicalName());
		}

		@Override
		public synchronized void start() {
			CavanAndroid.dLog("Nothing to be done.");
		}
	}

	public CavanThreadedHandler(Callback callback, String name, int priority) {
		super(new CavanHandlerThread(name, priority).getLooper(), callback);
	}

	public CavanThreadedHandler(String name, int priority) {
		this(null, name, priority);
	}

	public CavanThreadedHandler(Callback callback, String name) {
		this(callback, name, Process.THREAD_PRIORITY_DEFAULT);
	}

	public CavanThreadedHandler(String name) {
		this(null, name);
	}

	public CavanThreadedHandler(Callback callback) {
		this(callback, CavanThreadedHandler.class.getCanonicalName());
	}

	public CavanThreadedHandler() {
		this(null, CavanThreadedHandler.class.getCanonicalName());
	}
}
