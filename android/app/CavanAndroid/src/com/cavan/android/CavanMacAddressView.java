package com.cavan.android;

import android.content.Context;
import android.text.InputFilter;
import android.text.Spanned;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.cavan.java.CavanMacAddress;
import com.cavan.java.CavanString;

public class CavanMacAddressView extends LinearLayout {

	private static int TEXT_MAX_LENGTH = 2;

	private CavanKeyboardView mKeyboardView;
	private CavanMacAddress mAddress = new CavanMacAddress();
	private CavanMacAddressEditText[] mEditTextValues = new CavanMacAddressEditText[6];

	public class CavanMacAddressEditText extends EditText implements InputFilter, OnFocusChangeListener {

		private int mIndex;

		public CavanMacAddressEditText(Context context, int index) {
			super(context);

			mIndex = index;
			setSelectAllOnFocus(true);
			setOnFocusChangeListener(this);
			setFilters(new InputFilter[] { this });
		}

		@Override
		public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
			for (int i = start; i < end; i++) {
				if (!CavanString.isNumber(source.charAt(i), 16)) {
					return CavanString.EMPTY_STRING;
				}
			}

			int sLen = end - start;
			int dLen = dend - dstart;

			if (sLen > dLen) {
				int length = dest.length() - dLen;
				if (length + sLen >= TEXT_MAX_LENGTH) {
					end = start + (TEXT_MAX_LENGTH - length);

					if (mIndex < mEditTextValues.length - 1) {
						mEditTextValues[mIndex + 1].requestFocus();
					}
				}
			}

			return source.subSequence(start, end).toString().toUpperCase();
		}

		@Override
		public void onFocusChange(View v, boolean hasFocus) {
			if (hasFocus && mKeyboardView != null) {
				mKeyboardView.setEditText(this);
			}
		}
	};

	public CavanMacAddressView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanMacAddressView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanMacAddressView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanMacAddressView(Context context) {
		super(context);
	}

	public void setKeyboardView(CavanKeyboardView view) {
		mKeyboardView = view;
	}

	public String[] getAddressTexts() {
		String[] texts = new String[mEditTextValues.length];

		for (int i = 0; i < mEditTextValues.length; i++) {
			EditText view = mEditTextValues[i];
			String text;

			if (view != null) {
				text = view.getText().toString();
				while (text.length() < 2) {
					text = "0" + text;
				}
			} else {
				text = "00";
			}

			texts[i] = text;
		}

		return texts;
	}

	public byte[] getAddressBytes() {
		String[] texts = getAddressTexts();
		if (texts == null) {
			return null;
		}

		byte[] bytes = new byte[texts.length];

		for (int i = 0; i < texts.length; i++) {
			bytes[i] = (byte) Integer.parseInt(texts[i], 16);
		}

		return bytes;
	}

	public CavanMacAddressEditText getMacAddressView(int index) {
		return mEditTextValues[index];
	}

	public CavanMacAddress getAddress() {
		return mAddress.parse(getAddressTexts());
	}

	@Override
	public String toString() {
		return CavanString.join(getAddressTexts(), ":");
	}

	@Override
	protected void onFinishInflate() {
		for (int i = 0; i < mEditTextValues.length; i++) {
			if (i > 0) {
				TextView textView = new TextView(getContext());
				textView.getPaint().setFakeBoldText(true);
				textView.setText(":");
				addView(textView);
			}

			CavanMacAddressEditText editText = new CavanMacAddressEditText(getContext(), i);
			mEditTextValues[i] = editText;

			addView(editText);
		}

		super.onFinishInflate();
	}
}
