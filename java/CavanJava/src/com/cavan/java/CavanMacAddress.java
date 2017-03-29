package com.cavan.java;


public class CavanMacAddress {

	public static final char SEPRATOR = ':';

	private byte[] mBytes;
	private char mSeparator;

	public CavanMacAddress(byte[] bytes, char seprator) {
		mBytes = bytes;
		mSeparator = seprator;
	}

	public CavanMacAddress(byte[] bytes) {
		this(bytes, SEPRATOR);
	}

	public CavanMacAddress(String text, char seprator) {
		parseString(text, seprator);
	}

	public CavanMacAddress(String text) {
		this(text, SEPRATOR);
	}

	public CavanMacAddress() {
		mBytes = new byte[6];
		mSeparator = SEPRATOR;

		for (int i = 0; i < mBytes.length; i++) {
			mBytes[i] = 0;
		}
	}

	public CavanMacAddress parseString(String text, char seprator) {
		String[] texts = text.split("\\s*" + seprator + "\\s*");
		byte[] bytes = new byte[texts.length];

		for (int i = 0; i < texts.length; i++) {
			try {
				bytes[i] = (byte) Integer.parseInt(texts[i], 16);
			} catch (Exception e) {
				bytes[i] = 0;
			}
		}

		mBytes = bytes;
		mSeparator = seprator;

		return this;
	}

	public CavanMacAddress parseString(String text) {
		return parseString(text, SEPRATOR);
	}

	public CavanMacAddress add(int value) {
		for (int i = mBytes.length - 1; i >= 0 && value != 0; i--, value >>= 8) {
			value += mBytes[i] & 0xFF;
			mBytes[i] = (byte) (value & 0xFF);
		}

		return this;
	}

	public CavanMacAddress increase() {
		return add(1);
	}

	public CavanMacAddress decrease() {
		return add(-1);
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		for (byte value : mBytes) {
			if (builder.length() > 0) {
				builder.append(mSeparator);
			}

			CavanString.fromByte(builder, value);
		}

		return builder.toString();
	}

	public static void main(String[] args) {
		CavanMacAddress mac = new CavanMacAddress("1a:2b:3c:4d:5e:6f");

		for (int i = 0; i < 20; i++) {
			CavanJava.dLog("mac = " + mac);
			mac.add(-2000);
			mac.add(2000);
		}
	}
}
