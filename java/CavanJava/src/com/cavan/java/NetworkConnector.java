package com.cavan.java;

public abstract class NetworkConnector implements Runnable {

	protected abstract boolean doConnect();
	protected abstract void doDisconnect();
	protected abstract void onDaemonRun();

	private boolean mActive;
	private Thread mThread;
	private int mConnDelay;

	synchronized public void start() {
		mActive = true;
		mConnDelay = 0;

		if (mThread == null) {
			mThread = new Thread(this);
			mThread.start();
		}
	}

	synchronized public void stop() {
		mActive = false;
		doDisconnect();
	}

	synchronized public void restart() {
		mActive = true;
		doDisconnect();
		start();
	}

	@Override
	protected void finalize() throws Throwable {
		stop();
	}

	@Override
	public void run() {
		CavanJava.dLog("enter daemon");

		while (mActive) {
			CavanJava.dLog("start connect");

			if (doConnect()) {
				CavanJava.dLog("successfull connect");

				onDaemonRun();
				doDisconnect();
				mConnDelay = 0;
			} else if (mActive) {
				mConnDelay = mConnDelay * 2 + 500;
				CavanJava.dLog("mConnDelay = " + mConnDelay);

				synchronized (this) {
					try {
						wait(mConnDelay);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}

		mThread = null;
		CavanJava.dLog("exit daemon");
	}
}
