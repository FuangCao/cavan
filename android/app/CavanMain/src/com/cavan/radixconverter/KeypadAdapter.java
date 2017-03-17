package com.cavan.radixconverter;

import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;

public class KeypadAdapter extends BaseAdapter {

	public static final String NUM_KEY_LIST[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
	public static final String CTRL_KEY_LIST[] = { "◁", "○", "▷", "~", "◀" };

	private GridView mGridView;
	private OnClickListener mClickListenerNum;
	private OnClickListener mClickListenerCtrl;
	private OnLongClickListener mLongClickListenerCtrl;

	private int mNumKeyCount = NUM_KEY_LIST.length;
	private int mCtrlKeyCount = CTRL_KEY_LIST.length;
	private Button[] mButtons;

	public KeypadAdapter(GridView view, OnClickListener clickListenerNum, OnClickListener clickListenerCtrl, OnLongClickListener longClickListenerCtrl) {
		super();

		mGridView = view;
		mClickListenerNum = clickListenerNum;
		mClickListenerCtrl = clickListenerCtrl;
		mLongClickListenerCtrl = longClickListenerCtrl;

		updateKeys();
	}

	@Override
	public int getCount() {
		return mButtons.length;
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
		return mButtons[position];
	}

	private Button createButton(int id, String text, OnClickListener clickListener, OnLongClickListener longClickListener) {
		Button button = new Button(mGridView.getContext());

		button.setId(id);
		button.setText(text);
		button.setOnClickListener(clickListener);
		button.setOnLongClickListener(longClickListener);

		return button;
	}

	private void updateKeys() {
		mGridView.setAdapter(null);

		mButtons = new Button[mNumKeyCount + mCtrlKeyCount];

		int index = 0;

		for (int i = 0; i < mNumKeyCount; i++, index++) {
			mButtons[index] = createButton(i, NUM_KEY_LIST[i], mClickListenerNum, null);
		}

		for (int i = 0; i < mCtrlKeyCount; i++, index++) {
			mButtons[index] = createButton(i, CTRL_KEY_LIST[i], mClickListenerCtrl, mLongClickListenerCtrl);
		}

		int columns;

		if (mButtons.length <= 8) {
			columns = mButtons.length;
		} else if (mButtons.length <= 16) {
			columns = (mButtons.length + 1) / 2;
		} else {
			columns = (mButtons.length + 2) / 3;
		}

		mGridView.setNumColumns(columns);
		mGridView.setAdapter(this);
	}

	public void setNumKeyCount(int count) {
		if (count > NUM_KEY_LIST.length) {
			count = NUM_KEY_LIST.length;
		}

		if (count != mNumKeyCount) {
			mNumKeyCount = count;
			updateKeys();
		}
	}

	public void setCtrlKeyCount(int count) {
		if (count > CTRL_KEY_LIST.length) {
			count = CTRL_KEY_LIST.length;
		}

		if (count != mCtrlKeyCount) {
			mCtrlKeyCount = count;
			updateKeys();
		}
	}
}
