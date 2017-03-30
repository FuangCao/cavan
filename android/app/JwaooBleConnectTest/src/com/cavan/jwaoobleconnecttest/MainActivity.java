package com.cavan.jwaoobleconnecttest;

import android.os.Bundle;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.JwaooToyActivity;

public class MainActivity extends JwaooToyActivity {

	private static final int MSG_CONN_COMPLETE = 100;

	private int mFailed;
	private int mSuccess;

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
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
	public void onConnectionStateChanged(boolean connected) {
		super.onConnectionStateChanged(connected);

		if (connected) {
			mSuccess++;
			mHandler.sendEmptyMessageDelayed(MSG_CONN_COMPLETE, 1000);
		}
	}

	@Override
	public void onConnectFailed() {
		mFailed++;
		mHandler.sendEmptyMessageDelayed(MSG_CONN_COMPLETE, 5000);
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
}
