package com.cavan.resistordecoder;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class ResistorAdapter extends BaseAdapter {

	private Context mContext;

	private ColorView mViews[];
	private int mCount;

	public ResistorAdapter(Context context, int count) {
		super();

		mContext = context;
		mCount = count;
		mViews = new ColorView[mCount];
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
			mViews[position] = view = new ColorView(mContext);
		}

		return view;
	}

	double getValue() {
		int i = 0;
		long value = 0;

		while (i < mCount - 2) {
			value = value * 10 + mViews[i++].getValue();
		}

		return value * Math.pow(10, mViews[i++].getValue());
	}

	void setValue(double value) {
		if (value < 1) {

		} else {

		}
	}
}
