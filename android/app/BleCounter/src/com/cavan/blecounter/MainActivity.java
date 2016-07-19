package com.cavan.blecounter;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.java.CavanPeakValleyFinder;
import com.cavan.java.CavanPeakValleyValue;
import com.cavan.resource.CavanBleScanner;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends Activity {

	private static final int SENSOR_DELAY = 30;
	private static final int MSG_SHOW_SPEED = 1;

	private CavanPeakValleyFinder mFinders[] = {
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
	};

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;

	private int mCount;
	private JwaooBleToy mBleToy;
	private BluetoothDevice mDevice;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SHOW_SPEED:
				int count = mCount;
				mCount = 0;

				if (count > 0) {
					setTitle("freq = " + mFinders[0].getFreq() + ", count = " + count + ", speed = " + (1000.0 / count));
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

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveView1.setValueRange(-19.6, 19.6);
		mWaveView1.setZoom(3);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveView2.setValueRange(-19.6, 19.6);
		mWaveView2.setZoom(3);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveView3.setValueRange(0, 19.6);
		mWaveView3.setZoom(3);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveViewDepth);
		mWaveView4.setValueRange(0, 500);
		mWaveView4.setZoom(3);

		mHandler.sendEmptyMessage(MSG_SHOW_SPEED);
		CavanBleScanner.show(this);
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
		if (resultCode == RESULT_OK && data != null) {
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
							protected boolean onInitialize() {
								setSensorEnable(true, SENSOR_DELAY);
								return super.onInitialize();
							}

							@Override
							protected void onConnectionStateChange(boolean connected) {
								if (!connected) {
									CavanBleScanner.show(MainActivity.this);
								}
							}

							@Override
							protected void onSensorDataReceived(byte[] arg0) {
								mSensor.putBytes(arg0);

								double value = mSensor.getAxisZ();
								/* double x = accel.getCoorX();
								double y = accel.getCoorY();
								double z = accel.getCoorZ();
								double value = Math.sqrt(x * x + y * y + z * z); */

								mWaveView1.addValue(value);

								CavanPeakValleyValue result = mFinders[0].putFreqValue(value);
								if (result != null) {
									mWaveView3.addValue(result.getDiff());
								} else {
									mWaveView3.addValue(0);
								}

								mWaveView2.addValue(mFinders[0].getAvgValue());

								// int depth = accel.readValue8();
								// mWaveViewDepth.addValue(depth);

								// mWaveView4.addValue(value - mFinders[0].getValleyValue());
								/* mFinders[1].putValue(accel.getCoorX());
								mFinders[2].putValue(accel.getCoorY());
								mFinders[3].putValue(accel.getCoorZ());
								value = accel.getCoorX() - mFinders[1].getValleyValue();
								value += accel.getCoorY() - mFinders[2].getValleyValue();
								value += accel.getCoorZ() - mFinders[3].getValleyValue();
								mWaveView4.addValue(value); */
								mWaveView4.addValue(mFinders[0].getFreq());

								mCount++;
							}
						};

						if (!mBleToy.connect(true)) {
							CavanBleScanner.show(MainActivity.this);
						}
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
