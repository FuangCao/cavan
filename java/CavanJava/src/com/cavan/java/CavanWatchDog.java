package com.cavan.java;

public abstract class CavanWatchDog extends Thread {

	private boolean mEnabled;
	private boolean mHungry;
	private	int mMseconds;

	public CavanWatchDog(int mseconds) {
		mMseconds = mseconds;
	}

	public synchronized boolean isEnabled() {
		return mEnabled;
	}

	public synchronized void setEnabled(boolean enabled) {
		mEnabled = enabled;
		mHungry = false;

		if (enabled) {
			if (isAlive()) {
				notify();
			} else {
				start();
			}
		}
	}

	public synchronized void feed() {
		mHungry = false;
	}

	protected abstract void onWatchDogFire();

	@Override
	public synchronized void run() {
		while (true) {
			while (mEnabled) {
				mHungry = true;

				try {
					wait(mMseconds);

					if (mHungry && mEnabled) {
						onWatchDogFire();
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			try {
				wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}
