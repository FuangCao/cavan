package com.cavan.radixconverter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;

public class BitAdapter extends BaseAdapter {

	private GridView mGridView;
	private LayoutInflater mInflater;
	private OnClickListener mClickListener;
	private OnLongClickListener mLongClickListener;

	private int mCount;
	private int mBase;
	private long mMask;
	private int mBitWidth;
	private BitView mViews[];

	public BitAdapter(GridView view, OnClickListener clickListener, OnLongClickListener longClickListener, int count, int base) {
		super();

		mGridView = view;
		mInflater = (LayoutInflater) mGridView.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		mClickListener = clickListener;
		mLongClickListener = longClickListener;

		mCount = count;
		mBase = base;

		switch (mBase) {
		case 2:
			mBitWidth = 1;
			break;
		case 4:
			mBitWidth = 2;
			break;
		case 8:
			mBitWidth = 3;
			break;
		case 16:
			mBitWidth = 4;
			break;
		default:
			mBitWidth = 0;
		}

		if (mBitWidth > 0) {
			mMask = (1 << mBitWidth) - 1;
		} else {
			mMask = 0;
		}

		mViews = new BitView[mCount];
		for (int i = 0; i < mCount; i++) {
			mViews[i] = (BitView) mInflater.inflate(R.layout.bit_view, mGridView, false);
			mViews[i].setup(this, i, mClickListener, mLongClickListener);
		}

		mGridView.setAdapter(this);
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

		return mViews[offset];
	}

	public int getBase() {
		return mBase;
	}

	public BitView[] getViews() {
		return mViews;
	}

	public BitView getView(int index) {
		if (index < 0 || index >= mCount) {
			return null;
		}

		return mViews[index];
	}

	public BitView getFirstView() {
		return mViews[0];
	}

	public BitView getLastView() {
		return mViews[mCount - 1];
	}

	public int getBitWidth() {
		return mBitWidth;
	}

	public long getMask() {
		return mMask;
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

	public void setActiveViews(int index, int count) {
		int i = 0;

		while (i < index) {
			mViews[i++].setActive(false);
		}

		index += count;
		if (index > mCount) {
			index = mCount;
		}

		while (i < index) {
			mViews[i++].setActive(true);
		}

		while (i < mCount) {
			mViews[i++].setActive(false);
		}
	}
}
