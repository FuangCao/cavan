package com.cavan.java;

public class CavanArray {

	public static void copy(byte[] src, int srcPos, byte[] dest, int destPos, int length) {
		System.arraycopy(src, srcPos, dest, destPos, length);
	}

	public static void copy(byte[] src, byte[] dest, int length) {
		copy(src, 0, dest, 0, length);
	}

	public static void copy(byte[] src, byte[] dest) {
		int length = Math.min(src.length, dest.length);
		copy(src, dest, length);
	}

	public static byte[] clone(byte[] bytes, int start, int length) {
		byte[] newBytes = new byte[length];
		copy(bytes, start, newBytes, 0, length);
		return newBytes;
	}

	public static byte[] clone(byte[] bytes, int start) {
		return clone(bytes, start, bytes.length - start);
	}

	public static byte[] clone(byte[] bytes) {
		return clone(bytes, 0, bytes.length);
	}

	public static byte[] cloneByLength(byte[] bytes, int length) {
		return clone(bytes, 0, length);
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

	public static void reverse(char[] array, int index, int end) {
		while (index < --end) {
			char temp = array[index];
			array[index] = array[end];
			array[end] = temp;
			index++;
		}
	}
}
