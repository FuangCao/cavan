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

	public synchronized void msleep() {
		try {
			wait();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	public synchronized void msleep(long millis) {
		try {
			wait(millis);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	@Override
	public synchronized void start() {
		if (!isAlive()) {
			super.start();
		}
	}
}
