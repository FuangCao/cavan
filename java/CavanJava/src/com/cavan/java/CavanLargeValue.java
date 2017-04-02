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

	public CavanLargeValue(byte[] bytes) {
		mBytes = bytes.clone();
	}

	public CavanLargeValue(CavanLargeValue value) {
		this(value.getBytes());
	}

	public byte[] getBytes() {
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

		for (int i = 0; i < mBytes.length; i++) {
			product += (mBytes[i] & 0xFF) * value;
			mBytes[i] = (byte) product;
			product >>= 8;
		}

		return this;
	}

	public long div(long value) {
		long remain = 0;

		for (int i = mBytes.length - 1; i >= 0; i--) {
			remain = (remain << 8) + (mBytes[i] & 0xFF);
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
		for (int i = end - 1; i >= index; i--) {
			if (bytes[i] != 0) {
				return i;
			}
		}

		return -1;
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

	public void fromBytes(byte[] bytes, int index, int end) {
		int length = end - index;

		if (mBytes == null || mBytes.length < length) {
			mBytes = new byte[length];
		} else {
			clear(length);
		}

		System.arraycopy(bytes, index, mBytes, 0, length);
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
		StringBuilder builder = new StringBuilder(mBytes.length * 2);

		for (int i = mBytes.length - 1; i >= 0; i--) {
			CavanString.fromByte(builder, mBytes[i]);
		}

		return builder.toString();
	}

	public String toString(int radix) {
		int count;
		char[] chars = new char[length() * 8];
		CavanLargeValue value = new CavanLargeValue(this);

		for (count = 0; value.notZero(); count++) {
			int remain = (int) value.div(radix);
			chars[count] = CavanString.convertToCharUppercase(remain);
		}

		CavanArray.reverse(chars, 0, count);

		return new String(chars, 0, count);
	}

	public static void main(String[] args) {
		CavanLargeValue value1 = new CavanLargeValue(8).fromLong(0x12345);
		CavanLargeValue value2 = new CavanLargeValue(2).fromLong(1000);

		CavanJava.dLog("compareTo = " + value1.compareTo(value2));
		// CavanJava.dLog("remain = " + value1.div(55));
		CavanJava.dLog("value1 = " + value1.toString(12));
	}

	@Override
	public String toString() {
		return Long.toString(toLong());
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
