package com.cavan.blesensor;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends JwaooToyActivity {

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveViewX = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveViewX.setValueRange(-19.6, 19.6);
		mWaveViewX.setZoom(3);

		mWaveViewY = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveViewY.setValueRange(-19.6, 19.6);
		mWaveViewY.setZoom(3);

		mWaveViewZ = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveViewZ.setValueRange(-19.6, 19.6);
		mWaveViewZ.setZoom(3);

		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.setSuspendEnable(this, true);
		super.onDestroy();
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected void onConnectionStateChange(boolean connected) {
				showProgressDialog(!connected);
			}

			@Override
			protected void onConnectFailed() {
				showScanActivity();
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
			}
		};
	}
}
