package com.cavan.resistordecoder;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class ResistorAdapter extends BaseAdapter {

	private Context mContext;
	private ColorView mViews[];
	private int mCount;
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
		mViews = new ColorView[mCount];

		mSignificantDigitBits = mCount - 2;
		mSignificantDigitMax = ((long) Math.pow(10, mSignificantDigitBits)) - 1;
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
		ColorView view = mViews[position];
		if (view == null) {
			mViews[position] = view = new ColorView(mContext, this);
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

	void setResistence(double resistence) {
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
