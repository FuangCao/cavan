package com.cavan.radixconverter;

import android.content.Context;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;

public class KeypadAdapter extends BaseAdapter {

	public static final String VALUE_TEXT_MAP[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };

	private Context mContext;
	private OnClickListener mListener;

	private int mCount = VALUE_TEXT_MAP.length;
	private Button mButtons[] = new Button[VALUE_TEXT_MAP.length];

	public KeypadAdapter(Context context, OnClickListener listener) {
		super();

		mContext = context;
		mListener = listener;
	}

	@Override
	public int getCount() {
		return mCount;
	}

	@Override
	public Object getItem(int position) {
		return null;
	}

	@Override
	public long getItemId(int position) {
		return 0;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		Button button = mButtons[position];
		if (button == null) {
			mButtons[position] = button = new Button(mContext);
			button.setText(VALUE_TEXT_MAP[position]);
			button.setOnClickListener(mListener);
		}

		return button;
	}

	public void setKeyCount(int count) {
		if (count > VALUE_TEXT_MAP.length) {
			count = VALUE_TEXT_MAP.length;
		}

		for (int i = 0; i < count; i++) {
			if (mButtons[i] != null) {
				mButtons[i].setVisibility(View.VISIBLE);
			}
		}

		for (int i = count; i < VALUE_TEXT_MAP.length; i++) {
			if (mButtons[i] != null) {
				mButtons[i].setVisibility(View.INVISIBLE);
			}
		}
	}

}
