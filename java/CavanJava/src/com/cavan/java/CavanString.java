package com.cavan.java;

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CavanString {

	public static final String EMPTY_STRING = new String();
	public static final Pattern PATTERN_SPACE = Pattern.compile("\\s+");
	public static final char[] HEX_LOWERCASE_CHARS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	public static final char[] HEX_UPPERCASE_CHARS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	private static final String[] TRUE_TEXTS = {
		"1", "true", "on"
	};

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

	public static char convertToCharLowercase(int value) {
		return HEX_LOWERCASE_CHARS[value];
	}

	public static char convertToCharUppercase(int value) {
		return HEX_UPPERCASE_CHARS[value];
	}

	public static String fromByte(byte value) {
		return new String(new char[] { convertToCharUppercase((value >> 4) & 0x0F), convertToCharUppercase(value & 0x0F) } );
	}

	public static void fromByte(StringBuilder builder, byte value) {
		builder.append(convertToCharUppercase((value >> 4) & 0x0F)).append(convertToCharUppercase(value & 0x0F));
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

	public static int getLineCount(CharSequence text) {
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

	public static int findLineEnd(CharSequence text) {
		int length = text.length();

		for (int i = 0; i < length; i++) {
			if (text.charAt(i) == '\n') {
				return i;
			}
		}

		return length;
	}

	public static int lastIndexOf(String content, int end, int c) {
		int start = 0;
		int index = -1;

		while (true) {
			start = content.indexOf(c, start);
			if (start < 0 || start > end) {
				break;
			}

			index = start++;
		}

		return index;
	}

	public static boolean parseBoolean(String value) {
		for (String text : TRUE_TEXTS) {
			if (text.equalsIgnoreCase(value)) {
				return true;
			}
		}

		return false;
	}

	public static boolean isNumber(char c) {
		return c >= '0' && c <= '9';
	}

	public static boolean isNumberFloat(char c) {
		return isNumber(c) || c == '.';
	}

	public static boolean isNumber(char c, int radix) {
		if (radix <= 10) {
			return c >= '0' && c < ('0' + radix);
		}

		if (isNumber(c)) {
			return true;
		}

		if (c >= 'a' && c < ('a' + radix - 10)) {
			return true;
		}

		if (c >= 'A' && c < ('A' + radix - 10)) {
			return true;
		}

		return false;
	}

	public static boolean isNumber(String text) {
		for (int i = text.length() - 1; i >= 0; i--) {
			if (!isNumber(text.charAt(i))) {
				return false;
			}
		}

		return true;
	}

	public static boolean isNumber(String text, int radix) {
		for (int i = text.length() - 1; i >= 0; i--) {
			if (!isNumber(text.charAt(i), radix)) {
				return false;
			}
		}

		return true;
	}

	@Override
	public String toString() {
		return mContent;
	}
}
