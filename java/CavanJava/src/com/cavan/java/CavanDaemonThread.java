package com.cavan.java;

public abstract class CavanDaemonThread extends Thread {

	private boolean mSuspended;

	private Thread mStopThread = new Thread() {

		@Override
		public synchronized void start() {
			if (isAlive()) {
				super.notify();
			} else {
				super.start();
			}
		}

		@Override
		public void run() {
			while (true) {
				synchronized (CavanDaemonThread.this) {
					if (mSuspended) {
						onDaemonStop();
					}
				}

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	public synchronized boolean isSuspended() {
		return mSuspended;
	}

	public synchronized void stopDaemon() {
		mSuspended = true;
		mStopThread.start();
	}

	public synchronized void startDaemon() {
		mSuspended = false;

		if (isAlive()) {
			notify();
		} else {
			start();
		}
	}

	@Override
	public final void run() {
		while (true) {
			synchronized (this) {
				while (mSuspended) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			if (runDaemon()) {
				synchronized (this) {
					mSuspended = true;
				}
			}
		}
	}

	protected abstract void onDaemonStop();
	protected abstract boolean runDaemon();
}
