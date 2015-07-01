package com.cavan.resistordecoder;

import android.content.Context;
import android.graphics.Color;
import android.util.SparseIntArray;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Spinner;
import android.widget.TextView;

public class ColorView extends Spinner implements OnItemSelectedListener {

	private static int[] sColorNameList = { R.string.color_black, R.string.color_brown, R.string.color_red, R.string.color_orange, R.string.color_yellow, R.string.color_green, R.string.color_blue, R.string.color_purple, R.string.color_gray, R.string.color_white, R.string.color_gold, R.string.color_silver, R.string.color_none };
	private static SparseIntArray sHashMapColor = new SparseIntArray();

	static {
		sHashMapColor.put(R.string.color_black, 0xFF000000);
		sHashMapColor.put(R.string.color_blue, 0xFF0000FF);
		sHashMapColor.put(R.string.color_brown, 0xFFA52A2A);
		sHashMapColor.put(R.string.color_gold, 0xFFFFD700);
		sHashMapColor.put(R.string.color_gray, 0xFF808080);
		sHashMapColor.put(R.string.color_green, 0xFF00FF00);
		sHashMapColor.put(R.string.color_orange, 0xFFFFA500);
		sHashMapColor.put(R.string.color_purple, 0xFF800080);
		sHashMapColor.put(R.string.color_red, 0xFFFF0000);
		sHashMapColor.put(R.string.color_silver, 0xFFC0C0C0);
		sHashMapColor.put(R.string.color_white, 0xFFFFFFFF);
		sHashMapColor.put(R.string.color_yellow, 0xFFFFFF00);
		sHashMapColor.put(R.string.color_none, 0xFFFFFFFF);
	}

	private ResistorAdapter mAdapter;

	class ColorViewItem extends TextView {

		private int mNameId;
		private int mColor;
		private int mBright;
		private int mIndex;

		public ColorViewItem(Context context, int index) {
			super(context);

			mIndex = index;

			mNameId = sColorNameList[mIndex];
			setText(mNameId);
			setGravity(Gravity.CENTER);

			mColor = sHashMapColor.get(mNameId);
			setBackgroundColor(mColor);

			mBright = (int) (0.299 * ((mColor >> 16) & 0xFF) + 0.587 * ((mColor >> 8) & 0xFF) + 0.114 * (mColor & 0xFF));
			setTextColor(mBright > 0x7F ? Color.BLACK : Color.WHITE);

			setHeight(80);
		}

		public int getColor() {
			return mColor;
		}

		public int getIndex() {
			return mIndex;
		}

		public int getNameId() {
			return mNameId;
		}
	}

	private BaseAdapter mAdapterItem = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			return new ColorViewItem(getContext(), position);
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public int getCount() {
			return sColorNameList.length;
		}
	};

	public ColorView(Context context, ResistorAdapter adapter) {
		super(context);

		mAdapter = adapter;

		setAdapter(mAdapterItem);
		setOnItemSelectedListener(this);
		setGravity(Gravity.CENTER);
		setBackgroundColor(Color.TRANSPARENT);
	}

	int getValue() {
		int index;

		index = getSelectedItemPosition();
		if (index < 10) {
			return index;
		}

		return 9 - index;
	}

	void setValue(int value) {
		if (value < 0) {
			value = 9 - value;
		}

		setSelection(value);
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		ColorViewItem item = (ColorViewItem) arg1;
		setBackgroundColor(item.getColor());
		mAdapter.updateResistence();
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		setBackgroundColor(Color.TRANSPARENT);
	}
}
