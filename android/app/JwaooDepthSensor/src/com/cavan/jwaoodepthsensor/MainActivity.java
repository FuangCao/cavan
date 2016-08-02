package com.cavan.jwaoodepthsensor;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooDepthSquareWaveGenerator;

public class MainActivity extends JwaooToyActivity {

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;

	private JwaooDepthSquareWaveGenerator mGenerator1 = new JwaooDepthSquareWaveGenerator(6, 1000, 5000);
	private JwaooDepthSquareWaveGenerator mGenerator2 = new JwaooDepthSquareWaveGenerator(6, 1000, 5000);

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveView1);
		mWaveView1.setValueRange(-128, 127);
		mWaveView1.setZoom(3);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveView2);
		mWaveView2.setValueRange(0, 1);
		mWaveView2.setZoom(3);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveView3);
		mWaveView3.setValueRange(-128, 127);
		mWaveView3.setZoom(3);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveView4);
		mWaveView4.setValueRange(0, 1);
		mWaveView4.setZoom(3);

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
				if (!connected) {
					showScanActivity();
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

				int capacitys[] = mSensor.getCapacitys();
				CavanAndroid.logE(String.format("capacity: [%6d, %6d, %6d, %6d]", capacitys[0], capacitys[1], capacitys[2], capacitys[3]));

				mWaveView1.addValue(capacitys[0]);
				mWaveView2.addValue(mGenerator1.putValue(capacitys[0]) ? 1 : 0);

				mWaveView3.addValue(capacitys[1]);
				mWaveView4.addValue(mGenerator2.putValue(capacitys[1]) ? 1 : 0);
			}
		};
	}
}