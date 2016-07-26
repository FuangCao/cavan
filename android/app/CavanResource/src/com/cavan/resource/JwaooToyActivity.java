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
import com.jwaoo.android.JwaooBleToy.JwaooToyBmi160;
import com.jwaoo.android.JwaooBleToy.JwaooToyFdc1004;
import com.jwaoo.android.JwaooBleToy.JwaooToyMpu6050;

@SuppressLint("HandlerLeak")
public class JwaooToyActivity extends Activity {

	public static final int SENSOR_DELAY = 30;

	private static final int EVENT_BMI160_POLL = 1;
	private static final int EVENT_MPU6050_POLL = 2;
	private static final int EVENT_UPDATE_UI = 10;

	protected JwaooBleToy mBleToy;
	protected JwaooToyBmi160 mBmi160;
	protected JwaooToyMpu6050 mMpu6050;
	protected JwaooToyFdc1004 mFdc1004;
	protected List<View> mListViews = new ArrayList<View>();

	protected Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_BMI160_POLL:
				if (mBmi160.updateData()) {
					CavanAndroid.logE("bmi160: " + mBmi160);
					mHandler.sendEmptyMessageDelayed(msg.what, 100);
				}
				break;

			case EVENT_MPU6050_POLL:
				if (mMpu6050.updateData()) {
					CavanAndroid.logE("mpu6050: " + mMpu6050);
					mHandler.sendEmptyMessageDelayed(msg.what, 100);
				}
				break;

			case EVENT_UPDATE_UI:
				updateUI((boolean) msg.obj);
				break;
			}
		}
	};

	protected boolean onInitialize() {
		return true;
	}

	protected void onConnected() {}
	protected void onSensorDataReceived(byte[] data) {}

	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(this, device) {

			@Override
			protected boolean onInitialize() {
				mBmi160 = mBleToy.createBmi160();
				if (mBmi160.doInitialize() && mBmi160.setEnable(true)) {
					CavanAndroid.logE("=> BMI160 found");
				} else {
					mBmi160 = null;
				}

				mMpu6050 = mBleToy.createMpu6050();
				if (mMpu6050.doInitialize() && mMpu6050.setEnable(true)) {
					CavanAndroid.logE("=> MPU6050 found");
				} else {
					mMpu6050 = null;
				}

				mFdc1004 = mBleToy.createFdc1004();
				if (mFdc1004.doInitialize() && mFdc1004.setEnable(true)) {
					CavanAndroid.logE("=> FDC1004 found");
				} else {
					mFdc1004 = null;
				}

				if (!JwaooToyActivity.this.onInitialize()) {
					CavanAndroid.logE("Failed to JwaooToyActivity.this.onInitialize");
					return false;
				}

				return super.onInitialize();
			}

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.logE("onConnectionStateChange: connected = " + connected);

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
			mHandler.obtainMessage(EVENT_UPDATE_UI, enable).sendToTarget();
		}
	}

	public void showScanActivity() {
		updateUI(false);
		CavanBleScanActivity.show(this, "JwaooToy");
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
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
	protected void onDestroy() {
		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		super.onDestroy();
	}
}
