package com.cavan.android;

import com.cavan.java.CavanString;

import android.content.Context;
import android.text.InputFilter;
import android.util.AttributeSet;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class CavanMacAddressView extends LinearLayout {

	private EditText[] mEditTextValues = new EditText[6];

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

			EditText editText = new EditText(getContext());
			editText.setFilters(new InputFilter[] { new InputFilter.LengthFilter(2)} );
			editText.setText(Integer.toString(i));
			mEditTextValues[i] = editText;
			addView(editText);
		}

		super.onFinishInflate();
	}
}
