package com.cavan.java;

public class CavanLargeValue implements Cloneable, Comparable<CavanLargeValue> {

	public static int findMsbIndex(byte[] bytes, int msb) {
		while (msb >= 0 && bytes[msb] == 0) {
			msb--;
		}

		return msb;
	}

	public static int findMsbIndex(byte[] bytes) {
		return findMsbIndex(bytes, bytes.length - 1);
	}

	public static int findLsbIndex(byte[] bytes, int lsb) {
		while (lsb < bytes.length && bytes[lsb] == 0) {
			lsb++;
		}

		return lsb;
	}

	public static int findLsbIndex(byte[] bytes) {
		return findLsbIndex(bytes, 0);
	}

	public static boolean isZero(byte[] bytes) {
		for (byte value : bytes) {
			if (value != 0) {
				return false;
			}
		}

		return true;
	}

	public static boolean notZero(byte[] bytes) {
		for (byte value : bytes) {
			if (value != 0) {
				return true;
			}
		}

		return false;
	}

	public static byte getLastByte(byte[] bytes) {
		return bytes[bytes.length - 1];
	}

	public static boolean isNegative(byte[] bytes) {
		return (getLastByte(bytes) & (1 << 7)) != 0;
	}

	public static boolean isPositive(byte[] bytes) {
		return (getLastByte(bytes) & (1 << 7)) == 0;
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

	public static int compare(byte[] bytes1, int msb1, byte[] bytes2, int msb2) {
		if (msb1 > msb2) {
			return 1;
		}

		if (msb1 < msb2) {
			return -1;
		}

		for (int i = msb1; i >= 0; i--) {
			if (bytes1[i] != bytes2[i]) {
				return Integer.compare(bytes1[i] & 0xFF, bytes2[i] & 0xFF);
			}
		}

		return 0;
	}

	public static int compare(byte[] bytes1, byte[] bytes2) {
		return compare(bytes1, findMsbIndex(bytes1), bytes2, findMsbIndex(bytes2));
	}

	public static boolean startsWith(byte[] bytes1, int lsb1, byte[] bytes2, int lsb2) {
		for (int i = bytes1.length - 1, j = bytes2.length - 1; j >= lsb2; i--, j--) {
			if (i < lsb1 || bytes1[i] != bytes2[j]) {
				return false;
			}
		}

		return true;
	}

	public static boolean startsWith(byte[] bytes1, byte[] bytes2) {
		return startsWith(bytes1, findLsbIndex(bytes1), bytes2, findLsbIndex(bytes2));
	}

	public static void setPrefix(byte[] bytes1, byte[] bytes2, int lsb) {
		int length = Math.min(bytes1.length, bytes2.length);

		for (int i = lsb; i < length; i++) {
			bytes1[i] = bytes2[i];
		}
	}

	public static void setPrefix(byte[] bytes1, byte[] bytes2) {
		setPrefix(bytes1, bytes2, findLsbIndex(bytes2));
	}

	public static boolean endsWith(byte[] bytes1, byte[] bytes2) {
		for (int i = 0; i < bytes2.length; i++) {
			if (i >= bytes1.length || bytes1[i] != bytes2[i]) {
				return false;
			}
		}

		return true;
	}

	public static void setSuffix(byte[] bytes1, byte[] bytes2, int msb) {
		while (msb > bytes1.length) {
			msb--;
		}

		for (int i = msb; i >= 0; i--) {
			bytes1[i] = bytes2[i];
		}
	}

	public static void setSuffix(byte[] bytes1, byte[] bytes2) {
		setSuffix(bytes1, bytes2, findMsbIndex(bytes2));
	}

	public static int increase(byte[] bytes) {
		for (int i = 0; i < bytes.length; i++) {
			if (bytes[i] != (byte) 0xFF) {
				bytes[i]++;
				return 0;
			}

			bytes[i] = 0;
		}

		return 1;
	}

	public static int decrease(byte[] bytes) {
		for (int i = 0; i < bytes.length; i++) {
			if (bytes[i] != 0) {
				bytes[i]--;
				return 0;
			}

			bytes[i] = (byte) 0xFF;
		}

		return -1;
	}

	public static long add(byte[] bytes, long value) {
		for (int i = 0; i < bytes.length && value != 0; i++) {
			value += bytes[i] & 0xFF;
			bytes[i] = (byte) value;
			value >>= 8;
		}

		return value;
	}

	public static long sub(byte[] bytes, long value) {
		return add(bytes, -value);
	}

	public static long mul(byte[] bytes, int lsb, long value) {
		long carry = 0;

		for (int i = lsb; i < bytes.length; i++) {
			carry += (bytes[i] & 0xFF) * value;
			bytes[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public static long mul(byte[] bytes, long value) {
		return mul(bytes, findLsbIndex(bytes), value);
	}

	public static long div(byte[] bytes, int msb, long value) {
		long remain = 0;

		for (int i = msb; i >= 0; i--) {
			remain = (remain << 8) | (bytes[i] & 0xFF);
			bytes[i] = (byte) (remain / value);
			remain %= value;
		}

		return remain;
	}

	public static long div(byte[] bytes, long value) {
		return div(bytes, findMsbIndex(bytes), value);
	}

	public static int add(byte[] bytes1, byte[] bytes2) {
		int carry = 0;

		for (int i = 0, j = 0; i < bytes1.length; i++) {
			if (j < bytes2.length) {
				carry += bytes2[j++] & 0xFF;
			} else if (carry == 0) {
				return 0;
			}

			carry += bytes1[i] & 0xFF;
			bytes1[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public static int sub(byte[] bytes1, byte[] bytes2) {
		int carry = 0;

		for (int i = 0, j = 0; i < bytes1.length; i++) {
			if (j < bytes2.length) {
				carry -= bytes2[j++] & 0xFF;
			} else if (carry == 0) {
				return 0;
			}

			carry += bytes1[i] & 0xFF;
			bytes1[i] = (byte) carry;
			carry >>= 8;
		}

		return carry;
	}

	public static byte[] mul(byte[] bytes1, int msb1, byte[] bytes2, int msb2) {
		byte[] bytes = new byte[msb1 + msb2 + 2];

		for (int i = 0; i <= msb1; i++) {
			int value = bytes1[i] & 0xFF;
			int carry = 0;
			int k = i;

			for (int j = 0; j <= msb2; j++, k++) {
				carry += (bytes[k] & 0xFF) + (bytes2[j] & 0xFF) * value;
				bytes[k] = (byte) carry;
				carry >>= 8;
			}

			bytes[k] = (byte) carry;
		}

		return bytes;
	}

	public static byte[] mul(byte[] bytes1, byte[] bytes2) {
		return mul(bytes1, findMsbIndex(bytes1), bytes2, findMsbIndex(bytes2));
	}

	// ============================================================

	protected byte[] mBytes;

	public CavanLargeValue(byte[] bytes) {
		mBytes = bytes;
	}

	public CavanLargeValue(int length) {
		this(new byte[length]);
	}

	public CavanLargeValue(CavanLargeValue value) {
		this(value.getBytes().clone());
	}

	// ============================================================

	public int findMsbIndex(int msb) {
		return findMsbIndex(mBytes, msb);
	}

	public int findMsbIndex() {
		return findMsbIndex(mBytes);
	}

	public int findLsbIndex(int lsb) {
		return findLsbIndex(mBytes, lsb);
	}

	public int findLsbIndex() {
		return findLsbIndex(mBytes);
	}

	public void clear() {
		clear(mBytes);
	}

	public void setBytes(byte[] bytes, int msb) {
		mBytes = bytes;
	}

	public void setBytes(byte[] bytes) {
		setBytes(bytes, bytes.length - 1);
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public byte getByte(int index) {
		return mBytes[index];
	}

	public void setByte(int index, byte value) {
		mBytes[index] = value;
	}

	public byte getFirstByte() {
		return mBytes[0];
	}

	public byte getLastByte() {
		return mBytes[mBytes.length - 1];
	}

	public int length() {
		return mBytes.length;
	}

	public void setLength(int length) {
		if (length == mBytes.length) {
			return;
		}

		byte[] bytes = new byte[length];

		if (length > mBytes.length) {
			length = mBytes.length;
			clear(bytes, length);
		}

		System.arraycopy(mBytes, 0, bytes, 0, length);
		mBytes = bytes;
	}

	public boolean isZero() {
		return isZero(mBytes);
	}

	public boolean notZero() {
		return notZero(mBytes);
	}

	public boolean isNegative() {
		return isNegative(mBytes);
	}

	public boolean isPositive() {
		return isPositive(mBytes);
	}

	public boolean startsWith(byte[] bytes) {
		return startsWith(mBytes, bytes);
	}

	public boolean startsWith(CavanLargeValue value) {
		return startsWith(value.getBytes());
	}

	public void setPrefix(byte[] bytes) {
		setPrefix(mBytes, bytes);
	}

	public void setPrefix(CavanLargeValue value) {
		setPrefix(value.getBytes());
	}

	public boolean endsWith(byte[] bytes) {
		return endsWith(mBytes, bytes);
	}

	public boolean endsWith(CavanLargeValue value) {
		return endsWith(value.getBytes());
	}

	public void setSuffix(byte[] bytes) {
		setSuffix(mBytes, bytes);
	}

	public void setSuffix(CavanLargeValue value) {
		setSuffix(value.getBytes());
	}

	// ============================================================

	public int increase() {
		return increase(mBytes);
	}

	public CavanLargeValue increaseLargeValue() {
		CavanLargeValue value = cloneLargeValue();
		value.increase();
		return value;
	}

	public int decrease() {
		return decrease(mBytes);
	}

	public CavanLargeValue decreaseLargeValue() {
		CavanLargeValue value = cloneLargeValue();
		value.decrease();
		return value;
	}

	public long add(long value) {
		return add(mBytes, value);
	}

	public static CavanLargeValue add(CavanLargeValue left, long value) {
		left = left.cloneLargeValue();
		left.add(value);
		return left;
	}

	public int add(byte[] bytes) {
		return add(mBytes, bytes);
	}

	public static CavanLargeValue add(CavanLargeValue left, byte[] bytes) {
		left = left.cloneLargeValue();
		left.add(bytes);
		return left;
	}

	public int add(CavanLargeValue value) {
		return add(value.getBytes());
	}

	public static CavanLargeValue add(CavanLargeValue left, CavanLargeValue right) {
		left = left.cloneLargeValue();
		left.add(right);
		return left;
	}

	public long sub(long value) {
		return add(-value);
	}

	public static CavanLargeValue sub(CavanLargeValue left, long value) {
		left = left.cloneLargeValue();
		left.sub(value);
		return left;
	}

	public int sub(byte[] bytes) {
		return sub(mBytes, bytes);
	}

	public static CavanLargeValue sub(CavanLargeValue left, byte[] bytes) {
		left = left.cloneLargeValue();
		left.sub(bytes);
		return left;
	}

	public int sub(CavanLargeValue value) {
		return sub(value.getBytes());
	}

	public static CavanLargeValue sub(CavanLargeValue left, CavanLargeValue right) {
		left = left.cloneLargeValue();
		left.sub(right);
		return left;
	}

	public long mul(long value) {
		return mul(mBytes, value);
	}

	public static CavanLargeValue mul(CavanLargeValue left, long value) {
		left = left.cloneLargeValue();
		left.mul(value);
		return left;
	}

	public byte[] mul(byte[] bytes) {
		return mul(mBytes, bytes);
	}

	public static CavanLargeValue mul(CavanLargeValue left, byte[] bytes) {
		return new CavanLargeValue(left.mul(bytes));
	}

	public CavanLargeValue mul(CavanLargeValue value) {
		return new CavanLargeValue(mul(value.getBytes()));
	}

	public static CavanLargeValue mul(CavanLargeValue left, CavanLargeValue right) {
		return left.mul(right);
	}

	public long div(long value) {
		return div(mBytes, value);
	}

	public static CavanLargeValue div(CavanLargeValue left, long value) {
		left = left.cloneLargeValue();
		left.div(value);
		return left;
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
		for (int i = 0; i < mBytes.length; i++) {
			if (index < end) {
				mBytes[i] = bytes[index++];
			} else {
				mBytes[i] = 0;
			}
		}

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
		for (int i = 0, j = end - 1; i < mBytes.length; i++) {
			if (j < index) {
				mBytes[i] = 0;
			} else {
				String text = texts[j--];

				if (text == null || text.isEmpty()) {
					mBytes[i] = 0;
				} else {
					try {
						mBytes[i] = (byte) Integer.parseInt(text, radix);
					} catch (NumberFormatException e) {
						mBytes[i] = 0;
					}
				}
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
		for (int i = 0, j = end - 1; i < mBytes.length; i++) {
			if (j < index) {
				mBytes[i] = 0;
			} else {
				mBytes[i] = (byte) values[j--];
			}
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

	public CavanLargeValue fromString(String text, char separator, int radix) {
		return fromStrings(radix, text.split("\\s*" + separator + "\\s*"));
	}

	public CavanLargeValue fromString(String text, char separator) {
		return fromString(text, separator, 16);
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
		CavanLargeValue value = cloneLargeValue();
		char[] chars = new char[length() * 8];
		int length;

		for (length = 0; value.notZero(); length++) {
			int remain = (int) value.div(radix);
			chars[length] = CavanString.convertValueToCharUppercase(remain);
		}

		if (length > 0) {
			CavanArray.reverse(chars, 0, length);
			return new String(chars, 0, length);
		}

		return "0";
	}

	public String[] toStrings(int count) {
		String[] strings = new String[count];
		int msb = findMsbIndex();
		int index = 0;

		for (int end = count - msb - 1; index < end; index++) {
			strings[index] = "00";
		}

		for (int i = msb; index < count; i--, index++) {
			strings[index] = CavanString.fromByte(mBytes[i]);
		}

		return strings;
	}

	public String[] toStrings() {
		return toStrings(mBytes.length);
	}

	public String[] toStringsWithRadix(int count, int radix) {
		String[] strings = new String[count];
		int msb = findMsbIndex();
		int index = 0;

		for (int end = count - msb - 1; index < end; index++) {
			strings[index] = "0";
		}

		for (int i = msb; index < count; i--, index++) {
			strings[index] = Integer.toString(mBytes[i] & 0xFF, radix);
		}

		return strings;
	}

	public String[] toStringsWithRadix(int radix) {
		return toStringsWithRadix(mBytes.length, radix);
	}

	public String[] toStringsWithRadix() {
		return toStringsWithRadix(10);
	}

	public String toStringWithRadix(char separator, int count, int radix) {
		StringBuilder builder = new StringBuilder();

		for (String text : toStringsWithRadix(count, radix)) {
			if (builder.length() > 0) {
				builder.append(separator);
			}

			builder.append(text);
		}

		return builder.toString();
	}

	public String toStringWithRadix(char separator, int count) {
		return toStringWithRadix(separator, count, 10);
	}

	public String toStringWithRadix(char separator) {
		return toStringWithRadix(separator, mBytes.length);
	}

	public String toString(char separator) {
		char[] chars = new char[mBytes.length * 3 - 1];

		for (int i = mBytes.length - 1, j = 0; i >= 0; i--, j += 2) {
			if (j > 0) {
				chars[j++] = separator;
			}

			CavanString.fromByte(chars, j, mBytes[i]);
		}

		return new String(chars);
	}

	@Override
	public String toString() {
		return toString(10);
	}

	// ============================================================

	public CavanLargeValue cloneLargeValue() {
		return new CavanLargeValue(this);
	}

	@Override
	public int compareTo(CavanLargeValue o) {
		return compare(mBytes, o.getBytes());
	}

	@Override
	public boolean equals(Object obj) {
		if (obj instanceof CavanLargeValue) {
			return compareTo((CavanLargeValue) obj) == 0;
		} else if (obj instanceof byte[]) {
			return compare(getBytes(), (byte[]) obj) == 0;
		}

		return false;
	}

	public static void main(String[] args) {
		CavanLargeValue a = new CavanLargeValue(8).fromString("12345678");
		CavanLargeValue b = new CavanLargeValue(8).fromString("10000");

		CavanJava.dLog("a = " + a);
		CavanJava.dLog("b = " + b);

		CavanJava.dLog("add = " + CavanLargeValue.add(a, 10000));
		CavanJava.dLog("add = " + CavanLargeValue.add(a, b));

		CavanJava.dLog("sub = " + CavanLargeValue.sub(a, 10000));
		CavanJava.dLog("sub = " + CavanLargeValue.sub(a, b));

		CavanJava.dLog("mul = " + CavanLargeValue.mul(a, 10000));
		CavanJava.dLog("mul = " + CavanLargeValue.mul(a, b));

		CavanJava.dLog("div = " + CavanLargeValue.div(a, 10000));

		a.fromStrings("11", "22", "33", "44", "55", "66");
		CavanJava.dLog("a = " + a.toStringHex());

		for (String text : a.toStrings(6)) {
			CavanJava.dLog("text = " + text);
		}

		a.fromStrings(10, "192", "168", "1", "1");
		CavanJava.dLog("a = " + a.toStringWithRadix('.', 4));
	}
}
