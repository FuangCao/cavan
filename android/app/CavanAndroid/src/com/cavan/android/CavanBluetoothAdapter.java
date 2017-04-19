package com.cavan.android;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

public class CavanBluetoothAdapter {

	protected Context mContext;
	protected BluetoothAdapter mAdapter;

	private boolean mPoweredOn;
	private int mLastState = -1;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

			if (mLastState != state) {
				mLastState = state;
				onBluetoothAdapterStateChanged(state);
			}
		}
	};

	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		CavanAndroid.dLog("onBluetoothAdapterStateChanged: enabled = " + enabled);
	}

	protected void onBluetoothAdapterStateChanged(int state) {
		CavanAndroid.dLog("onBluetoothAdapterStateChanged: state = " + state);

		switch (state) {
		case BluetoothAdapter.STATE_ON:
			if (mPoweredOn) {
				break;
			}

			mPoweredOn = true;
			onBluetoothAdapterStateChanged(true);
			break;

		case BluetoothAdapter.STATE_OFF:
			if (mPoweredOn) {
				mPoweredOn = false;
				onBluetoothAdapterStateChanged(false);
			}
			break;
		}
	}

	public CavanBluetoothAdapter(Context context) {
		mContext = context;
		mAdapter = BluetoothAdapter.getDefaultAdapter();
		mPoweredOn = isAdapterEnabled();

		IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
		mContext.registerReceiver(mReceiver, filter);
	}

	public Context getContext() {
		return mContext;
	}

	public BluetoothAdapter getAdapter() {
		return mAdapter;
	}

	public boolean setAdapterEnable(boolean enable) {
		if (enable) {
			return mAdapter.enable();
		}

		return mAdapter.disable();
	}

	public boolean isAdapterEnabled() {
		return mAdapter.isEnabled();
	}

	public boolean isPoweredOn() {
		return isAdapterEnabled();
	}

	public BluetoothDevice getRemoteDevice(String address) {
		try {
			return mAdapter.getRemoteDevice(address);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	@Override
	protected void finalize() throws Throwable {
		mContext.unregisterReceiver(mReceiver);
		super.finalize();
	}
}