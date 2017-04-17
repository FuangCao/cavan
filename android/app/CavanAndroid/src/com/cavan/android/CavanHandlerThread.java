package com.cavan.android;

import java.util.HashMap;

import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;

public class CavanHandlerThread extends HandlerThread {

	private static HashMap<String, CavanHandlerThread> sThreadMap = new HashMap<String, CavanHandlerThread>();

	public static CavanHandlerThread getInstance(Class<?> cls, int index, int priority) {
		String name = cls.getCanonicalName() + index;

		synchronized (sThreadMap) {
			CavanHandlerThread thread = sThreadMap.get(name);
			if (thread != null && thread.isAlive()) {
				return thread;
			}

			thread = new CavanHandlerThread(name, priority);
			sThreadMap.put(name, thread);
			thread.start();

			return thread;
		}
	}

	public static CavanHandlerThread getInstance(Class<?> cls, int index) {
		return getInstance(cls, index, Process.THREAD_PRIORITY_DEFAULT);
	}

	public static CavanHandlerThread getInstance(Class<?> cls) {
		return getInstance(cls, 0);
	}

	public static Looper getLooper(Class<?> cls, int index, int priority) {
		return getInstance(cls, index, priority).getLooper();
	}

	public static Looper getLooper(Class<?> cls, int index) {
		return getLooper(cls, index, Process.THREAD_PRIORITY_DEFAULT);
	}

	public static Looper getLooper(Class<?> cls) {
		return getLooper(cls, 0);
	}

	private String mName;

	public CavanHandlerThread(String name, int priority) {
		super(name, priority);
		mName = name;
	}

	@Override
	public void run() {
		CavanAndroid.dLog("Enter CavanHandlerThread: " + mName);
		super.run();
		CavanAndroid.dLog("Exit CavanHandlerThread: " + mName);

		synchronized (sThreadMap) {
			sThreadMap.remove(mName);
		}
	}
}
