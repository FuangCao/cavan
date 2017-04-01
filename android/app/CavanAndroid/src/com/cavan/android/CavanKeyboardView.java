package com.cavan.android;

import android.content.Context;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.text.Editable;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
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
			try {
				processText(text);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public void onRelease(int primaryCode) {}

		@Override
		public void onPress(int primaryCode) {}

		@Override
		public void onKey(int primaryCode, int[] keyCodes) {
			try {
				processKey(primaryCode, keyCodes);
			} catch (Exception e) {
				e.printStackTrace();
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

	public void processText(CharSequence text) {
		if (mEditText == null) {
			return;
		}

		int start = mEditText.getSelectionStart();
		int end = mEditText.getSelectionEnd();
		Editable editable = mEditText.getEditableText();
		int length = editable.length() + start - end;

		length = editable.replace(start, end, text).length() - length;
		mEditText.setSelection(start + length);
	}

	public void processKey(int primaryCode, int[] keyCodes) {
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

	public void setEditText(EditText view) {
		mEditText = view;
	}

	public EditText getEditText() {
		return mEditText;
	}

	public void setupEditText(EditText view, OnFocusChangeListener listener) {
		view.setInputType(InputType.TYPE_NULL);
		view.setSelectAllOnFocus(true);
		view.setOnFocusChangeListener(listener);
		view.setOnLongClickListener(new OnLongClickListener() {

			@Override
			public boolean onLongClick(View view) {
				((EditText) view).getEditableText().clear();
				return true;
			}
		});
	}

	public void setupEditText(EditText view) {
		setupEditText(view, new OnFocusChangeListener() {

			@Override
			public void onFocusChange(View view, boolean hasFocus) {
				setEditText((EditText) view);
			}
		});
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
