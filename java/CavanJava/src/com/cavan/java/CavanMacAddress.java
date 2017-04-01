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
		parseStrings(texts);
	}

	public CavanMacAddress(String[] texts) {
		this(texts, SEPRATOR);
	}

	public CavanMacAddress(String text, char seprator) {
		parseString(text, seprator);
	}

	public CavanMacAddress(String text) {
		this(text, SEPRATOR);
	}

	public CavanMacAddress() {
		mSeparator = SEPRATOR;
		clear(getBytes(6));
	}

	public CavanMacAddress parseString(String text, char seprator) {
		mSeparator = seprator;
		return parseStrings(text.split("\\s*" + seprator + "\\s*"));
	}

	public CavanMacAddress parseString(String text) {
		return parseString(text, SEPRATOR);
	}

	public CavanMacAddress parseStrings(String[] texts) {
		byte[] bytes = getBytes(texts.length);

		for (int i = texts.length - 1, j = bytes.length - 1; i >= 0; i--, j--) {
			try {
				String text = texts[i];

				if (text == null || text.isEmpty()) {
					bytes[j] = 0;
				} else {
					bytes[j] = (byte) Integer.parseInt(texts[i], 16);
				}
			} catch (Exception e) {
				bytes[j] = 0;
			}
		}

		return this;
	}

	public CavanMacAddress parseInt(int value) {
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

		return this;
	}

	public CavanMacAddress parseLong(long value) {
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

		return this;
	}

	public CavanMacAddress parseFloat(float value) {
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

		return this;
	}

	public CavanMacAddress parseDouble(double value) {
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

		return this;
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public byte[] getBytes(int length) {
		if (mBytes == null || mBytes.length < length) {
			mBytes = new byte[length];
		} else {
			clear(mBytes, 0, mBytes.length - length);
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

	public int getIntValue() {
		int value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = (value << 8) | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public long getLongValue() {
		long value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = (value << 8) | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public float getFloatValue() {
		float value = 0;

		for (int i = 0; i < mBytes.length; i++) {
			value = value * 0x100 + (mBytes[i] & 0xFF);
		}

		return value;
	}

	public double getDoubleValue() {
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

	public void clear(byte[] bytes, int start, int end) {
		while (start < end) {
			bytes[start++] = 0;
		}
	}

	public void clear(byte[] bytes, int start) {
		clear(bytes, start, bytes.length);
	}

	public void clear(byte[] bytes) {
		clear(bytes, 0);
	}

	public void clear() {
		if (mBytes != null) {
			clear(mBytes);
		}
	}

	public void clearByLength(byte[] bytes, int start, int length) {
		clear(bytes, start, start + length);
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

		CavanJava.dLog("mac1 = " + mac1.parseDouble(12345.5));
	}
}
