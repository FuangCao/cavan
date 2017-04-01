package com.cavan.android;

import android.content.Context;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.cavan.java.CavanMacAddress;
import com.cavan.java.CavanString;

public class CavanMacAddressView extends LinearLayout {

	private static int TEXT_MAX_LENGTH = 2;

	private TextWatcher mWatcher;
	private CavanKeyboardView mKeyboardView;
	private CavanMacAddress mAddress = new CavanMacAddress();
	private CavanMacAddressSubView[] mSubViews = new CavanMacAddressSubView[6];

	public class CavanMacAddressSubView extends EditText implements InputFilter, OnFocusChangeListener {

		private int mIndex;

		private Runnable mRunnableGotoNextView = new Runnable() {

			@Override
			public void run() {
				gotoNextView();
			}
		};

		public CavanMacAddressSubView(Context context, int index) {
			super(context);

			mIndex = index;
			setInputType(InputType.TYPE_NULL);

			setMinEms(1);
			setText("00");
			setSelectAllOnFocus(true);
			setOnFocusChangeListener(this);
			setFilters(new InputFilter[] { this });

			if (mWatcher != null) {
				addTextChangedListener(mWatcher);
			}
		}

		public String getString() {
			return getText().toString();
		}

		public int length() {
			return getEditableText().length();
		}

		public boolean isEmpty() {
			return length() <= 0;
		}

		public void clear() {
			getEditableText().clear();
		}

		public CavanMacAddressSubView getNextView() {
			int index = mIndex + 1;
			if (index < mSubViews.length) {
				return mSubViews[index];
			}

			return null;
		}

		public boolean gotoNextView() {
			CavanMacAddressSubView view = getNextView();
			if (view != null) {
				view.requestFocus();
				return true;
			}

			selectAll();

			return false;
		}

		public CavanMacAddressSubView getPrevView() {
			if (mIndex > 0) {
				return mSubViews[mIndex - 1];
			}

			return null;
		}

		public boolean gotoPrevView() {
			CavanMacAddressSubView view = getPrevView();
			if (view != null) {
				view.requestFocus();
				return true;
			}

			selectAll();

			return true;
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

					if (isFocused()) {
						postDelayed(mRunnableGotoNextView, 100);
					}
				}
			}

			return source.subSequence(start, end).toString().toUpperCase();
		}

		@Override
		public void onFocusChange(View v, boolean hasFocus) {
			if (hasFocus) {
				if (mKeyboardView != null) {
					mKeyboardView.setEditText(this);
				}
			} else {
				Editable editable = getEditableText();
				while (editable.length() < TEXT_MAX_LENGTH) {
					editable.insert(0, "0");
				}
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

	public CavanMacAddressSubView getSubView(int index) {
		return mSubViews[index];
	}

	public CavanMacAddressSubView[] getSubViews(int index) {
		return mSubViews;
	}

	public String[] getTexts() {
		String[] texts = new String[mSubViews.length];

		for (int i = texts.length - 1; i >= 0; i--) {
			CavanMacAddressSubView view = mSubViews[i];
			if (view != null) {
				texts[i] = view.getString();
			} else {
				texts[i] = "0";
			}
		}

		return texts;
	}

	public CavanMacAddress getAddress() {
		return mAddress.parseStrings(getTexts());
	}

	public void clear() {
		for (CavanMacAddressSubView view : mSubViews) {
			if (view != null) {
				view.clear();
			}
		}

		if (mSubViews[0] != null) {
			mSubViews[0].requestFocus();
		}
	}

	public void addTextChangedListener(TextWatcher watcher) {
		mWatcher = watcher;

		for (CavanMacAddressSubView view : mSubViews) {
			if (view != null) {
				view.addTextChangedListener(watcher);
			}
		}
	}

	@Override
	public String toString() {
		CavanMacAddress address = getAddress();
		return address.toString();
	}

	@Override
	protected void onFinishInflate() {
		for (int i = 0; i < mSubViews.length; i++) {
			if (i > 0) {
				TextView textView = new TextView(getContext());
				textView.getPaint().setFakeBoldText(true);
				textView.setText(":");
				addView(textView);
			}

			CavanMacAddressSubView editText = new CavanMacAddressSubView(getContext(), i);
			mSubViews[i] = editText;
			addView(editText);
		}

		super.onFinishInflate();
	}
}
