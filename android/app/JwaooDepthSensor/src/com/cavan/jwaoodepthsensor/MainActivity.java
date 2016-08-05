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
	private CavanWaveView mWaveView5;
	private CavanWaveView mWaveView6;

	private JwaooDepthSquareWaveGenerator mGenerator1 = new JwaooDepthSquareWaveGenerator(10, 1000, 10000);
	private JwaooDepthSquareWaveGenerator mGenerator2 = new JwaooDepthSquareWaveGenerator(10, 1000, 10000);
	private JwaooDepthSquareWaveGenerator mGenerator3 = new JwaooDepthSquareWaveGenerator(10, 1000, 10000);

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveView1);
		mWaveView1.setValueRange(40, 60);
		mWaveView1.setZoom(3);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveView2);
		mWaveView2.setValueRange(0, 1);
		mWaveView2.setZoom(3);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveView3);
		mWaveView3.setValueRange(20, 40);
		mWaveView3.setZoom(3);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveView4);
		mWaveView4.setValueRange(0, 1);
		mWaveView4.setZoom(3);

		mWaveView5 = (CavanWaveView) findViewById(R.id.waveView5);
		mWaveView5.setValueRange(10, 35);
		mWaveView5.setZoom(3);

		mWaveView6 = (CavanWaveView) findViewById(R.id.waveView6);
		mWaveView6.setValueRange(0, 1);
		mWaveView6.setZoom(3);

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

				double capacitys[] = mSensor.getCapacitys();
				CavanAndroid.logE(String.format("capacity: [%7.2f, %7.2f, %7.2f, %7.2f]", capacitys[0], capacitys[1], capacitys[2], capacitys[3]));

				double value = capacitys[0];

				mWaveView1.addValue(value);
				mWaveView2.addValue(mGenerator1.putValue(value) ? 1 : 0);

				value = capacitys[1];
				mWaveView3.addValue(value);
				mWaveView4.addValue(mGenerator2.putValue(value) ? 1 : 0);

				value = capacitys[2];
				mWaveView5.addValue(value);
				mWaveView6.addValue(mGenerator3.putValue(value) ? 1 : 0);
			}
		};
	}
}
