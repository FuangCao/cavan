package com.cavan.resource;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.view.View;

import com.cavan.android.CavanAndroid;
import com.jwaoo.android.JwaooBleToy;

@SuppressLint("HandlerLeak")
public class JwaooToyActivity extends Activity {

	public static final int SENSOR_DELAY = 30;

	protected JwaooBleToy mBleToy;
	protected List<View> mListViews = new ArrayList<View>();

	private boolean mUiEnable;
	private Runnable mRunnableUpdateUi = new Runnable() {

		@Override
		public void run() {
			updateUI(mUiEnable);
		}
	};

	protected void handleMessage(Message msg) {}

	protected Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			JwaooToyActivity.this.handleMessage(msg);
		}
	};

	protected boolean onInitialize() {
		return true;
	}

	protected void onConnected() {
		updateUI(true);
	}

	protected void onSensorDataReceived(byte[] data) {}

	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected boolean onInitialize() {
				if (!JwaooToyActivity.this.onInitialize()) {
					CavanAndroid.eLog("Failed to JwaooToyActivity.this.onInitialize");
					return false;
				}

				return super.onInitialize();
			}

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.eLog("onConnectionStateChange: connected = " + connected);

				if (connected) {
					JwaooToyActivity.this.onConnected();
				} else {
					showScanActivity();
				}
			}

			@Override
			protected void onSensorDataReceived(byte[] data) {
				super.onSensorDataReceived(data);
				JwaooToyActivity.this.onSensorDataReceived(data);
			}
		};
	}

	public void updateUI(boolean enable) {
		if (CavanAndroid.isMainThread()) {
			for (View view : mListViews) {
				view.setEnabled(enable);
			}
		} else {
			mUiEnable = enable;
			mHandler.post(mRunnableUpdateUi);
		}
	}

	public void showScanActivity() {
		updateUI(false);
		CavanBleScanActivity.show(this, "JwaooToy");
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.eLog("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			} else {
				mBleToy = createJwaooBleToy(device);
				if (mBleToy == null || mBleToy.connect(true) == false) {
					showScanActivity();
				}
			}
		} else {
			finish();
		}
	}

	@Override
	public View findViewById(int id) {
		View view = super.findViewById(id);
		if (view != null && mListViews.indexOf(view) < 0) {
			mListViews.add(view);
		}

		return view;
	}

	@Override
	protected void onDestroy() {
		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		super.onDestroy();
	}
}
