package com.cavan.radixconverter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class BitAdapter extends BaseAdapter {

	private Context mContext;
	private LayoutInflater mInflater;
	private OnClickListener mListener;

	private int mCount;
	private int mBase;
	private BitView mViews[];

	public BitAdapter(Context context, OnClickListener listener, int count, int base) {
		super();

		mContext = context;
		mInflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		mListener = listener;

		mCount = count;
		mBase = base;
		mViews = new BitView[mCount];
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

	public LayoutInflater getLayoutInflater() {
		return mInflater;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		int offset = mCount - position - 1;

		BitView view = mViews[offset];
		if (view == null) {
			mViews[offset] = view = (BitView) mInflater.inflate(R.layout.bit_view, parent, false);
			view.setup(this, offset, mListener);
		}

		return view;
	}

	public int getBase() {
		return mBase;
	}

	public BitView[] getViews() {
		return mViews;
	}

	public BitView getButton(int index) {
		return mViews[index];
	}

	public void setValue(long value) {
		for (int i = 0; i < mCount; i++, value /= mBase) {
			if (mViews[i] != null) {
				mViews[i].setValue((int) (value % mBase));
			}
		}
	}

	public long getValue() {
		long value = 0;

		for (int i = mCount - 1; i >= 0; i--) {
			value = value * mBase + mViews[i].getValue();
		}

		return value;
	}
}
