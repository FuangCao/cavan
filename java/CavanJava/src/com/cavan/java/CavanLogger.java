package com.cavan.java;

import java.io.IOException;

public class CavanLogger extends CavanMemOutputStream {

	public void dLog(String message) {
		System.out.println(message);
	}

	public void wLog(String message) {
		eLog(message);
	}

	public void eLog(String message) {
		System.err.println(message);
	}

	public void eLog(Throwable throwable) {
		eLog(throwable, null);
	}

	public void eLog(Throwable throwable, String message) {
		eLog(CavanJava.buildThrowableMessage(throwable, message));
	}

	public void wLog(Throwable throwable) {
		eLog(throwable);
	}

	public void wLog(Throwable throwable, String message) {
		eLog(throwable, message);
	}

	public void efLog(String format, Object... args) {
		eLog(String.format(format, args));
	}

	public void wfLog(String format, Object... args) {
		wLog(String.format(format, args));
	}

	public void dfLog(String format, Object... args) {
		dLog(String.format(format, args));
	}

	public void pLog(int index) {
		dLog(CavanJava.buildPosMessage(index));
	}

	public void pLog() {
		pLog(5);
	}

	public void pfLog(int index, String format, Object... args) {
		dLog(CavanJava.buildPosMessage(format, args));
	}

	public void pfLog(String format, Object... args) {
		pfLog(5, format, args);
	}

	public void dumpstack(Throwable throwable) {
		eLog(throwable);
	}

	public void dumpstack() {
		eLog(new Throwable());
	}

	public void sepLog(int length) {
		dLog(CavanJava.buildSepMessage(length));
	}

	public void sepLog() {
		sepLog(0);
	}

	@Override
	public synchronized void flush() throws IOException {
		dLog(toString());
		clear();
	}
}
