package com.cavan.java;

public class CavanArray {

	public static void copy(byte[] src, int srcPos, byte[] dest, int destPos, int length) {
		System.arraycopy(src, srcPos, dest, destPos, length);
	}

	public static void copy(byte[] src, byte[] dest, int count) {
		copy(src, 0, dest, 0, count);
	}

	public static void copy(byte[] src, byte[] dest) {
		int length = Math.min(src.length, dest.length);
		copy(src, dest, length);
	}

	public static byte[] copy(byte[] bytes, int start, int count) {
		byte[] newBytes = new byte[count];
		copy(bytes, start, newBytes, 0, count);
		return newBytes;
	}

	public static byte[] copy(byte[] bytes, int count) {
		return copy(bytes, 0, count);
	}

	public static byte[] copy(byte[] bytes) {
		return copy(bytes, 0, bytes.length);
	}

	public static byte[] copySkip(byte[] bytes, int skip) {
		return copy(bytes, skip, bytes.length - skip);
	}

	public static int indexOf(Object[] objects, Object object) {
		if (objects == null) {
			return -1;
		}

		for (int i = objects.length - 1; i >= 0; i--) {
			if (objects[i].equals(object)) {
				return i;
			}
		}

		return -1;
	}

	public static boolean contains(Object[] objects, Object object) {
		if (objects == null) {
			return false;
		}

		for (Object o : objects) {
			if (o.equals(object)) {
				return true;
			}
		}

		return false;
	}

	public static byte[] clone(byte[] source) {
		byte[] bytes = new byte[source.length];
		System.arraycopy(source, 0, bytes, 0, source.length);
		return bytes;
	}

	public static short[] clone(short[] source) {
		short[] shorts = new short[source.length];
		System.arraycopy(source, 0, shorts, 0, source.length);
		return shorts;
	}

	public static int[] clone(int[] source) {
		int[] ints = new int[source.length];
		System.arraycopy(source, 0, ints, 0, source.length);
		return ints;
	}

	public static long[] clone(long[] source) {
		long[] longs = new long[source.length];
		System.arraycopy(source, 0, longs, 0, source.length);
		return longs;
	}

	public static void reverse(char[] array, int index, int end) {
		while (index < --end) {
			char temp = array[index];
			array[index] = array[end];
			array[end] = temp;
			index++;
		}
	}
}
