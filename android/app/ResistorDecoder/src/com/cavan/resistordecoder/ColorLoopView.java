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

public class ColorLoopView extends Spinner implements OnItemSelectedListener {

	public static final int COLOR_LOOP_TYPE_SIGNIFICANCE_DIGIT = 1;
	public static final int COLOR_LOOP_TYPE_MAGNITUDE = 2;
	public static final int COLOR_LOOP_TYPE_MISTAKE = 3;
	public static final int COLOR_LOOP_TYPE_TEMPERATUE_COEFFICIENT = 4;

	private static int[] sSignificanceDigitList = {
		R.string.color_black, R.string.color_brown, R.string.color_red, R.string.color_orange, R.string.color_yellow,
		R.string.color_green, R.string.color_blue, R.string.color_purple, R.string.color_gray, R.string.color_white
	};
	private static int[] sMagnitudeList = {
		R.string.color_black, R.string.color_brown, R.string.color_red, R.string.color_orange, R.string.color_yellow,
		R.string.color_green, R.string.color_blue, R.string.color_purple, R.string.color_gray, R.string.color_white,
		R.string.color_gold, R.string.color_silver
	};
	private static int[] sMistakeList = {
		R.string.color_brown, R.string.color_red, R.string.color_green, R.string.color_blue, R.string.color_purple,
		R.string.color_gray, R.string.color_gold, R.string.color_silver, R.string.color_none
	};
	private static int[] sTemperatueCoefficientList = {
		R.string.color_brown, R.string.color_red, R.string.color_orange, R.string.color_yellow, R.string.color_blue,
		R.string.color_purple, R.string.color_white
	};

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
	private int mType;
	private int[] mColorList;

	class ColorView extends TextView {

		private int mColorId;
		private int mColor;
		private int mBright;
		private int mIndex;

		public ColorView(Context context, int index) {
			super(context);

			mIndex = index;

			mColorId = mColorList[mIndex];
			setText(mColorId);
			setGravity(Gravity.CENTER);

			mColor = sHashMapColor.get(mColorId);
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

		public int getColorId() {
			return mColorId;
		}
	}

	private BaseAdapter mAdapterItem = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			return new ColorView(getContext(), position);
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
			return mColorList.length;
		}
	};

	public ColorLoopView(Context context, ResistorAdapter adapter, int type) {
		super(context);

		mType = type;
		switch (mType) {
		case COLOR_LOOP_TYPE_MAGNITUDE:
			mColorList = sMagnitudeList;
			break;

		case COLOR_LOOP_TYPE_MISTAKE:
			mColorList = sMistakeList;
			break;

		case COLOR_LOOP_TYPE_TEMPERATUE_COEFFICIENT:
			mColorList = sTemperatueCoefficientList;
			break;

		default:
			mColorList = sSignificanceDigitList;
			break;
		}

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
		ColorView item = (ColorView) arg1;
		setBackgroundColor(item.getColor());
		mAdapter.updateResistence();
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		setBackgroundColor(Color.TRANSPARENT);
	}
}
