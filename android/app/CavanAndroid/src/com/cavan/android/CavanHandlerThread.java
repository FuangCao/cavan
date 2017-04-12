package com.cavan.android;

import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.Message;

public class CavanHandlerThread extends HandlerThread implements Callback {

	protected CavanThreadedHandler mHandler;

	public class CavanThreadedHandler extends Handler {

		public CavanThreadedHandler(Callback callback) {
			super(CavanHandlerThread.this.getLooper(), callback);
		}

		public CavanThreadedHandler() {
			this(CavanHandlerThread.this);
		}
	}

	public CavanHandlerThread(String name, int priority) {
		super(name, priority);
	}

	public CavanHandlerThread(String name) {
		super(name);
	}

	public CavanHandlerThread() {
		this(CavanHandlerThread.class.getCanonicalName());
	}

	public CavanThreadedHandler getHandler() {
		synchronized (this) {
			while (mHandler == null && isAlive()) {
				try {
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		return mHandler;
	}

	@Override
	protected void onLooperPrepared() {
		super.onLooperPrepared();
		mHandler = new CavanThreadedHandler();
	}

	@Override
	public boolean handleMessage(Message msg) {
		return false;
	}
}
