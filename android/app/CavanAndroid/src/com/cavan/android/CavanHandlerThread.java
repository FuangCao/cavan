package com.cavan.android;

import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.Message;

public class CavanHandlerThread extends HandlerThread implements Callback {

	protected CavanThreadedHandler mThreadedHandler;

	public class CavanThreadedHandler extends Handler {

		public CavanThreadedHandler(Callback callback) {
			super(CavanHandlerThread.this.getLooper(), callback);
		}

		public CavanThreadedHandler() {
			this(CavanHandlerThread.this);
		}
	}

	public CavanHandlerThread(String name) {
		super(name);
		super.start();
	}

	public CavanHandlerThread() {
		this(CavanHandlerThread.class.getCanonicalName());
	}

	public CavanThreadedHandler getHandler() {
		synchronized (this) {
			while (mThreadedHandler == null && isAlive()) {
				try {
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		return mThreadedHandler;
	}

	@Override
	public synchronized void start() {
		CavanAndroid.dLog("Nothing to be done");
	}

	@Override
	protected void onLooperPrepared() {
		super.onLooperPrepared();
		mThreadedHandler = new CavanThreadedHandler();
	}

	@Override
	public boolean handleMessage(Message msg) {
		return false;
	}
}
