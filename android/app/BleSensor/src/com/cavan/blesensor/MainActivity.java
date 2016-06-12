package com.cavan.blesensor;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;

import com.cavan.cavanutils.BleScanner;
import com.cavan.cavanutils.CavanBleUart;
import com.cavan.cavanutils.CavanGattCharacteristic;
import com.cavan.cavanutils.CavanUtils;

@SuppressLint({ "HandlerLeak", "DefaultLocale" })
public class MainActivity extends Activity {

	public static int BLE_SCAN_RESULT = 1;

	private CavanBleUart mBleUart;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		doScan();
	}

	private void doScan() {
		Intent intent = new Intent(this, BleScanner.class);
		startActivityForResult(intent, BLE_SCAN_RESULT);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanUtils.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (requestCode == BLE_SCAN_RESULT && resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			}

			mBleUart = new CavanBleUart(device) {

				@Override
				protected void onDataReceived(CavanGattCharacteristic characteristic, byte[] data) {
					if (data.length == 14) {
						float accX = ((float) (short) ((data[0] & 0xFF) << 8 | (data[1] & 0xFF))) / 16384;
						float accY = ((float) (short) ((data[2] & 0xFF) << 8 | (data[3] & 0xFF))) / 16384;
						float accZ = ((float) (short) ((data[4] & 0xFF) << 8 | (data[5] & 0xFF))) / 16384;
						float temp = (((float) (short) ((data[6] & 0xFF) << 8 | (data[7] & 0xFF))) + 13200) / 280 - 13;
						float gyrX = ((float) (short) ((data[8] & 0xFF) << 8 | (data[9] & 0xFF))) / 131;
						float gyrY = ((float) (short) ((data[10] & 0xFF) << 8 | (data[11] & 0xFF))) / 131;
						float gyrZ = ((float) (short) ((data[12] & 0xFF) << 8 | (data[13] & 0xFF))) / 131;

						CavanUtils.logE(String.format("ACC: [%f, %f, %f]", accX, accY, accZ));
						CavanUtils.logE(String.format("GYR: [%f, %f, %f]", gyrX, gyrY, gyrZ));
						CavanUtils.logE("temp = " + temp);
					}

				}
			};

			if (!mBleUart.connect(this)) {
				finish();
			}
		} else {
			finish();
		}
	}
}
