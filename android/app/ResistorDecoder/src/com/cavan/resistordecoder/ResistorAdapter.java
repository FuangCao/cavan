package com.cavan.resistordecoder;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class ResistorAdapter extends BaseAdapter {

	private Context mContext;
	private ColorLoopView mViews[];
	private int mCount;
	private int mMistakeBit;
	private int mMagnitudeBit;
	private int mTempCofficientBit;
	private int mSignificantDigitBits;
	private long mSignificantDigitMax;
	private OnResistenceChangedListener mListener;

	interface OnResistenceChangedListener {
		void onResistenceChanged(ResistorAdapter adapter);
	}

	public ResistorAdapter(Context context, int count) {
		super();

		mContext = context;
		mCount = count;
		mViews = new ColorLoopView[mCount];

		mSignificantDigitBits = mCount - 2;
		if (mSignificantDigitBits > 3) {
			mSignificantDigitBits = 3;
		}

		mSignificantDigitMax = ((long) Math.pow(10, mSignificantDigitBits)) - 1;

		mMagnitudeBit = mSignificantDigitBits;
		mMistakeBit = mMagnitudeBit + 1;
		mTempCofficientBit = mMistakeBit + 1;
		if (mTempCofficientBit >= mCount) {
			mTempCofficientBit = -1;
		}
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

	private int getColorLoopType(int index) {
		if (index < mSignificantDigitBits) {
			return ColorLoopView.COLOR_LOOP_TYPE_SIGNIFICANCE_DIGIT;
		} else if (index == mMagnitudeBit) {
			return ColorLoopView.COLOR_LOOP_TYPE_MAGNITUDE;
		} else if (index == mMistakeBit) {
			return ColorLoopView.COLOR_LOOP_TYPE_MISTAKE;
		} else {
			return ColorLoopView.COLOR_LOOP_TYPE_TEMPERATUE_COEFFICIENT;
		}
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ColorLoopView view = mViews[position];
		if (view == null) {
			mViews[position] = view = new ColorLoopView(mContext, this, getColorLoopType(position));
		}

		return view;
	}

	double getResistence() {
		int i = 0;
		long value = 0;

		while (i < mSignificantDigitBits) {
			value = value * 10 + mViews[i++].getValue();
		}

		return value * Math.pow(10, mViews[i++].getValue());
	}

	void setResistence(double resistence, int mistake, int tempCofficient) {
		int pow = 0;

		while (resistence > (long) resistence && pow > -2) {
			resistence *= 10;
			pow--;
		}

		while (resistence > mSignificantDigitMax && pow < 9) {
			resistence /= 10;
			pow++;
		}

		long iResistence = (long) resistence;
		if (iResistence > mSignificantDigitMax) {
			iResistence = mSignificantDigitMax;
		}

		for (int i = mSignificantDigitBits - 1; i >= 0; i--, iResistence /= 10) {
			mViews[i].setValue((int) (iResistence % 10));
		}

		mViews[mSignificantDigitBits].setValue(pow);

		setMistake(mistake);
		setTempCofficient(tempCofficient);
	}

	public void setMistake(int value) {
		mViews[mMistakeBit].setValue(value);
	}

	public int getMistake() {
		return mViews[mMistakeBit].getValue();
	}

	public void setTempCofficient(int value) {
		if (mTempCofficientBit >= 0) {
			mViews[mTempCofficientBit].setValue(value);
		}
	}

	public int getTempCofficient() {
		if (mTempCofficientBit < 0) {
			return 0;
		}

		return mViews[mTempCofficientBit].getValue();
	}

	void updateResistence() {
		if (mListener != null) {
			mListener.onResistenceChanged(this);
		}
	}

	public void setOnResistenceChangedListener(OnResistenceChangedListener listener) {
		mListener = listener;
	}
}
