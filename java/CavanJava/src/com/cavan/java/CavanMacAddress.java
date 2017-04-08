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

	public CavanMacAddress increaseMacAddress() {
		CavanMacAddress address = cloneMacAddress();
		address.increase();
		return address;
	}

	public CavanMacAddress decreaseMacAddress() {
		CavanMacAddress address = cloneMacAddress();
		address.decrease();
		return address;
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

	public CavanMacAddress getAddressEnd(long count) {
		CavanMacAddress address = cloneMacAddress();

		if (count > 0) {
			address.add(count - 1);
		} else {
			address.decrease();
		}

		return address;
	}

	public CavanMacAddress getAddressEnd() {
		return decreaseMacAddress();
	}

	public CavanMacAddress getAddressNext(long count) {
		return add(this, count);
	}

	public CavanMacAddress getAddressNext() {
		return increaseMacAddress();
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

		CavanJava.dLog("mac1 = " + mac1.fromString("00:00:11:22:33:44:55:66"));
		CavanJava.dLog("mac1 = " + mac1.fromValues(0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66));

		mac2.fromString("11:22:00:00:00:00");
		CavanJava.dLog("startsWith = " + mac1.startsWith(mac2));
	}
}
