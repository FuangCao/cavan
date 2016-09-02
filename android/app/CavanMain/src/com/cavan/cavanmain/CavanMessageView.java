package com.cavan.cavanmain;

import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.method.LinkMovementMethod;
import android.text.style.BackgroundColorSpan;
import android.text.util.Linkify;
import android.text.util.Linkify.TransformFilter;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;

public class CavanMessageView extends LinearLayout implements OnClickListener {

	public static final String SCHEME = "cavan://";
	public static final String ACTION_ALIPAY = "alipay";
	public static final String ACTION_OPEN = "open";

	public static final int HIGH_LIGHT_COLOR = Color.YELLOW;

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

	private Activity mActivity;
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

	public static CavanMessageView getInstance(Activity activity) {
		CavanMessageView view = (CavanMessageView) inflate(activity, R.layout.message_item, null);
		view.setActivity(activity);

		return view;
	}

	public void setActivity(Activity activity) {
		mActivity = activity;
	}

	public void setTitle(CharSequence text) {
		mTextViewTitle.setText(text);
	}

	public void setContent(String text, String pkgName, Pattern[] patterns) {
		SpannableStringBuilder builder = new SpannableStringBuilder(text);

		Linkify.addLinks(builder, Linkify.WEB_URLS);

		String scheme = buildScheme(pkgName, ACTION_ALIPAY);

		for (Pattern pattern : sAlipayPatterns) {
			Linkify.addLinks(builder, pattern, scheme, null, mTransformFilter);
		}

		scheme = buildScheme(pkgName, ACTION_OPEN);

		for (Pattern pattern : sRedPacketPatterns) {
			Linkify.addLinks(builder, pattern, scheme, null, mTransformFilter);
		}

		if (patterns != null) {
			for (Pattern pattern : patterns) {
				Matcher matcher = pattern.matcher(text);

				while (matcher.find()) {
					builder.setSpan(new BackgroundColorSpan(HIGH_LIGHT_COLOR), matcher.start(), matcher.end(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}
			}
		}

		mTextViewContent.setText(builder);
	}

	@Override
	protected void onFinishInflate() {
		mTextViewTitle = (TextView) findViewById(R.id.textViewTitle);

		mTextViewContent = (TextView) findViewById(R.id.textViewContent);
		mTextViewContent.setOnClickListener(this);
		mTextViewContent.setMovementMethod(LinkMovementMethod.getInstance());
	}

	@Override
	public void onClick(View v) {
		ContextDialog dialog = new ContextDialog();
		dialog.show(mActivity.getFragmentManager(), CavanAndroid.TAG);
	}

	public class ContextDialog extends DialogFragment implements android.content.DialogInterface.OnClickListener {

		private EditText mEditTextMessage;

		public void show(FragmentManager manager) {
			super.show(manager, CavanAndroid.TAG);
		}

		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {
			View view = mActivity.getLayoutInflater().inflate(R.layout.message_context, null);
			mEditTextMessage = (EditText) view.findViewById(R.id.editTextMessage);
			mEditTextMessage.setText(mTextViewContent.getText());

			AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);

			builder.setView(view);
			builder.setCancelable(false);
			builder.setPositiveButton(R.string.text_copy, this);
			builder.setNegativeButton(android.R.string.cancel, null);

			return builder.create();
		}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			switch (which) {
			case DialogInterface.BUTTON_POSITIVE:
				CharSequence text = mEditTextMessage.getText();
				int start = mEditTextMessage.getSelectionStart();
				int end = mEditTextMessage.getSelectionEnd();

				if (start < end) {
					text = text.subSequence(start, end);
				}

				CavanAndroid.postClipboardText(mActivity, text);
				break;
			}
		}
	}
}
