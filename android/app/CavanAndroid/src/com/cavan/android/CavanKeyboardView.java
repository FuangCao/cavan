package com.cavan.android;

import android.content.Context;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.util.AttributeSet;
import android.widget.EditText;

public abstract class CavanKeyboardView extends KeyboardView {

	public static final int KEYCODE_DELETE = 1;
	public static final int KEYCODE_CLEAR = 2;

	private EditText mEditText;

	private OnKeyboardActionListener mListener = new OnKeyboardActionListener() {

		@Override
		public void swipeUp() {
			CavanAndroid.dLog("swipeUp");
		}

		@Override
		public void swipeRight() {
			CavanAndroid.dLog("swipeRight");
		}

		@Override
		public void swipeLeft() {
			CavanAndroid.dLog("swipeLeft");
		}

		@Override
		public void swipeDown() {
			CavanAndroid.dLog("swipeDown");
		}

		@Override
		public void onText(CharSequence text) {
			CavanAndroid.dLog("onText: text = " + text);

			if (mEditText != null) {
				int start = mEditText.getSelectionStart();
				int end = mEditText.getSelectionEnd();

				mEditText.getEditableText().replace(start, end, text);

				try {
					mEditText.setSelection(start + text.length());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}

		@Override
		public void onRelease(int primaryCode) {
			CavanAndroid.dLog("onRelease: primaryCode = " + primaryCode);
		}

		@Override
		public void onPress(int primaryCode) {
			CavanAndroid.dLog("onPress: primaryCode = " + primaryCode);
		}

		@Override
		public void onKey(int primaryCode, int[] keyCodes) {
			CavanAndroid.dLog("onKey: primaryCode = " + primaryCode);

			switch (primaryCode) {
			case KEYCODE_DELETE:
				if (mEditText != null) {
					int start = mEditText.getSelectionStart();
					int end = mEditText.getSelectionEnd();

					if (start < end) {
						mEditText.getEditableText().delete(start, end);
					} else if (start > 0) {
						mEditText.getEditableText().delete(start - 1, start);
					}
				}
				break;

			case KEYCODE_CLEAR:
				if (mEditText != null) {
					mEditText.getEditableText().clear();
				}
				break;
			}
		}
	};

	public CavanKeyboardView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanKeyboardView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanKeyboardView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	protected abstract int getKeyboardResource();

	public void setEditText(EditText view) {
		mEditText = view;

		if (view != null) {
			// view.setInputType(InputType.TYPE_NULL);
		}
	}

	public EditText getEditText() {
		return mEditText;
	}

	@Override
	protected void onAttachedToWindow() {
		setKeyboard(new Keyboard(getContext(), getKeyboardResource()));

		setEnabled(true);
		setPreviewEnabled(false);
		setOnKeyboardActionListener(mListener);

		super.onAttachedToWindow();
	}
}
