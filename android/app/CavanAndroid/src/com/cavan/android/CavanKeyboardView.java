package com.cavan.android;

import java.util.HashMap;

import android.content.Context;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.text.Editable;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;

import com.cavan.android.CavanAndroidListeners.CavanKeyboardViewListener;

public abstract class CavanKeyboardView extends KeyboardView implements CavanKeyboardViewListener {

	public static final int KEYCODE_DELETE = -1;
	public static final int KEYCODE_CLEAR = -2;


	private EditText mEditText;
	private Keyboard[] mKeyboards;
	private int[] mKeyboardResources;

	private boolean mInputDisabled;
	private CavanKeyboardViewListener mListener = this;
	private HashMap<View, Integer> mKeyboardMap = new HashMap<View, Integer>();

	private OnKeyboardActionListener mActionListener = new OnKeyboardActionListener() {

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

	protected abstract int[] getKeyboardResources();

	@Override
	public void onStartInput(EditText view) {
		setVisibility(View.VISIBLE);
	}

	@Override
	public void onStopInput(EditText view) {
		setVisibility(View.GONE);
	}

	@Override
	public void onEditTextClick(EditText view) {
		startInput();
	}

	@Override
	public void onEditTextLongClick(EditText view) {
		view.getEditableText().clear();
	}

	public CavanKeyboardViewListener getEventListener() {
		return mListener;
	}

	public void setEventListener(CavanKeyboardViewListener listener) {
		if (listener != null) {
			mListener = listener;
		} else {
			mListener = this;
		}
	}

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
		if (mEditText != view) {
			mEditText = view;

			setKeyboard(getKeyboard(view));
			startInput();
		}
	}

	public EditText getEditText() {
		return mEditText;
	}

	private OnFocusChangeListener mEditTextFocusChangeListener = new OnFocusChangeListener() {

		@Override
		public void onFocusChange(View view, boolean hasFocus) {
			if (hasFocus) {
				setEditText((EditText) view);
			}
		}
	};

	private OnLongClickListener mEditTextLongClickListener = new OnLongClickListener() {

		@Override
		public boolean onLongClick(View view) {
			mListener.onEditTextLongClick((EditText) view);
			return true;
		}
	};

	private OnClickListener mEditTextClickListener = new OnClickListener() {

		@Override
		public void onClick(View view) {
			mListener.onEditTextClick((EditText) view);
		}
	};

	public void setupEditText(EditText view) {
		view.setInputType(InputType.TYPE_NULL);
		view.setSelectAllOnFocus(true);

		view.setOnFocusChangeListener(mEditTextFocusChangeListener);
		view.setOnLongClickListener(mEditTextLongClickListener);
		view.setOnClickListener(mEditTextClickListener);
	}

	public Keyboard getKeyboard(int index) {
		if (mKeyboards == null) {
			mKeyboardResources = getKeyboardResources();
			mKeyboards = new Keyboard[mKeyboardResources.length];
		}

		Keyboard keyboard = mKeyboards[index];
		if (keyboard != null) {
			return keyboard;
		}

		keyboard = new Keyboard(getContext(), mKeyboardResources[index]);
		mKeyboards[index] = keyboard;
		return keyboard;
	}

	public void setKeyboard(int index) {
		try {
			setKeyboard(getKeyboard(index));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public Keyboard getKeyboard(View view) {
		Integer index = mKeyboardMap.get(view);
		if (index == null) {
			index = 0;
		}

		return getKeyboard(index);
	}

	public void setKeyboard(View view, int keyboard) {
		mKeyboardMap.put(view, keyboard);
	}

	public void setKeyboard(EditText view) {
		try {
			setKeyboard(getKeyboard(view));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void startInput() {
		if (mInputDisabled || mEditText == null) {
			return;
		}

		mListener.onStartInput(mEditText);
	}

	public void stopInput() {
		if (mEditText != null) {
			mListener.onStopInput(mEditText);
		}
	}

	public boolean isInputDisabled() {
		return mInputDisabled;
	}

	public void setInputEnable(boolean enable, long delayMillis) {
		if (enable) {
			mInputDisabled = false;
		} else {
			mInputDisabled = true;
			stopInput();
		}

		if (delayMillis > 0) {
			postDelayed(new Runnable() {

				@Override
				public void run() {
					setInputEnable(mInputDisabled, 0);
				}
			}, delayMillis);
		}
	}

	public void setInputEnable(boolean enable) {
		setInputEnable(enable, 0);
	}

	@Override
	protected void onAttachedToWindow() {
		setEnabled(true);
		setPreviewEnabled(false);
		setOnKeyboardActionListener(mActionListener);

		super.onAttachedToWindow();
	}
}
