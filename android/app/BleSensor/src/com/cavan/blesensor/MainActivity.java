package com.cavan.blesensor;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.CavanBleScanner;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends Activity {

	private static final int SENSOR_DELAY = 30;

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;
	private CavanWaveView mWaveViewDepth;

	private JwaooBleToy mBleToy;
	private BluetoothDevice mDevice;
	private Handler mHandler = new Handler();

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
		mWaveViewZ.setValueRange(-19.6, 19.6);
		mWaveViewZ.setZoom(3);

		mWaveViewDepth = (CavanWaveView) findViewById(R.id.waveViewDepth);
		mWaveViewDepth.setValueRange(0, JwaooToySensor.MAX_DEPTH);
		mWaveViewDepth.setZoom(3);

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
							protected void onConnectionStateChange(boolean connected) {
								if (!connected) {
									CavanBleScanner.show(MainActivity.this);
								}
							}

							@Override
							protected boolean onInitialize() {
								mBleToy.setSensorEnable(true, SENSOR_DELAY);
								return super.onInitialize();
							}

							@Override
							protected void onSensorDataReceived(byte[] arg0) {
								mSensor.putBytes(arg0);
								mWaveViewX.addValue(mSensor.getAxisX());
								mWaveViewY.addValue(mSensor.getAxisY());
								mWaveViewZ.addValue(mSensor.getAxisZ());
								mWaveViewDepth.addValue(mSensor.getDepth());
							}
						};
					} catch (Exception e) {
						e.printStackTrace();
						finish();
					}

					if (!mBleToy.connect(true)) {
						CavanBleScanner.show(MainActivity.this);
					}
				}
			});
		} else {
			finish();
		}
	}
}
