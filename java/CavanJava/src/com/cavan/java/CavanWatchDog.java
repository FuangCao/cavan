package com.cavan.java;

public abstract class CavanWatchDog extends Thread {

	private boolean mEnabled;
	private long mFeedTime;
	private	int mOvertime;

	public CavanWatchDog(int overtime) {
		mOvertime = overtime;
	}

	public synchronized boolean isEnabled() {
		return mEnabled;
	}

	public synchronized void enable() {
		mEnabled = true;
		feed();

		if (isAlive()) {
			notify();
		} else {
			start();
		}
	}

	public synchronized void disable() {
		mEnabled = false;
	}

	public synchronized void feed() {
		mFeedTime = System.currentTimeMillis();
	}

	protected abstract void onWatchDogFire();

	@Override
	public synchronized void run() {
		while (true) {
			try {
				if (mEnabled) {
					long timeNow = System.currentTimeMillis();
					long time = mFeedTime + mOvertime;

					if (time > timeNow) {
						wait(time - timeNow);
					} else if (mEnabled) {
						mEnabled = false;
						onWatchDogFire();
					}
				} else {
					wait();
				}
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}
