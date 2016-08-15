package com.cavan.java;

import java.util.List;

public class CavanString {

	private String mContent;

	public CavanString(String content) {
		super();
		mContent = content;
	}

	public CavanString(List<?> list) {
		this(fromList(list));
	}

	public String getContent() {
		return mContent;
	}

	public void setmContent(String content) {
		mContent = content;
	}

	public static boolean isChineseChar(char c) {
		return c >= 0x4E00 && c <= 0x9FA5;
	}

	public static boolean hasChineseChar(String text) {
		for (int i = text.length() - 1; i >= 0; i--) {
			if (isChineseChar(text.charAt(i))) {
				return true;
			}
		}

		return false;
	}

	public static String fixupSpace(String text) {
		return text.replace((char) 0xA0, (char) 0x20);
	}

	public static String strip(String text) {
		return fixupSpace(text).trim();
	}

	public static String join(Object[] array, String sep) {
		if (array.length > 1) {
			StringBuilder builder = new StringBuilder(array[0].toString());

			for (int i = 1; i < array.length; i++) {
				builder.append(sep);
				builder.append(array[i].toString());
			}

			return builder.toString();
		} else if (array.length > 0) {
			return array[0].toString();
		} else {
			return "";
		}
	}

	public static String join(List<?> list, String sep) {
		return join(list.toArray(), sep);
	}

	public static String fromList(List<?> list) {
		StringBuilder builder = new StringBuilder("[ ");

		if (list != null && list.size() > 0) {
			builder.append(list.get(0));

			for (int i = 1; i < list.size(); i++) {
				builder.append(" | ");
				builder.append(list.get(i));
			}
		}

		builder.append(" ]");

		return builder.toString();
	}

	public boolean hasChineseChar() {
		return hasChineseChar(mContent);
	}

	public String fixupSpace() {
		return fixupSpace(mContent);
	}

	public String strip() {
		return strip(mContent);
	}

	@Override
	public String toString() {
		return mContent;
	}
}
