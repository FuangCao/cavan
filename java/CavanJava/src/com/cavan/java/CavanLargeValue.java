package com.cavan.java;

public class CavanLargeValue implements Cloneable, Comparable<CavanLargeValue> {

	private byte[] mBytes;

	public CavanLargeValue(int length) {
		mBytes = new byte[length];
	}

	public CavanLargeValue(byte[] bytes, int index, int end) {
		fromBytes(bytes, index, end);
	}

	public CavanLargeValue(byte[] bytes, int index) {
		this(bytes, index, bytes.length);
	}

	public CavanLargeValue(byte... args) {
		mBytes = args.clone();
	}

	public CavanLargeValue(String[] texts, int index, int end, int radix) {
		fromStrings(texts, index, end, radix);
	}

	public CavanLargeValue(String[] texts, int index, int end) {
		fromStrings(texts, index, texts.length);
	}

	public CavanLargeValue(String[] texts, int index) {
		this(texts, index, texts.length);
	}

	public CavanLargeValue(String... texts) {
		this(texts, 0);
	}

	public CavanLargeValue(int radix, String... args) {
		fromStrings(radix, args);
	}

	public CavanLargeValue(CavanLargeValue value) {
		this(value.getBytes());
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public byte[] getBytes(int length) {
		if (mBytes == null || mBytes.length < length) {
			mBytes = new byte[length];
		} else {
			clear(length);
		}

		return mBytes;
	}

	public void setBytes(byte[] bytes) {
		mBytes = bytes;
	}

	public byte getByte(int index) {
		return mBytes[index];
	}

	public boolean isZero() {
		for (byte value : mBytes) {
			if (value != 0) {
				return false;
			}
		}

		return true;
	}

	public boolean notZero() {
		for (byte value : mBytes) {
			if (value != 0) {
				return true;
			}
		}

		return false;
	}

	public static void clear(byte[] bytes, int index, int end) {
		while (index < end) {
			bytes[index++] = 0;
		}
	}

	public static void clear(byte[] bytes, int index) {
		clear(bytes, index, bytes.length);
	}

	public static void clear(byte[] bytes) {
		clear(bytes, 0);
	}

	public void clear(int index, int end) {
		clear(mBytes, index, end);
	}

	public void clear(int index) {
		clear(mBytes, index);
	}

	public void clear() {
		clear(0);
	}

	public CavanLargeValue increase() {
		for (int i = 0; i < mBytes.length; i++) {
			if (mBytes[i] != (byte) 0xFF) {
				mBytes[i]++;
				break;
			}

			mBytes[i] = 0;
		}

		return this;
	}

	public CavanLargeValue decrease() {
		for (int i = 0; i < mBytes.length; i++) {
			if (mBytes[i] != 0) {
				mBytes[i]--;
				break;
			}

			mBytes[i] = (byte) 0xFF;
		}

		return this;
	}

	public CavanLargeValue add(long value) {
		for (int i = 0; i < mBytes.length && value != 0; i++) {
			value += mBytes[i] & 0xFF;
			mBytes[i] = (byte) value;
			value >>= 8;
		}

		return this;
	}

	public CavanLargeValue sub(long value) {
		return add(-value);
	}

	public boolean add(byte[] bytes, int index, int end) {
		int carry = 0;

		for (int i = 0; i < mBytes.length; i++) {
			if (index < end) {
				carry += bytes[index++] & 0xFF;
			} else if (carry == 0) {
				return true;
			}

			carry += mBytes[i] & 0xFF;
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry == 0;
	}

	public boolean add(byte[] bytes, int index) {
		return add(bytes, index, bytes.length);
	}

	public boolean add(byte[] bytes) {
		return add(bytes, 0);
	}

	public boolean add(CavanLargeValue value) {
		return add(value.getBytes());
	}

	public boolean sub(byte[] bytes, int index, int end) {
		int carry = 0;

		for (int i = 0; i < mBytes.length; i++) {
			if (index < end) {
				carry -= bytes[index++] & 0xFF;
			} else if (carry == 0) {
				return true;
			}

			carry += mBytes[i] & 0xFF;
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry == 0;
	}

	public boolean sub(byte[] bytes, int index) {
		return sub(bytes, index, bytes.length);
	}

	public boolean sub(byte[] bytes) {
		return sub(bytes, 0);
	}

	public boolean sub(CavanLargeValue value) {
		return sub(value.getBytes());
	}

	public CavanLargeValue mul(long value) {
		long product = 0;

		for (int i = findLsb(); i < mBytes.length; i++) {
			product += (mBytes[i] & 0xFF) * value;
			mBytes[i] = (byte) product;
			product >>= 8;
		}

		return this;
	}

	public long div(long value) {
		long remain = 0;

		for (int i = findMsb(); i >= 0; i--) {
			remain = (remain << 8) | (mBytes[i] & 0xFF);
			mBytes[i] = (byte) (remain / value);
			remain %= value;
		}

		return remain;
	}

	public long toLong() {
		long value = 0;

		for (int i = mBytes.length - 1; i >= 0; i--) {
			value = value << 8 | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public static int findMsb(byte[] bytes, int index, int end) {
		while (--end >= index && bytes[end] == 0);

		return end;
	}

	public static int findMsb(byte[] bytes, int index) {
		return findMsb(bytes, index, bytes.length);
	}

	public static int findMsb(byte[] bytes) {
		return findMsb(bytes, 0);
	}

	public int findMsb() {
		return findMsb(mBytes);
	}

	public int findLsb(byte[] bytes, int index, int end) {
		while (index < end && bytes[index] == 0) {
			index++;
		}

		return index;
	}

	public int findLsb(byte[] bytes, int index) {
		return findLsb(bytes, index, bytes.length);
	}

	public int findLsb(byte[] bytes) {
		return findLsb(bytes, 0);
	}

	public int findLsb() {
		return findLsb(mBytes);
	}

	public int compareTo(byte[] bytes, int index, int end) {
		int i = findMsb();
		int j = findMsb(bytes, index, end);

		if (i > j) {
			return 1;
		} else if (i < j) {
			return -1;
		} else {
			while (i >= 0 && j >= index) {
				if (mBytes[i] != bytes[j]) {
					return (mBytes[i] & 0xFF) - (bytes[j] & 0xFF);
				}

				i--;
				j--;
			}
		}

		return 0;
	}

	public int compareTo(byte[] bytes, int index) {
		return compareTo(bytes, index, bytes.length);
	}

	public int compareTo(byte[] bytes) {
		return compareTo(bytes, 0);
	}

	public CavanLargeValue fromLong(long value) {
		for (int i = 0; i < mBytes.length; i++, value >>= 8) {
			mBytes[i] = (byte) value;
		}

		return this;
	}

	public CavanLargeValue fromDouble(double value) {
		return fromLong((long) value);
	}

	public CavanLargeValue fromBytes(byte[] bytes, int index, int end) {
		int length = end - index;

		System.arraycopy(bytes, index, getBytes(length), 0, length);
		return this;
	}

	public CavanLargeValue fromBytes(byte[] bytes, int index) {
		return fromBytes(bytes, index, bytes.length);
	}

	public CavanLargeValue fromBytes(byte... args) {
		mBytes = args.clone();
		return this;
	}

	public CavanLargeValue fromStrings(String[] texts, int index, int end, int radix) {
		int length = end - index;
		byte[] bytes = getBytes(length);

		for (int i = length - 1; i >= 0; i--, index++) {
			try {
				bytes[i] = (byte) Integer.parseInt(texts[index], radix);
			} catch (NumberFormatException e) {
				e.printStackTrace();
			}
		}

		return this;
	}

	public CavanLargeValue fromStrings(String[] texts, int index, int end) {
		return fromStrings(texts, index, end, 16);
	}

	public CavanLargeValue fromStrings(String[] texts, int index) {
		return fromStrings(texts, index, texts.length);
	}

	public CavanLargeValue fromStrings(String... args) {
		return fromStrings(args, 0);
	}

	public CavanLargeValue fromStrings(int radix, String... args) {
		return fromStrings(args, 0, args.length, radix);
	}

	public int length() {
		return mBytes.length;
	}

	public void setLength(int length) {
		if (length == mBytes.length) {
			return;
		}

		byte[] bytes = mBytes;

		mBytes = new byte[length];

		if (length > bytes.length) {
			length = bytes.length;
			clear(bytes.length);
		}

		System.arraycopy(bytes, 0, mBytes, 0, length);
	}

	public String toStringHex() {
		char[] chars = new char[mBytes.length * 2];

		for (int i = mBytes.length - 1, j = 0; i >= 0; i--, j += 2) {
			CavanString.fromByte(chars, j, mBytes[i]);
		}

		return new String(chars);
	}

	public String toStringBin() {
		char[] chars = new char[mBytes.length * 8];

		for (int i = mBytes.length - 1, j = 0; i >= 0; i--) {
			byte value = mBytes[i];

			for (int k = 7; k >= 0; j++, k--) {
				chars[j] = CavanString.convertToCharUppercase((value >> k) & 1);
			}
		}

		return new String(chars);
	}

	public String toString(int radix) {
		char[] chars = new char[length() * 8];
		CavanLargeValue value = clone();
		int length;

		for (length = 0; value.notZero(); length++) {
			int remain = (int) value.div(radix);
			chars[length] = CavanString.convertToCharUppercase(remain);
		}

		CavanArray.reverse(chars, 0, length);

		return new String(chars, 0, length);
	}

	public CavanLargeValue clone() {
		return new CavanLargeValue(mBytes);
	}

	public static void main(String[] args) {
		CavanLargeValue value1 = new CavanLargeValue(4).fromStrings("12", "34");
		CavanLargeValue value2 = new CavanLargeValue(2).fromLong(1000);

		CavanJava.dLog("compareTo = " + value1.compareTo(value2));
		CavanJava.dLog("value1 = " + value1.toString(16) + " = " + value1.toStringHex());
	}

	@Override
	public String toString() {
		return toString(10);
	}

	@Override
	public boolean equals(Object obj) {
		if (obj instanceof CavanLargeValue) {
			return compareTo((CavanLargeValue) obj) == 0;
		} else if (obj instanceof byte[]) {
			return compareTo((byte[]) obj) == 0;
		}

		return false;
	}

	@Override
	public int hashCode() {
		return mBytes.hashCode();
	}

	@Override
	public int compareTo(CavanLargeValue o) {
		return compareTo(o.getBytes());
	}
}
