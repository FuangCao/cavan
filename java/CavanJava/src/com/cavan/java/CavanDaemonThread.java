package com.cavan.java;


public abstract class CavanDaemonThread extends Thread {

	private boolean mSuspended;

	private Thread mSendThread = new Thread() {

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
				while (true) {
					synchronized (CavanDaemonThread.this) {
						if (mSuspended) {
							doDisconnect();
							break;
						}
					}

					if (doSendData()) {
						synchronized (CavanDaemonThread.this) {
							if (mSuspended) {
								doDisconnect();
							}
						}

						break;
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

	public void startSendThread() {
		mSendThread.start();
	}

	public synchronized void stopDaemon() {
		mSuspended = true;
		mSendThread.start();
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

	protected boolean doSendData() {
		return true;
	}

	protected abstract void doDisconnect();
	protected abstract boolean runDaemon();
}
