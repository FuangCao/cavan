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
	private BroadcastReceiver mReceiver;
	private boolean mPoweredOn;
	private int mLastState = -1;

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
		mAdapter = BluetoothAdapter.getDefaultAdapter();
		mPoweredOn = isAdapterEnabled();
		setContext(context);
	}

	public Context getContext() {
		return mContext;
	}

	public void setContext(Context context) {
		if (context == null || context == mContext) {
			return;
		}

		if (mContext != null && mReceiver != null) {
			mContext.unregisterReceiver(mReceiver);
		}

		mContext = context;

		if (mReceiver == null) {
			mReceiver = new BroadcastReceiver() {

				@Override
				public void onReceive(Context context, Intent intent) {
					int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

					if (mLastState != state) {
						mLastState = state;
						onBluetoothAdapterStateChanged(state);
					}
				}
			};
		}

		IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
		context.registerReceiver(mReceiver, filter);
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

	public void cleaup() {
		if (mContext != null && mReceiver != null) {
			mContext.unregisterReceiver(mReceiver);
			mReceiver = null;
		}
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
		cleaup();
		super.finalize();
	}
}