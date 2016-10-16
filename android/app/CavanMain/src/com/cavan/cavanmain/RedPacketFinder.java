package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class RedPacketFinder {

	private static final String NORMAL_PATTERN = "(\\w+红包)";
	private static final String DIGIT_PATTERN = "([\\d\\s]+)";
	private static final String DIGIT_MULTI_LINE_PATTERN = "((?:\\D*\\d)+)";
	private static final String WORD_PATTERN = "(\\w+)";
	private static final String WORD_MULTI_LINE_PATTERN = "([\\w\\s]+)";

	private static final Pattern[] sNormalPatterns = {
		Pattern.compile("^\\[" + NORMAL_PATTERN + "\\]"),
		Pattern.compile("^【" + NORMAL_PATTERN + "】"),
	};

	private static final Pattern[] sPicturePatterns = {
		Pattern.compile("支\\s*付\\s*宝\\s*红\\s*包"),
		Pattern.compile("支\\s*付\\s*宝\\s*口\\s*令"),
		Pattern.compile("红\\s*包\\s*口\\s*令"),
		Pattern.compile("口\\s*令\\s*红\\s*包"),
	};

	private static final Pattern[] sPredictPatterns = {
		Pattern.compile("准\\s*备.*红\\s*包"),
	};

	private static final Pattern[] sDigitPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("[:：]\\s*" + DIGIT_PATTERN),
		Pattern.compile("\\b" + DIGIT_PATTERN + "\\s*$"),
	};

	private static final Pattern[] sMultiLineDigitPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包" + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令" + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令" + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包" + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包\\s*[:：]" + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*[:：]" + DIGIT_MULTI_LINE_PATTERN),
	};

	private static final Pattern[] sWordPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("中\\s*文\\s*口\\s*令[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("中\\s*文\\s*红\\s*包[\\s\\d]*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令[\\s\\d:：]*【" + WORD_PATTERN + "】"),
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包[\\s\\d:：]*【" + WORD_PATTERN + "】"),
		Pattern.compile("红\\s*包[\\s\\d]*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
		Pattern.compile("口\\s*令[\\s\\d]*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
	};

	private static final Pattern[] sMultiLineWordPatterns = {
		Pattern.compile("支\\s*付\\s*宝\\s*红\\s*包\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("支\\s*付\\s*宝\\s*口\\s*令\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("中\\s*文\\s*口\\s*令\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("中\\s*文\\s*红\\s*包\\s*[:：]\\s*" + WORD_MULTI_LINE_PATTERN),
	};

	public static final Pattern[] sExcludePatterns = {
		Pattern.compile("[a-z]+://\\S+", Pattern.CASE_INSENSITIVE),
		Pattern.compile("(?:(?:Q\\s*Q)|群|(?:手\\s*机)|(?:电\\s*话)|(?:微\\s*信)|码|号)(?:(?:[\\W\\d]*[:：])|(?:\\W*[:：]?))\\s*\\d+", Pattern.CASE_INSENSITIVE),
		Pattern.compile("领\\s*取\\s*方\\s*法\\s*[:：].*$"),
		Pattern.compile("领\\s*取\\s*方\\s*法.*口\\s*令"),
	};

	public static HashMap<String, String> sPackageCodeMap = new HashMap<String, String>();

	static {
		sPackageCodeMap.put("com.tencent.mobileqq", "QQ红包");
		sPackageCodeMap.put("com.tencent.mm", "微信红包");
	}

	private String mJoinedLines = CavanString.EMPTY_STRING;
	private List<String> mLines = new ArrayList<String>();

	public void split(String content) {
		if (content == null) {
			return;
		}

		StringBuilder builder = new StringBuilder(mJoinedLines);

		for (String line : content.split("\n")) {
			for (Pattern pattern : sExcludePatterns) {
				Matcher matcher = pattern.matcher(line);
				line = matcher.replaceAll(CavanString.EMPTY_STRING);
			}

			line = CavanString.strip(line);
			mLines.add(line);

			if (builder.length() > 0) {
				builder.append(' ');
			}

			builder.append(line);
		}

		mJoinedLines = builder.toString();
	}

	public void addLine(String line) {
		line = CavanString.strip(line);
		mLines.add(line);

		if (mJoinedLines.length() > 0) {
			mJoinedLines += ' ';
		}

		mJoinedLines += line;
	}


	public static boolean isRedPacketWordCode(String code) {
		int number_count = 0;
		int chinese_count = 0;

		for (int i = code.length() - 1; i >= 0; i--) {
			char c = code.charAt(i);

			if (CavanJava.isDigit(c)) {
				if (++number_count > 6) {
					return false;
				}
			} else {
				number_count = 0;

				if (CavanString.isChineseChar(c)) {
					chinese_count++;
				}
			}
		}

		return (chinese_count > 0);
	}

	public List<String> getRedPacketCodes(String line, Pattern[] patterns, List<String> codes, boolean strip) {
		for (Pattern pattern : patterns) {
			Matcher matcher = pattern.matcher(line);

			while (matcher.find()) {
				if (patterns == sMultiLineWordPatterns) {
					int end = matcher.end();
					if (end < line.length() && CavanString.isColon(line.charAt(end))) {
						continue;
					}
				}

				String code = matcher.group(1);

				if (strip) {
					code = CavanString.deleteSpace(code);
				}

				if (code.length() < 2) {
					continue;
				}

				// CavanAndroid.eLog("code = " + code + ", pattern = " + pattern.pattern());

				codes.add(code);
			}
		}

		return codes;
	}

	public List<String> getRedPacketCodes(String line, Pattern[] patterns, boolean strip) {
		return getRedPacketCodes(line, patterns, new ArrayList<String>(), strip);
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, List<String> codes, boolean strip, boolean multiLine) {
		for (String line : mLines) {
			getRedPacketCodes(line, patterns, codes, strip);
		}

		if (multiLine) {
			getRedPacketCodes(mJoinedLines, patterns, codes, strip);
		}

		return codes;
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, boolean strip, boolean multiLine) {
		return getRedPacketCodes(patterns, new ArrayList<String>(), strip, multiLine);
	}

	public List<String> getRedPacketWordCodes() {
		List<String> codes = new ArrayList<String>();
		getRedPacketCodes(sWordPatterns, codes, true, false);
		getRedPacketCodes(mJoinedLines, sMultiLineWordPatterns, codes, false);
		return codes;
	}

	public boolean addRedPacketCode(List<String> codes, String code) {
		for (String a : codes) {
			if (code.indexOf(a) >= 0) {
				return false;
			}
		}

		codes.add(code);

		return true;
	}

	public void addRedPacketCodes(List<String> codes, String code) {
		for (int end = 8; end <= code.length(); end += 8) {
			addRedPacketCode(codes, code.substring(end - 8, end));
		}
	}

	public String getRedPacketDigitCode(String text) {
		StringBuilder builder = new StringBuilder();
		int length = text.length();

		for (int i = 0; i < length; i++) {
			char c = text.charAt(i);

			if (CavanString.isColon(c)) {
				builder.setLength(0);
			} else if (CavanJava.isDigit(c)) {
				if (builder.length() % 8 == 0 && i > 0 && CavanJava.isDigit(text.charAt(i - 1))) {
					return null;
				}

				builder.append(c);
			} else if (CavanString.isChineseChar(c)) {
				break;
			}
		}

		return builder.toString();
	}

	public List<String> getRedPacketCodes() {
		List<String> codes = new ArrayList<String>();

		for (String text : getRedPacketCodes(sDigitPatterns, false, true)) {
			String code = getRedPacketDigitCode(text);
			if (code != null && code.length() % 8 == 0) {
				addRedPacketCodes(codes, code);
			}
		}

		for (String text : getRedPacketCodes(mJoinedLines, sMultiLineDigitPatterns, false)) {
			String code = getRedPacketDigitCode(text);
			if (code != null) {
				addRedPacketCodes(codes, code);
			}
		}

		for (String code : getRedPacketWordCodes()) {
			if (isRedPacketWordCode(code)) {
				addRedPacketCode(codes, code);
			}
		}

		return codes;
	}

	public boolean isPictureCode() {
		if (!mJoinedLines.contains("[图片]")) {
			return false;
		}

		if (mJoinedLines.contains("不要使用支付宝红包")) {
			return false;
		}

		for (Pattern pattern : sPicturePatterns) {
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				return true;
			}
		}

		return false;
	}

	public boolean isPredictCode() {
		for (Pattern pattern : sPredictPatterns) {
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				return true;
			}
		}

		return false;
	}

	public String getNormalCode(String pkgName) {
		String code = sPackageCodeMap.get(pkgName);
		if (code != null) {
			if (mJoinedLines.startsWith("[" + code + "]")) {
				return code;
			}

			return null;
		}

		for (Pattern pattern : sNormalPatterns) {
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				return matcher.group(1);
			}
		}

		return null;

	}
}
