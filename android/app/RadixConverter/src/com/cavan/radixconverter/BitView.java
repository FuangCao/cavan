package com.cavan.radixconverter;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class BitView extends RelativeLayout {

	private int mIndex;
	private BitAdapter mAdapter;

	private TextView mTextViewIndex;
	private TextView mTextViewTitle;

	public BitView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public BitView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public BitView(Context context) {
		super(context);
	}

	@Override
	protected void onFinishInflate() {
		mTextViewIndex = (TextView) findViewById(R.id.textViewIndex);
		mTextViewIndex.setText("0");

		mTextViewTitle = (TextView) findViewById(R.id.textViewTitle);
		mTextViewTitle.setText("0");

		setActive(false);

		super.onFinishInflate();
	}

	public void setup(BitAdapter adapter, int index, OnClickListener listener) {
		mAdapter = adapter;
		mIndex = index;

		mTextViewIndex.setText(Integer.toString(index));
		setOnClickListener(listener);
	}

	public BitAdapter getAdapter() {
		return mAdapter;
	}

	public int getIndex() {
		return mIndex;
	}

	public void setActive(boolean active) {
		if (active) {
			mTextViewIndex.setBackgroundColor(Color.RED);
			mTextViewTitle.setBackgroundColor(Color.MAGENTA);
		} else {
			mTextViewIndex.setBackgroundColor(Color.BLACK);
			mTextViewTitle.setBackgroundColor(Color.GRAY);
		}
	}

	public BitView getNextView() {
		if (mIndex > 0) {
			return mAdapter.getButton(mIndex - 1);
		} else {
			return null;
		}
	}

	public int parseText(CharSequence charSequence) {
		char c = charSequence.charAt(0);
		if (c >= '0' && c <= '9') {
			return c - '0';
		} else if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		} else if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}

		return -1;
	}

	public int getValue() {
		return parseText(mTextViewTitle.getText());
	}

	public void setValue(int value) {
		mTextViewTitle.setText(KeypadAdapter.VALUE_TEXT_MAP[value]);
	}

	public void add(int value) {
		setValue((getValue() + value) % mAdapter.getBase());
	}

	public void setText(CharSequence text) {
		mTextViewTitle.setText(text);
	}

	public CharSequence getText() {
		return mTextViewTitle.getText();
	}
}
