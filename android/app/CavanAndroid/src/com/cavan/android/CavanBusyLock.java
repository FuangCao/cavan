package com.cavan.android;

import android.os.Handler;
import android.os.Message;

public class CavanBusyLock extends Handler {

	private long mDelay;
	private Object mOwner;

	public CavanBusyLock(long delay) {
		mDelay = delay;
	}

	synchronized public long getDelay() {
		return mDelay;
	}

	synchronized public void setDelay(long delay) {
		mDelay = delay;
	}

	synchronized public Object getOwner() {
		return mOwner;
	}

	synchronized public boolean isBusy() {
		return (mOwner != null);
	}

	synchronized public void setOwner(Object owner) {
		mOwner = owner;
	}

	synchronized public boolean acquire(Object owner) {
		if (mOwner != owner) {
			if (mOwner != null) {
				return false;
			}

			mOwner = owner;
		}

		removeMessages(0);
		sendEmptyMessageDelayed(0, mDelay);

		return true;
	}

	synchronized public boolean release(Object owner) {
		if (mOwner == owner) {
			removeMessages(0);
			mOwner = null;
			return true;
		}

		return false;
	}

	@Override
	public void handleMessage(Message msg) {
		removeMessages(msg.what);

		synchronized (this) {
			mOwner = null;
		}
	}
}
