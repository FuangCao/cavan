package com.cavan.java;

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CavanString {

	public static final String EMPTY_STRING = new String();
	public static final Pattern PATTERN_SPACE = Pattern.compile("\\s+");

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

	public static String fromArray(Object[] array) {
		StringBuilder builder = new StringBuilder("[ ");

		if (array != null && array.length > 0) {
			builder.append(array[0]);

			for (int i = 1; i < array.length; i++) {
				builder.append(" | ");
				builder.append(array[i]);
			}
		}

		builder.append(" ]");

		return builder.toString();
	}

	public static String deleteSpace(String text) {
		Matcher matcher = PATTERN_SPACE.matcher(text);

		return matcher.replaceAll(EMPTY_STRING);
	}

	public static String fromBdAddr(byte[] bytes) {
		return String.format("%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
	}

	public static byte[] parseBdAddr(String text) {
		String[] texts = text.split("\\s*[:\\-\\.]\\s*");
		if (texts.length != 6) {
			if (texts.length == 1 && text.length() == 12) {
				texts = new String[6];
				for (int i = 0; i < 12; i += 2) {
					texts[i / 2] = text.substring(i, i + 2);
				}
			} else {
				return null;
			}
		}

		byte[] bytes = new byte[6];

		try {
			for (int i = 0; i < 6; i++) {
				bytes[i] = (byte) Integer.parseInt(texts[i], 16);
			}
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		return bytes;
	}

	public static String fromCharSequence(CharSequence sequence) {
		if (sequence == null) {
			return EMPTY_STRING;
		}

		return sequence.toString();
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

	public static String getDigit(String text) {
		StringBuilder builder = new StringBuilder();

		for (int i = 0; i < text.length(); i++) {
			char c = text.charAt(i);

			if (CavanJava.isDigit(c)) {
				builder.append(c);
			}
		}

		return builder.toString();
	}

	public static boolean isColon(char c) {
		return c == ':' || c == '：';
	}

	public static int getLineCount(String text) {
		int length = text.length();

		if (length > 0) {
			int count = 1;

			for (int i = length - 1; i >= 0; i--) {
				if (text.charAt(i) == '\n') {
					count++;
				}
			}

			return count;
		} else {
			return 0;
		}
	}

	public static int findLineEnd(String text) {
		int index = text.indexOf('\n');
		if (index < 0) {
			return text.length();
		}

		return index;
	}

	@Override
	public String toString() {
		return mContent;
	}
}
