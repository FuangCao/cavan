package com.cavan.java;

public class CavanJavaLogger extends CavanLogger {

	@Override
	public void eLog(String message) {
		System.err.println(message);
	}

	@Override
	public void dLog(String message) {
		System.out.println(message);
	}
}