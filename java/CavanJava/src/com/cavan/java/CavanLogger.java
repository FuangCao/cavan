package com.cavan.java;


public abstract class CavanLogger {

	public abstract void eLog(String message);
	public abstract void dLog(String message);

	public void wLog(String message) {
		eLog(message);
	}

	public void efLog(String format, Object... args) {
		eLog(String.format(format, args));
	}

	public void dfLog(String format, Object... args) {
		dLog(String.format(format, args));
	}

	public void wfLog(String format, Object... args) {
		dLog(String.format(format, args));
	}
}