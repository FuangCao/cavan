package com.cavan.cavanmain;

import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.text.util.Linkify.TransformFilter;
import android.util.AttributeSet;
import android.widget.LinearLayout;
import android.widget.TextView;

public class CavanMessageView extends LinearLayout {

	public static final String SCHEME = "cavan://";
	public static final String ACTION_ALIPAY = "alipay";
	public static final String ACTION_OPEN = "open";

	private static HashMap<String, HashMap<String, String>> mSchemeMap = new HashMap<String, HashMap<String, String>>();

	public static final Pattern[] sAlipayPatterns = {
		Pattern.compile("\\b(\\d{8})\\b"),
		Pattern.compile("(红包|口令)\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("(红包|口令)\\s+(\\w+)"),
	};

	public static final Pattern[] sRedPacketPatterns = {
		Pattern.compile("\\[(\\w+红包)\\]"), Pattern.compile("【(\\w+红包)】"),
	};

	private static TransformFilter mTransformFilter = new TransformFilter() {

		@Override
		public String transformUrl(Matcher match, String url) {
			return match.group(match.groupCount());
		}
	};

	public static String buildScheme(String pkgName, String action) {
		HashMap<String, String> map = mSchemeMap.get(action);
		if (map == null) {
			map = new HashMap<String, String>();
			mSchemeMap.put(action, map);
		}

		String scheme = map.get(pkgName);
		if (scheme == null) {
			scheme = SCHEME + pkgName + "/" + action + "/";
			map.put(pkgName, scheme);
		}

		return scheme;
	}

	private TextView mTextViewTitle;
	private TextView mTextViewContent;

	public CavanMessageView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanMessageView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanMessageView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanMessageView(Context context) {
		super(context);
	}

	public void setTitle(CharSequence text) {
		mTextViewTitle.setText(text);
	}

	public void setContent(CharSequence text, String pkgName) {
		mTextViewContent.setText(text);

		Linkify.addLinks(mTextViewContent, Linkify.WEB_URLS);

		String scheme = buildScheme(pkgName, ACTION_ALIPAY);

		for (Pattern pattern : sAlipayPatterns) {
			Linkify.addLinks(mTextViewContent, pattern, scheme, null, mTransformFilter);
		}

		scheme = buildScheme(pkgName, ACTION_OPEN);

		for (Pattern pattern : sRedPacketPatterns) {
			Linkify.addLinks(mTextViewContent, pattern, scheme, null, mTransformFilter);
		}
	}

	@Override
	protected void onFinishInflate() {
		mTextViewTitle = (TextView) findViewById(R.id.textViewTitle);

		mTextViewContent = (TextView) findViewById(R.id.textViewContent);
		mTextViewContent.setMovementMethod(LinkMovementMethod.getInstance());
	}

	public static CavanMessageView getInstance(Context context) {
		return (CavanMessageView) inflate(context, R.layout.message_item, null);
	}
}
