package com.cavan.android;

import android.os.Handler;
import android.os.SystemClock;

public abstract class DelayedRunnable implements Runnable {

	protected long mTime;
	protected boolean mEnabled;
	protected Handler mHandler;

	protected void onRunableStateChanged(boolean enabled) {}
	protected void onRunableFire() {}

	public DelayedRunnable(Handler handler) {
		mHandler = handler;
	}

	@Override
	synchronized public void run() {
		if (mEnabled && mTime <= SystemClock.uptimeMillis()) {
			cancel();
			onRunableFire();
		}
	}

	synchronized public boolean isEnabled() {
		return mEnabled;
	}

	synchronized public long getDelay() {
		long time = SystemClock.uptimeMillis();
		if (mTime < time) {
			return 0;
		}

		return mTime - time;
	}

	synchronized public void post(long delay, boolean force) {
		long time = SystemClock.uptimeMillis() + delay;
		if (force || time > mTime) {
			if (!mEnabled) {
				mEnabled = true;
				onRunableStateChanged(true);
			}

			mTime = time;
			mHandler.removeCallbacks(this);
			mHandler.postAtTime(this, time);
		}
	}

	synchronized public void cancel() {
		if (mEnabled) {
			mEnabled = false;
			onRunableStateChanged(false);
		}
	}
}
