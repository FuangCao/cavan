package com.cavan.java;

public class CavanLargeValue implements Cloneable, Comparable<CavanLargeValue> {

	protected byte[] mBytes;

	public CavanLargeValue(int length) {
		mBytes = new byte[length];
		clear();
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

	public CavanLargeValue(int[] values, int index, int end) {
		fromValues(values, index, end);
	}

	public CavanLargeValue(int[] values, int index) {
		this(values, index, values.length);
	}

	public CavanLargeValue(int... args) {
		this(args, 0);
	}

	public CavanLargeValue(int radix, String... args) {
		fromStrings(radix, args);
	}

	public CavanLargeValue(CavanLargeValue value) {
		this(value.getBytes());
	}

	// ============================================================

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

	// ============================================================

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

	// ============================================================

	public int increase() {
		for (int i = 0; i < mBytes.length; i++) {
			if (mBytes[i] != (byte) 0xFF) {
				mBytes[i]++;
				return 0;
			}

			mBytes[i] = 0;
		}

		return 1;
	}

	public int decrease() {
		for (int i = 0; i < mBytes.length; i++) {
			if (mBytes[i] != 0) {
				mBytes[i]--;
				return 0;
			}

			mBytes[i] = (byte) 0xFF;
		}

		return -1;
	}

	public long add(long value) {
		for (int i = 0; i < mBytes.length && value != 0; i++) {
			value += mBytes[i] & 0xFF;
			mBytes[i] = (byte) value;
			value >>= 8;
		}

		return value;
	}

	public long sub(long value) {
		return add(-value);
	}

	public int add(byte[] bytes, int index, int end) {
		int carry = 0;

		for (int i = 0; i < mBytes.length; i++) {
			if (index < end) {
				carry += bytes[index++] & 0xFF;
			} else if (carry == 0) {
				return 0;
			}

			carry += mBytes[i] & 0xFF;
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public int add(byte[] bytes, int index) {
		return add(bytes, index, bytes.length);
	}

	public int add(byte[] bytes) {
		return add(bytes, 0);
	}

	public int add(CavanLargeValue value) {
		return add(value.getBytes());
	}

	public int sub(byte[] bytes, int index, int end) {
		int carry = 0;

		for (int i = 0; i < mBytes.length; i++) {
			if (index < end) {
				carry -= bytes[index++] & 0xFF;
			} else if (carry == 0) {
				return 0;
			}

			carry += mBytes[i] & 0xFF;
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public int sub(byte[] bytes, int index) {
		return sub(bytes, index, bytes.length);
	}

	public int sub(byte[] bytes) {
		return sub(bytes, 0);
	}

	public int sub(CavanLargeValue value) {
		return sub(value.getBytes());
	}

	public long mul(long value) {
		long carry = 0;

		for (int i = findLsb(); i < mBytes.length; i++) {
			carry += (mBytes[i] & 0xFF) * value;
			mBytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public CavanLargeValue mul(byte[] bytes, int index, int end) {
		end = findMsb(bytes, index, end) + 1;
		int length = findMsb() + 1;

		CavanLargeValue result = new CavanLargeValue(length + end - index);
		byte[] product = result.getBytes();

		for (int i = 0; index < end; i++, index++) {
			int value = bytes[index] & 0xFF;
			int carry = 0;
			int j = i;

			for (int k = 0; k < length; k++, j++) {
				carry += (product[j] & 0xFF) + (mBytes[k] & 0xFF) * value;
				product[j] = (byte) carry;
				carry >>= 8;
			}

			product[j] = (byte) carry;
		}

		return result;
	}

	public CavanLargeValue mul(byte[] bytes, int index) {
		return mul(bytes, index, bytes.length);
	}

	public CavanLargeValue mul(byte[] bytes) {
		return mul(bytes, 0);
	}

	public CavanLargeValue mul(CavanLargeValue value) {
		return mul(value.getBytes());
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

	// ============================================================

	public static CavanLargeValue add(CavanLargeValue a, CavanLargeValue b) {
		a = a.clone();
		a.add(b);
		return a;
	}

	public static CavanLargeValue sub(CavanLargeValue a, CavanLargeValue b) {
		a = a.clone();
		a.sub(b);
		return a;
	}

	public static CavanLargeValue mul(CavanLargeValue a, long value) {
		a = a.clone();
		a.mul(value);
		return a;
	}

	public static CavanLargeValue div(CavanLargeValue a, long value) {
		a = a.clone();
		a.div(value);
		return a;
	}

	// ============================================================

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

	// ============================================================

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

	public CavanLargeValue fromValues(int[] values, int index, int end) {
		int length = end - index;
		byte[] bytes = getBytes(length);

		for (int i = length - 1; i >= 0 && index < end; i--, index++) {
			bytes[i] = (byte) values[index];
		}

		return this;
	}

	public CavanLargeValue fromValues(int[] values, int index) {
		return fromValues(values, index, values.length);
	}

	public CavanLargeValue fromValues(int... args) {
		return fromValues(args, 0);
	}

	public CavanLargeValue fromString(String text, int index, int end, int radix) {
		clear();

		while (index < end) {
			int value = CavanString.convertCharToValue(text.charAt(index));
			if (value < 0 || value >= radix) {
				break;
			}

			mul(radix);
			add(value);
			index++;
		}

		return this;
	}

	public CavanLargeValue fromString(String text, int index, int end) {
		return fromString(text, index, end, 10);
	}

	public CavanLargeValue fromString(String text, int index) {
		return fromString(text, index, text.length());
	}

	public CavanLargeValue fromString(String text) {
		return fromString(text, 0);
	}

	// ============================================================

	public long toLong() {
		long value = 0;

		for (int i = mBytes.length - 1; i >= 0; i--) {
			value = value << 8 | (mBytes[i] & 0xFF);
		}

		return value;
	}

	public double toDouble() {
		return toLong();
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
				chars[j] = CavanString.convertValueToCharUppercase((value >> k) & 1);
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
			chars[length] = CavanString.convertValueToCharUppercase(remain);
		}

		CavanArray.reverse(chars, 0, length);

		return new String(chars, 0, length);
	}

	@Override
	public String toString() {
		return toString(10);
	}

	// ============================================================

	public CavanLargeValue clone() {
		return new CavanLargeValue(mBytes);
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

	@Override
	public int compareTo(CavanLargeValue o) {
		return compareTo(o.getBytes());
	}

	public static void main(String[] args) {
		CavanLargeValue value1 = new CavanLargeValue(100).fromString("123456789012345678901234567890123456789012345678901234567890");
		CavanLargeValue value2 = new CavanLargeValue(100).fromString("112233445566778899001122334455667788990011223344556677889900");

		CavanJava.dLog("value1 = " + value1);
		CavanJava.dLog("value2 = " + value2);
		CavanJava.dLog("mul = " + value1.mul(value2));
	}
}
