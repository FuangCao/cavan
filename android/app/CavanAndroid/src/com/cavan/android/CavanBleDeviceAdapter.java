package com.cavan.android;

import java.util.ArrayList;
import java.util.List;

import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class CavanBleDeviceAdapter extends BaseAdapter implements OnClickListener {

	private ListView mListView;
	private Runnable mLastRunnable;
	private CavanBleDevice[] mDevices;
	private CavanBleDevice[] mPendDevices;
	private List<CavanBleDeviceView> mPendViews = new ArrayList<CavanBleDeviceView>();

	private Runnable mRunnableNotify = new Runnable() {

		@Override
		public void run() {
			synchronized (mPendViews) {
				mDevices = mPendDevices;

				if (mListView.getChildCount() == mDevices.length) {
					for (int i = mDevices.length - 1; i >= 0; i--) {
						CavanBleDeviceView view = (CavanBleDeviceView) mListView.getChildAt(i);
						view.setDevice(mDevices[i], i);
					}
				} else {
					notifyDataSetChanged();
				}

				mPendViews.clear();
				mLastRunnable = null;
			}
		}
	};

	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			synchronized (mPendViews) {
				for (CavanBleDeviceView view : mPendViews) {
					view.updateText();
				}

				mPendViews.clear();
				mLastRunnable = null;
			}
		}
	};

	protected void onItemClicked(CavanBleDeviceView view) {}

	public CavanBleDeviceAdapter(ListView view) {
		super();
		mListView = view;
		view.setAdapter(this);
	}

	public void setDevices(CavanBleDevice[] devices, CavanBleDevice device) {
		synchronized (mPendViews) {
			Runnable runnable;

			int index = device.getIndex();
			CavanBleDeviceView view = (CavanBleDeviceView) mListView.getChildAt(index);
			if (view != null && view.getDevice() == devices[index]) {
				mPendViews.add(view);
				runnable = mRunnableUpdate;
			} else {
				mPendDevices = devices;
				runnable = mRunnableNotify;

				if (mLastRunnable == mRunnableUpdate) {
					mListView.removeCallbacks(mRunnableUpdate);
					mLastRunnable = null;
				}
			}

			if (mLastRunnable == null) {
				mLastRunnable = runnable;
				mListView.postDelayed(runnable, 500);
			}
		}
	}

	@Override
	public int getCount() {
		if (mDevices == null) {
			return 0;
		}

		return mDevices.length;
	}

	@Override
	public Object getItem(int position) {
		return mDevices[position];
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		CavanBleDeviceView view;

		if (convertView == null) {
			view = new CavanBleDeviceView(mListView.getContext(), mDevices[position], position);
			view.setOnClickListener(this);
		} else {
			view = (CavanBleDeviceView) convertView;
			view.setDevice(mDevices[position], position);
		}

		return view;
	}

	@Override
	public void onClick(View v) {
		onItemClicked((CavanBleDeviceView) v);
	}
}
