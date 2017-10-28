package com.cavan.java;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class RedPacketFinder {

	private static final int MIN_CODE_SIZE = 2;
	private static final int MAX_CODE_SIZE = 30;

	private static final String COLON = "：:";
	private static final String SUFFIX = "是为";
	private static final String SUFFIX_PATTERN = "\\s*[" + SUFFIX + "]";
	private static final String COLON_PATTERN = SUFFIX_PATTERN + "?\\s*[" + COLON + "]";
	private static final String COLON_PATTERN_LESS = COLON_PATTERN + "*\\s*";
	private static final String COLON_PATTERN_MORE = COLON_PATTERN + "+\\s*";
	private static final String SEPARATOR = "～~\\-_+=\\s";
	private static final String NORMAL_PATTERN = "(\\w+红包)";
	private static final String DIGIT_PATTERN = "([\\d" + SEPARATOR + "]+)";
	private static final String DIGIT_MULTI_LINE_PATTERN = "((?:[" + SEPARATOR + "]*\\D?[" + SEPARATOR + "]*\\d)+)";
	private static final String WORD_PATTERN = "(\\w+)";
	private static final String WORD_MULTI_LINE_PATTERN = "([\\w\\s]+)";

	private static final Pattern PATTERN_URL = Pattern.compile("[a-z]+://\\S+", Pattern.CASE_INSENSITIVE);
	private static final Pattern PATTERN_FU_DAI = Pattern.compile("\\[链接\\].*福袋");
	private static final Pattern PATTERN_THANKS = Pattern.compile("^@(.+)谢谢(.*)$");

	private static final Pattern[] sNormalPatterns = {
		Pattern.compile("^\\[" + NORMAL_PATTERN + "\\]"),
		Pattern.compile("^【" + NORMAL_PATTERN + "】"),
	};

	private static final String[] sInvalidCodes = {
		"感谢亲们长期以来的支持与信任", "领券下单二合一", "复制这条消息", "晒单奖励"
	};

	private static final String[] sExcludePredicts = {
		"严禁使用红包", "红包不返现", "红包专员", "预告", "早知道", "优惠券"
	};

	private static final String[] sExcludePictures = {
		"不要使用支付宝红包", "购物清单", "促销清单"
	};

	public static final Pattern[] sExcludePatterns = {
		PATTERN_URL,
		Pattern.compile("=\\d{8,}"),
	};

	public static final String[] sExcludeWords = {
		"领取方法", "红牛口令", "下单口令", "新口令", "淘口令", "群里输入口令", "语音口令", "语言口令"
	};

	public static final Pattern[] sUnsafePatterns = {
		Pattern.compile("Q\\s*Q", Pattern.CASE_INSENSITIVE),
		Pattern.compile("\\d+\\W*w", Pattern.CASE_INSENSITIVE),
	};

	public static final String[] sUnsafeWords = {
		"扣扣", "群", "手机", "电话", "微信", "号码", "联系", "客服", "咨询", "功", "价", "元", "好友", "机器人", "查找", "加", "点", "时", "年"
	};

	private static final Pattern[] sPicturePatterns = {
		Pattern.compile("支\\s*付\\s*宝\\s*红\\s*包"),
		Pattern.compile("支\\s*付\\s*宝\\s*口\\s*令"),
		Pattern.compile("红\\s*包\\s*口\\s*令"),
		Pattern.compile("口\\s*令\\s*红\\s*包"),
	};

	private static final Pattern[] sPredictPatterns = {
		Pattern.compile("准备.*红包"),
		Pattern.compile("红包.*准备"),
		Pattern.compile("突袭.*红包"),
		Pattern.compile("红包.*突袭"),
		Pattern.compile("提前.*红包"),
		Pattern.compile("红包.*提前"),
		Pattern.compile("马上.*红包"),
		Pattern.compile("红包.*马上"),
		Pattern.compile("红包.*空间"),
		Pattern.compile("红包.*说说"),
		Pattern.compile("红包.*朋友圈"),
	};

	private static final Pattern[] sDigitPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包[\\s\\d]*" + COLON_PATTERN_LESS + DIGIT_PATTERN),
		Pattern.compile("口\\s*令[\\s\\d]*" + COLON_PATTERN_LESS + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*\\w?" + COLON_PATTERN_MORE + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*\\w?" + COLON_PATTERN_MORE + DIGIT_PATTERN),
		Pattern.compile("\\b" + DIGIT_PATTERN + "走起"),
		Pattern.compile("\\b" + DIGIT_PATTERN + "go", Pattern.CASE_INSENSITIVE),
	};

	private static final Pattern[] sMultiLineDigitPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包.*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令.*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令.*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包.*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包[\\s\\d]*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令[\\s\\d]*" + COLON_PATTERN_MORE + DIGIT_MULTI_LINE_PATTERN),
		Pattern.compile("^" + DIGIT_PATTERN + "$"),
	};

	private static final Pattern[] sUnsafeDigitPatterns = {
		Pattern.compile(COLON_PATTERN_MORE + DIGIT_PATTERN),
		Pattern.compile("\\b" + DIGIT_PATTERN + "$"),
	};

	private static final Pattern[] sWordPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("中\\s*文\\s*口\\s*令[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("中\\s*文\\s*红\\s*包[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令[\\s\\d" + COLON + "]*【" + WORD_PATTERN + "】"),
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包[\\s\\d" + COLON + "]*【" + WORD_PATTERN + "】"),
		Pattern.compile("红\\s*包[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN + "\\s*$"),
		Pattern.compile("口\\s*令[\\s\\d]*" + COLON_PATTERN_MORE + WORD_PATTERN + "\\s*$"),
		Pattern.compile("红\\s*包\\s*\\w?" + COLON_PATTERN_MORE + WORD_PATTERN + "\\s*$"),
		Pattern.compile("口\\s*令\\s*\\w?" + COLON_PATTERN_MORE + WORD_PATTERN + "\\s*$"),
	};

	private static final Pattern[] sMultiLineWordPatterns = {
		Pattern.compile("支\\s*付\\s*宝\\s*红\\s*包" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("支\\s*付\\s*宝\\s*口\\s*令" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("中\\s*文\\s*口\\s*令" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("中\\s*文\\s*红\\s*包" + COLON_PATTERN_MORE + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*为\\b\\s*" + "[" + COLON + "]*" + WORD_MULTI_LINE_PATTERN),
		Pattern.compile("口\\s*令\\s*是\\s*" + "[" + COLON + "]+" + WORD_MULTI_LINE_PATTERN),
	};

	public static HashMap<String, String> sPackageCodeMap = new HashMap<String, String>();

	static {
		sPackageCodeMap.put("com.tencent.mobileqq", "QQ");
		sPackageCodeMap.put("com.tencent.mm", "微信");
	}

	private String mJoinedLines = CavanString.EMPTY_STRING;
	private ArrayList<String> mLines = new ArrayList<String>();
	private ArrayList<String> mSafeLines = new ArrayList<String>();

	public void split(String content) {
		if (content == null) {
			return;
		}

		StringBuilder builder = new StringBuilder(mJoinedLines);

		for (String line : content.split("\n")) {
			if (isExcludeLine(line)) {
				continue;
			}

			for (Pattern pattern : sExcludePatterns) {
				Matcher matcher = pattern.matcher(line);
				line = matcher.replaceAll(CavanString.EMPTY_STRING);
			}

			line = CavanString.strip(line);
			mLines.add(line);

			if (isSafeLine(line)) {
				mSafeLines.add(line);
			}

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

		if (isSafeLine(line)) {
			mSafeLines.add(line);
		}

		if (mJoinedLines.length() > 0) {
			mJoinedLines += ' ';
		}

		mJoinedLines += line;
	}

	public static boolean isInvalidCode(String code) {
		int length = code.length();

		if (length < MIN_CODE_SIZE || length > MAX_CODE_SIZE) {
			return true;
		}

		for (String value : sInvalidCodes) {
			if (value.equals(code)) {
				return true;
			}
		}

		return false;
	}

	public static boolean isExcludeLine(String line) {
		for (String word : sExcludeWords) {
			if (line.contains(word)) {
				return true;
			}
		}

		return false;
	}

	public static boolean isSafeLine(String line) {
		for (String word : sUnsafeWords) {
			if (line.contains(word)) {
				return false;
			}
		}

		for (Pattern pattern : sUnsafePatterns) {
			Matcher matcher = pattern.matcher(line);
			if (matcher.find()) {
				return false;
			}
		}

		return true;
	}

	public static boolean isRedPacketWordCode(List<String> codes, String text) {
		char prev = 0;
		int same_count = 0;
		int number_count = 0;
		int chinese_count = 0;
		boolean invalid = true;
		int length = text.length();
		StringBuilder builder = new StringBuilder();

		for (int i = 0; i < length; i++) {
			char c = text.charAt(i);

			if (CavanJava.isDigit(c)) {
				if (i == 0) {
					return false;
				}

				if (++number_count > 6) {
					return false;
				}

				builder.append(c);
				chinese_count = 0;
			} else {
				number_count = 0;

				if (CavanString.isChineseChar(c)) {
					if (++chinese_count > 1) {
						invalid = false;
					}
				} else {
					chinese_count = 0;
				}
			}

			if (c == prev) {
				if (++same_count > 4) {
					return false;
				}
			} else {
				same_count = 0;
			}

			prev = c;
		}

		if (invalid) {
			return false;
		}

		if (builder.length() > 0) {
			String numStr = builder.toString();

			for (String code : codes) {
				if (code.startsWith(numStr)) {
					return false;
				}
			}
		}

		return true;
	}

	public static boolean containsUrl(CharSequence text) {
		Matcher matcher = PATTERN_URL.matcher(text);
		return matcher.find();
	}

	private List<String> getRedPacketCodes(String line, Pattern[] patterns, List<String> codes, boolean strip, boolean unsafe) {
		for (Pattern pattern : patterns) {
			Matcher matcher = pattern.matcher(line);

			while (matcher.find()) {
				if (patterns == sMultiLineWordPatterns) {
					int end = matcher.end();
					if (end < line.length() && CavanString.isColon(line.charAt(end))) {
						continue;
					}
				} else if (unsafe) {
					if (!isSafeLine(matcher.group())) {
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

				// CavanAndroid.eLog("line = " + line);
				// CavanAndroid.eLog("code = " + code + ", pattern = " + pattern.pattern());

				codes.add(code);
			}
		}

		return codes;
	}

	private List<String> getRedPacketCodes(String line, Pattern[] patterns, boolean strip, boolean unsafe) {
		return getRedPacketCodes(line, patterns, new ArrayList<String>(), strip, unsafe);
	}

	private List<String> getRedPacketCodes(List<String> lines, Pattern[] patterns, List<String> codes, boolean strip, boolean unsafe) {
		for (String line : lines) {
			getRedPacketCodes(line, patterns, codes, strip, unsafe);
		}

		return codes;
	}

	private List<String> getRedPacketCodes(List<String> lines, Pattern[] patterns, boolean strip, boolean unsafe) {
		return getRedPacketCodes(lines, patterns, new ArrayList<String>(), strip, unsafe);
	}

	private List<String> getRedPacketWordCodes() {
		List<String> codes = new ArrayList<String>();
		getRedPacketCodes(mLines, sWordPatterns, codes, true, false);
		getRedPacketCodes(mJoinedLines, sMultiLineWordPatterns, codes, false, false);
		return codes;
	}

	public static boolean addRedPacketCode(List<String> codes, String code) {
		if (isInvalidCode(code)) {
			return false;
		}

		for (String a : codes) {
			if (code.indexOf(a) >= 0) {
				return false;
			}
		}

		codes.add(code);

		return true;
	}

	public static boolean isValidDigitCode(String code, boolean unsafe) {
		if (unsafe && code.startsWith("201") && code.charAt(3) >= '6') {
			return false;
		}

		for (int i = code.length() - 1; i > 0; i--) {
			if (code.charAt(i) != code.charAt(i - 1)) {
				return true;
			}
		}

		return false;
	}

	public static void addRedPacketDigitCodes(List<String> codes, String text, boolean unsafe) {
		for (int end = 8; end <= text.length(); end += 8) {
			String code = text.substring(end - 8, end);
			if (isValidDigitCode(code, unsafe)) {
				addRedPacketCode(codes, code);
			}
		}
	}

	public static void addRedPacketDigitCodes(List<String> codes, List<String> texts, boolean unsafe) {
		for (String text : texts) {
			String code = getRedPacketDigitCode(text, true);
			if (code != null && code.length() % 8 == 0) {
				addRedPacketDigitCodes(codes, code, unsafe);
			}
		}
	}

	public static List<String> addRedPacketDigitCodes(String text) {
		List<String> codes = new ArrayList<String>();
		addRedPacketDigitCodes(codes, text, false);
		return codes;
	}

	public static String[] splitRedPacketDigitCodes(String text) {
		List<String> list = new ArrayList<String>();
		addRedPacketDigitCodes(list, text, false);

		String[] codes = new String[list.size()];
		list.toArray(codes);

		return codes;
	}

	public static String getRedPacketDigitCode(String text, boolean unsafe) {
		StringBuilder builder = new StringBuilder();
		int length = text.length();

		for (int i = 0; i < length; i++) {
			char c = text.charAt(i);

			if (CavanString.isColon(c)) {
				builder.setLength(builder.length() & (~7));
			} else if (CavanJava.isDigit(c)) {
				if (unsafe && builder.length() % 8 == 0 && i > 0 && CavanJava.isDigit(text.charAt(i - 1))) {
					return null;
				}

				builder.append(c);
			}
		}

		return builder.toString();
	}

	public List<String> getRedPacketCodes() {
		List<String> codes = new ArrayList<String>();

		addRedPacketDigitCodes(codes, getRedPacketCodes(mLines, sDigitPatterns, false, true), false);
		addRedPacketDigitCodes(codes, getRedPacketCodes(mSafeLines, sUnsafeDigitPatterns, false, false), true);
		addRedPacketDigitCodes(codes, getRedPacketCodes(mJoinedLines, sMultiLineDigitPatterns, false, true), false);

		for (String code : getRedPacketWordCodes()) {
			if (isRedPacketWordCode(codes, code)) {
				addRedPacketCode(codes, code);
			}
		}

		return codes;
	}

	private boolean isPictureCode() {
		if (mJoinedLines.contains("[图片]")) {
			for (Pattern pattern : sPicturePatterns) {
				Matcher matcher = pattern.matcher(mJoinedLines);
				if (matcher.find()) {
					return true;
				}
			}
		}

		return false;
	}

	private boolean isPredictCode(String line) {
		for (Pattern pattern : sPredictPatterns) {
			Matcher matcher = pattern.matcher(line);
			if (matcher.find()) {
				return true;
			}
		}

		return false;
	}

	private boolean isPredictCode() {
		for (String line : mLines) {
			if (isPredictCode(line)) {
				return true;
			}
		}

		return false;
	}

	public String getPredictCode() {
		for (String word : sExcludePictures) {
			if (mJoinedLines.contains(word)) {
				return null;
			}
		}

		if (isPictureCode()) {
			return "图片";
		}

		for (String word : sExcludePredicts) {
			if (mJoinedLines.contains(word)) {
				return null;
			}
		}

		if (isPredictCode()) {
			return "准备";
		}

		return null;
	}

	public String getNormalCode(String pkgName, boolean fudai) {
		if (fudai) {
			Matcher matcher = PATTERN_FU_DAI.matcher(mJoinedLines);
			if (matcher.find()) {
				return "福袋";
			}
		}

		String code = sPackageCodeMap.get(pkgName);
		if (code != null) {
			if (mLines.size() != 1) {
				return null;
			}

			String line = mLines.get(0);
			if (line.length() > 100) {
				return null;
			}

			if (line.startsWith("[" + code + "红包]")) {
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

	public String getThanks()
	{
		if (mLines.size() != 1) {
			return null;
		}

		Matcher matcher = PATTERN_THANKS.matcher(mLines.get(0));
		if (matcher.find()) {
			String name = matcher.group(2).trim();
			if (name.isEmpty()) {
				name = matcher.group(1).trim();
			}

			return name;
		}

		return null;
	}

	public ArrayList<String> getLines() {
		return mLines;
	}

	public String getJoinedLines() {
		return mJoinedLines;
	}

	public boolean contains(String text) {
		return mJoinedLines.contains(text);
	}
}
