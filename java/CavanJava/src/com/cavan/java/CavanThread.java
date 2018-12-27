package com.cavan.java;

public class CavanThread extends Thread {

	public CavanThread() {
		super();
	}

	public CavanThread(Runnable runnable) {
		super(runnable);
	}

	public synchronized void wakeup() {
		if (isAlive()) {
			super.notify();
		} else {
			super.start();
		}
	}

	@Override
	public synchronized void start() {
		if (!isAlive()) {
			super.start();
		}
	}
}
