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

	public CavanMacAddress(String[] texts, char seprator) {
		mSeparator = seprator;
		parse(texts);
	}

	public CavanMacAddress(String[] texts) {
		this(texts, SEPRATOR);
	}

	public CavanMacAddress(String text, char seprator) {
		parse(text, seprator);
	}

	public CavanMacAddress(String text) {
		this(text, SEPRATOR);
	}

	public CavanMacAddress() {
		mSeparator = SEPRATOR;

		byte[] bytes = getBytes(6);

		for (int i = 0; i < bytes.length; i++) {
			bytes[i] = 0;
		}
	}

	public CavanMacAddress parse(String text, char seprator) {
		mSeparator = seprator;
		return parse(text.split("\\s*" + seprator + "\\s*"));
	}

	public CavanMacAddress parse(String[] texts) {
		byte[] bytes = getBytes(texts.length);

		for (int i = texts.length - 1, j = bytes.length - 1; i >= 0; i--, j--) {
			try {
				bytes[j] = (byte) Integer.parseInt(texts[i], 16);
			} catch (Exception e) {
				bytes[j] = 0;
			}
		}

		mBytes = bytes;

		return this;
	}

	public CavanMacAddress parse(int value) {
		byte[] bytes = getBytes(6);

		for (int i = bytes.length - 1; i >= 0; i--) {
			if (value < 1) {
				while (i >= 0) {
					bytes[i--] = 0;
				}

				break;
			}

			bytes[i] = (byte) value;
			value >>= 8;
		}

		mBytes = bytes;

		return this;
	}

	public CavanMacAddress parse(long value) {
		byte[] bytes = getBytes(6);

		for (int i = bytes.length - 1; i >= 0; i--) {
			if (value < 1) {
				while (i >= 0) {
					bytes[i--] = 0;
				}

				break;
			}

			bytes[i] = (byte) value;
			value >>= 8;
		}

		mBytes = bytes;

		return this;
	}

	public CavanMacAddress parse(float value) {
		byte[] bytes = getBytes(6);

		for (int i = bytes.length - 1; i >= 0; i--) {
			if (value < 1) {
				while (i >= 0) {
					bytes[i--] = 0;
				}

				break;
			}

			bytes[i] = (byte) value;
			value /= 0x100;
		}

		mBytes = bytes;

		return this;
	}

	public CavanMacAddress parse(double value) {
		byte[] bytes = getBytes(6);

		for (int i = bytes.length - 1; i >= 0; i--) {
			if (value < 1) {
				while (i >= 0) {
					bytes[i--] = 0;
				}

				break;
			}

			bytes[i] = (byte) value;
			value /= 0x100;
		}

		mBytes = bytes;

		return this;
	}

	public CavanMacAddress parse(String text) {
		return parse(text, SEPRATOR);
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public byte[] getBytes(int length) {
		if (mBytes == null || mBytes.length < length) {
			mBytes = new byte[length];
		} else {
			for (int i = mBytes.length - length - 1; i >= 0; i--) {
				mBytes[i] = 0;
			}
		}

		return mBytes;
	}

	public String[] getStrings() {
		String[] strings = new String[mBytes.length];

		for (int i = mBytes.length - 1; i >= 0; i--) {
			strings[i] = CavanString.fromByte(mBytes[i]);
		}

		return strings;
	}

	public int toInteger() {
		int value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = (value << 8) | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public long toLong() {
		long value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = (value << 8) | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public float toFloat() {
		float value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = value * 0x100 + (mBytes[i] & 0xFF);
		}

		return value;
	}

	public double toDouble() {
		double value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = value * 0x100 + (mBytes[i] & 0xFF);
		}

		return value;
	}

	public CavanMacAddress add(long value) {
		for (int i = mBytes.length - 1; i >= 0 && value != 0; i--, value >>= 8) {
			value += mBytes[i] & 0xFF;
			mBytes[i] = (byte) value;
		}

		return this;
	}

	public CavanMacAddress increase() {
		for (int i = mBytes.length - 1; i >= 0; i--) {
			if (mBytes[i] != (byte) 0xFF) {
				mBytes[i]++;
				break;
			}

			mBytes[i] = 0;
		}

		return this;
	}

	public CavanMacAddress decrease() {
		for (int i = mBytes.length - 1; i >= 0; i--) {
			if (mBytes[i] != (byte) 0) {
				mBytes[i]--;
				break;
			}

			mBytes[i] = (byte) 0xFF;
		}

		return this;
	}

	public CavanMacAddress add(byte[] bytes) {
		int carry = 0;

		for (int i = mBytes.length - 1, j = bytes.length - 1; i >= 0 && j >= 0; i--, j--) {
			carry += (mBytes[i] & 0xFF) + (bytes[j] & 0xFF);
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return this;
	}

	public CavanMacAddress add(CavanMacAddress address) {
		return add(address.getBytes());
	}

	public CavanMacAddress sub(byte[] bytes) {
		int carry = 0;

		for (int i = mBytes.length - 1, j = bytes.length - 1; i >= 0 && j >= 0; i--, j--) {
			carry = (mBytes[i] & 0xFF) - (bytes[j] & 0xFF) - carry;
			if (carry < 0) {
				mBytes[i] = (byte) (carry + 0x100);
				carry = 1;
			} else {
				mBytes[i] = (byte) carry;
				carry = 0;
			}
		}

		return this;
	}

	public CavanMacAddress sub(CavanMacAddress address) {
		return sub(address.getBytes());
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
		CavanMacAddress mac1 = new CavanMacAddress("2a:2b:3c:4d:5e:fe");
		CavanMacAddress mac2 = new CavanMacAddress("1a:2b:3c:4d:5e:02");

		for (int i = 0; i < 10; i++) {
			CavanJava.dLog("mac1 = " + mac1.increase());
		}

		for (int i = 0; i < 10; i++) {
			CavanJava.dLog("mac2 = " + mac2.decrease());
		}

		CavanJava.dLog("mac1 = " + mac1.parse(12345.5));
	}
}
