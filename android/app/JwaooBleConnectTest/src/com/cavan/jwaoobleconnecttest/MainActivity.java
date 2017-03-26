package com.cavan.jwaoobleconnecttest;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends JwaooToyActivity {

	private static final int MSG_CONN_COMPLETE = 100;

	private int mFailed;
	private int mSuccess;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.acquireWakeLock(this);
		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.releaseWakeLock();
		super.onDestroy();
	}

	@Override
	protected void onConnectionStateChange(boolean connected) {
		super.onConnectionStateChange(connected);

		if (connected) {
			mSuccess++;
			mHandler.sendEmptyMessageDelayed(MSG_CONN_COMPLETE, 3000);
		}
	}

	@Override
	public boolean handleMessage(Message msg) {
		if (super.handleMessage(msg)) {
			return true;
		}

		if (msg.what == MSG_CONN_COMPLETE) {
			mBleToy.disconnect();
			mBleToy.connect();
		}

		return true;
	}

	@Override
	protected String buildProgressDialogMessage() {
		return "success = " + mSuccess + ", failed = " + mFailed;
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.dLog("JwaooBleToy.onConnectionStateChange: connected = " + connected);
				showProgressDialog(!connected);

				if (connected) {
					mSuccess++;
					mHandler.sendEmptyMessageDelayed(MSG_CONN_COMPLETE, 1000);
				}
			}

			@Override
			protected void onConnectFailed() {
				mFailed++;
				mHandler.sendEmptyMessageDelayed(MSG_CONN_COMPLETE, 5000);
			}
		};
	}
}
