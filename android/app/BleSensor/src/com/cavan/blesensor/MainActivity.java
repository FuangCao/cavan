package com.cavan.blesensor;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleScanner;
import com.cavan.android.CavanWaveView;
import com.cavan.java.AccelFreqParser;
import com.cavan.java.CavanFreqParser;
import com.cavan.java.Mpu6050Accel;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends Activity {

	public static final int BLE_SCAN_RESULT = 1;

	private static final int MSG_SENSOR_ENABLE = 1;

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;
	private CavanWaveView mWaveViewDepth;
	private AccelFreqParser mParser = new AccelFreqParser(2);

	private JwaooBleToy mBleToy;
	private BluetoothDevice mDevice;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SENSOR_ENABLE:
				mBleToy.setSensorDelay(20);
				mBleToy.setSensorEnable(true);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mWaveViewX = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveViewX.setValueRange(0, 500);
		mWaveViewX.setZoom(3);

		mWaveViewY = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveViewY.setValueRange(0, 9.8);
		mWaveViewY.setZoom(3);

		mWaveViewZ = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveViewZ.setValueRange(-9.8, 9.8);
		mWaveViewZ.setZoom(3);

		mWaveViewDepth = (CavanWaveView) findViewById(R.id.waveViewDepth);
		mWaveViewDepth.setValueRange(0, 4);
		mWaveViewDepth.setZoom(3);

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
							protected void onConnected() {
								mHandler.sendEmptyMessage(MSG_SENSOR_ENABLE);
								super.onConnected();
							}

							@Override
							protected void onDisconnected() {
								CavanBleScanner.show(MainActivity.this, BLE_SCAN_RESULT);
							}

							@Override
							protected void onSensorDataReceived(byte[] arg0) {
								Mpu6050Accel accel = new Mpu6050Accel(arg0);
								CavanFreqParser parser = mParser.putValue(accel);
								CavanAndroid.logE("freq = " + parser.getFreq());
								mWaveViewX.addValue(parser.getFreq());
								mWaveViewY.addValue(parser.getAvgDiff());

								int depth = accel.readValue8();
								mWaveViewDepth.addValue(depth);
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
