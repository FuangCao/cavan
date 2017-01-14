package com.cavan.android;

import android.os.Handler;
import android.os.SystemClock;

public abstract class DelayedSwitch implements Runnable {

	protected long mTime;
	protected boolean mEnabled;
	protected Handler mHandler;

	protected void onSwitchStateChanged(boolean enabled) {}
	protected void handleMessage() {}

	public DelayedSwitch(Handler handler) {
		mHandler = handler;
	}

	@Override
	synchronized public final void run() {
		if (mEnabled && mTime <= SystemClock.uptimeMillis()) {
			handleMessage();
		}
	}

	synchronized public boolean isEnabled() {
		return mEnabled;
	}

	synchronized public void post(long delay, boolean force) {
		long time = SystemClock.uptimeMillis() + delay;
		if (force || time > mTime) {
			if (!mEnabled) {
				mEnabled = true;
				onSwitchStateChanged(true);
			}

			mTime = time;
			mHandler.postAtTime(this, time);
		}
	}

	synchronized public void cancel() {
		if (mEnabled) {
			mEnabled = false;
			onSwitchStateChanged(false);
		}

		mHandler.removeCallbacks(this);
	}
}
