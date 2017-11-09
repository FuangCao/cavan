package com.cavan.android;

import android.content.Context;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListAdapter;

public abstract class CavanBleDeviceAdapter extends BaseAdapter {

	private CavanBleDevice[] mDevices;
	private CavanBleDevice[] mPendDevices;
	private AdapterView<ListAdapter> mAdapterView;

	protected abstract void selectDevice(View view, CavanBleDevice device);

	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			if (mPendDevices != null) {
				mDevices = mPendDevices;
				mPendDevices = null;

				notifyDataSetChanged();
			}
		}
	};

	public CavanBleDeviceAdapter(AdapterView<ListAdapter> view) {
		super();
		mAdapterView = view;
		view.setAdapter(this);
	}

	public Context getContext() {
		return mAdapterView.getContext();
	}

	public void setDevices(CavanBleDevice[] devices, CavanBleDevice device) {
		if (mPendDevices == null) {
			mPendDevices = devices;
			mAdapterView.postDelayed(mRunnableUpdate, 500);
		} else {
			mPendDevices = devices;
		}
	}

	public View getView(CavanBleDevice device, View convertView, ViewGroup parent) {
		Button view;

		if (convertView != null) {
			view = (Button) convertView;
		} else {
			view = new Button(getContext());
			view.setAllCaps(false);
		}

		view.setText(device.toString());

		return view;
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
		CavanBleDevice device = mDevices[position];
		View view = getView(device, convertView, parent);
		if (view == null) {
			return null;
		}

		view.setOnClickListener(new OnBleDeviceClickListener(view, device));

		return view;
	}

	public class OnBleDeviceClickListener implements OnClickListener {

		private View mView;
		private CavanBleDevice mDevice;

		public OnBleDeviceClickListener(View view, CavanBleDevice device) {
			mView = view;
			mDevice = device;
		}

		@Override
		public void onClick(View v) {
			selectDevice(mView, mDevice);
		}
	}
}
