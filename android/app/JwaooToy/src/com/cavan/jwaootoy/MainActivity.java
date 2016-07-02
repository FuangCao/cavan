package com.cavan.jwaootoy;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleScanner;
import com.jwaoo.android.JwaooBleToy;

@SuppressLint("HandlerLeak")
public class MainActivity extends Activity implements OnClickListener {

	public static final int BLE_SCAN_RESULT = 1;
	private static final int EVENT_DATA_RECEIVED = 1;
	private static final int EVENT_OTA_START = 2;
	private static final int EVENT_OTA_FAILED = 3;
	private static final int EVENT_OTA_SUCCESS = 4;

	private JwaooBleToy mBleToy;
	private boolean mOtaBusy;

	private Button mButtonSend;
	private Button mButtonUpgrade;
	private Button mButtonReboot;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_DATA_RECEIVED:
				break;

			case EVENT_OTA_START:
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_start);
				break;

			case EVENT_OTA_FAILED:
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_failed);
				break;

			case EVENT_OTA_SUCCESS:
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_successfull);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);

		mButtonReboot = (Button) findViewById(R.id.buttonReboot);
		mButtonReboot.setOnClickListener(this);

		CavanBleScanner.show(this, BLE_SCAN_RESULT);
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonSend:
			String identify = mBleToy.doIdentify();
			CavanAndroid.logE("identify = " + identify);
			String buildDate = mBleToy.readBuildDate();
			CavanAndroid.logE("buildDate = " + buildDate);
			int version = mBleToy.readVersion();
			CavanAndroid.logE("version = " + Integer.toHexString(version));
			break;

		case R.id.buttonUpgrade:
			if (mOtaBusy) {
				break;
			}

			mOtaBusy = true;

			new Thread() {

				@Override
				public void run() {
					mHandler.sendEmptyMessage(EVENT_OTA_START);
					if (mBleToy.doOtaUpgrade("/mnt/sdcard/jwaoo-toy.hex")) {
						mHandler.sendEmptyMessage(EVENT_OTA_SUCCESS);
					} else {
						mHandler.sendEmptyMessage(EVENT_OTA_FAILED);
					}

					mOtaBusy = false;
				}
			}.start();
			break;

		case R.id.buttonReboot:
			mBleToy.doReboot();
			break;
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (requestCode == BLE_SCAN_RESULT && resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			}

			try {
				mBleToy = new JwaooBleToy(this, device) {

					@Override
					protected void onDisconnected() {
						CavanAndroid.logE("onDisconnected");
						CavanBleScanner.show(MainActivity.this, BLE_SCAN_RESULT);
					}

					@Override
					protected void onSensorDataReceived(byte[] data) {
					}
				};
			} catch (Exception e) {
				e.printStackTrace();
				finish();
			}
		} else {
			finish();
		}
	}
}
