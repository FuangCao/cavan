package com.cavan.android;

import com.cavan.android.AndroidListeners.CavanBusyLockListener;

public class CavanBusyLock implements CavanBusyLockListener, Runnable {

	private long mDelay;
	private Object mOwner;
	private CavanBusyLockListener mListener = this;

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

	synchronized public CavanBusyLockListener getListener() {
		return mListener;
	}

	synchronized public void setListener(CavanBusyLockListener listener) {
		mListener = listener;
	}

	synchronized public boolean acquire(Object owner) {
		if (mOwner != owner) {
			if (mOwner != null) {
				return false;
			}

			mOwner = owner;
			mListener.onBusyLockAcquired(owner);
		}

		if (CavanAndroid.postRunnable(this, mDelay)) {
			return true;
		}

		mOwner = null;

		return false;
	}

	synchronized private void release() {
		CavanAndroid.removeRunnable(this);

		if (mOwner != null) {
			Object owner = mOwner;
			mOwner = null;
			mListener.onBusyLockReleased(owner);
		}
	}

	synchronized public boolean release(Object owner) {
		if (mOwner != owner) {
			return false;
		}

		release();

		return true;
	}

	@Override
	public void onBusyLockAcquired(Object owner) {
		CavanAndroid.dLog("onLockAcquired: owner = " + owner);
	}

	@Override
	public void onBusyLockReleased(Object owner) {
		CavanAndroid.dLog("onLockReleased: owner = " + owner);
	}

	@Override
	public void run() {
		release();
	}
}
