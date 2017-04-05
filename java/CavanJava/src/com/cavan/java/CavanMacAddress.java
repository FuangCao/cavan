package com.cavan.java;

public class CavanMacAddress extends CavanLargeValue {

	public static final char SEPRATOR = ':';

	private char mSeparator;

	public CavanMacAddress(byte[] bytes, char separator) {
		super(bytes);
		mSeparator = separator;
	}

	public CavanMacAddress(byte[] bytes) {
		this(bytes, SEPRATOR);
	}

	public CavanMacAddress(int length, char separator) {
		super(length);
		mSeparator = separator;
	}

	public CavanMacAddress(int length) {
		this(length, SEPRATOR);
	}

	public CavanMacAddress() {
		this(6);
	}

	public CavanMacAddress(CavanMacAddress address) {
		super(address);
		mSeparator = address.getSeparator();
	}

	public void setSeparator(char separator) {
		mSeparator = separator;
	}

	public char getSeparator() {
		return mSeparator;
	}

	public CavanMacAddress fromValues(int... args) {
		return (CavanMacAddress) super.fromValues(args);
	}

	public CavanMacAddress fromStrings(String... args) {
		return (CavanMacAddress) super.fromStrings(args);
	}

	public CavanMacAddress fromString(String text) {
		return (CavanMacAddress) super.fromString(text, mSeparator);
	}

	public static CavanMacAddress add(CavanMacAddress left, long value) {
		left = left.cloneMacAddress();
		left.add(value);
		return left;
	}

	public static CavanMacAddress sub(CavanMacAddress address, long value) {
		address = address.cloneMacAddress();
		address.sub(value);
		return address;
	}

	public static CavanMacAddress sub(CavanMacAddress left, CavanMacAddress right) {
		CavanMacAddress address = left.cloneMacAddress();
		address.sub(right);
		return address;
	}

	public CavanMacAddress cloneMacAddress() {
		return new CavanMacAddress(this);
	}

	@Override
	public String toString() {
		return toString(mSeparator);
	}

	public static void main(String[] args) {
		CavanMacAddress mac1 = new CavanMacAddress().fromString("2a:2b:3c:4d:5e:fe");
		CavanMacAddress mac2 = new CavanMacAddress().fromString("1a:2b:3c:4d:5e:02");

		for (int i = 0; i < 10; i++) {
			CavanJava.dLog("mac1 = " + mac1);
			mac1.increase();
		}

		for (int i = 0; i < 10; i++) {
			CavanJava.dLog("mac2 = " + mac2);
			mac2.decrease();
		}

		CavanJava.dLog("mac1 = " + mac1);
		CavanJava.dLog("mac2 = " + mac2);
		CavanJava.dLog("sub = " + CavanMacAddress.sub(mac1, mac2));
	}
}
