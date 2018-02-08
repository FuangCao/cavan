package com.cavan.android;

public abstract class CavanDaemonThread implements Runnable {

	private Thread mThread;

	public synchronized boolean isEnabled() {
		return (mThread != null);
	}

	public synchronized Thread getThread() {
		return mThread;
	}

	public synchronized void start() {
		Thread thread = mThread;
		if (thread != null) {
			synchronized (thread) {
				thread.notify();
			}
		} else {
			mThread = thread = new Thread(this);
			thread.start();
		}
	}

	public synchronized void stop() {
		Thread thread = mThread;
		if (thread != null) {
			mThread = null;
			onDaemonStopping();

			synchronized (thread) {
				thread.notify();
			}
		}
	}

	public synchronized void restart() {
		onDaemonStopping();
		start();
	}

	protected void onDaemonStarted() {}
	protected void onDaemonStopped() {}
	protected void onDaemonWaiting() {}
	protected void onDaemonRuning() {}
	protected void onDaemonStopping() {}

	protected abstract void mainLoop();

	@Override
	public void run() {
		onDaemonStarted();

		while (isEnabled()) {
			onDaemonRuning();
			CavanDaemonThread.this.mainLoop();
			onDaemonWaiting();

			synchronized (this) {
				try {
					wait(5000);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}

		onDaemonStopped();
	}
}
