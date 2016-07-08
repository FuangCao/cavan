package com.cavan.blecounter;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleScanner;
import com.cavan.android.CavanWaveView;
import com.cavan.java.CavanPeakValleyFinder;
import com.cavan.java.CavanPeakValleyValue;
import com.cavan.java.Mpu6050Accel;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends Activity {

	public static final int BLE_SCAN_RESULT = 1;

	private static final int MSG_SENSOR_ENABLE = 1;
	private static final int MSG_SHOW_SPEED = 2;

	private CavanPeakValleyFinder mFinder = new CavanPeakValleyFinder(100, 1);

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;
	private CavanWaveView mWaveViewDepth;

	private int mCount;
	private JwaooBleToy mBleToy;
	private BluetoothDevice mDevice;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SENSOR_ENABLE:
				mBleToy.setSensorDelay(10);
				mBleToy.setSensorEnable(true);
				break;

			case MSG_SHOW_SPEED:
				int count = mCount;
				mCount = 0;

				if (count > 0) {
					setTitle("count = " + count + ", speed = " + (1000.0 / count));
				}
				mHandler.sendEmptyMessageDelayed(MSG_SHOW_SPEED, 1000);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mWaveViewX = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveViewX.setValueRange(-19.6, 19.6);
		mWaveViewX.setZoom(3);

		mWaveViewY = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveViewY.setValueRange(-19.6, 19.6);
		mWaveViewY.setZoom(3);

		mWaveViewZ = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveViewZ.setValueRange(0, 19.6);
		mWaveViewZ.setZoom(3);

		mWaveViewDepth = (CavanWaveView) findViewById(R.id.waveViewDepth);
		mWaveViewDepth.setValueRange(0, 4);
		mWaveViewDepth.setZoom(3);

		mHandler.sendEmptyMessage(MSG_SHOW_SPEED);
		CavanBleScanner.show(this, BLE_SCAN_RESULT);
	}

	@Override
	protected void onDestroy() {
		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		super.onDestroy();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (requestCode == BLE_SCAN_RESULT && resultCode == RESULT_OK && data != null) {
			mDevice = data.getParcelableExtra("device");
			if (mDevice == null) {
				finish();
			}

			mHandler.post(new Runnable() {

				@Override
				public void run() {

					try {
						mBleToy = new JwaooBleToy(MainActivity.this, mDevice) {

							@Override
							protected void onConnectionStateChange(boolean connected) {
								if (connected) {
									mHandler.sendEmptyMessage(MSG_SENSOR_ENABLE);
								} else {
									CavanBleScanner.show(MainActivity.this, BLE_SCAN_RESULT);
								}
							}

							@Override
							protected void onSensorDataReceived(byte[] arg0) {
								Mpu6050Accel accel = new Mpu6050Accel(arg0);
								double value = accel.getCoorZ();

								mWaveViewX.addValue(value);

								CavanPeakValleyValue result = mFinder.putValue(value);
								if (result != null && result.isRising()) {
									mWaveViewZ.addValue(result.getDiff());
								} else {
									mWaveViewZ.addValue(0);
								}

								mWaveViewY.addValue(mFinder.getAvgValue());

								int depth = accel.readValue8();
								mWaveViewDepth.addValue(depth);

								mCount++;
							}
						};
					} catch (Exception e) {
						e.printStackTrace();
						finish();
					}
				}
			});
		} else {
			finish();
		}
	}
}
