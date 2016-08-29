package com.cavan.bleuart;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleGatt.CavanBleDataListener;
import com.cavan.android.CavanBleUart;
import com.cavan.java.CavanHexFile;
import com.cavan.resource.CavanBleScanActivity;

@SuppressLint("HandlerLeak")
public class MainActivity extends Activity implements OnClickListener, OnLongClickListener {

	public static int BLE_SCAN_RESULT = 1;
	private static final int EVENT_DATA_RECEIVED = 1;

	private CavanBleUart mBleUart;

	private Button mButtonSend;
	private Button mButtonUpgrade;
	private EditText mEditTextSend;
	private EditText mEditTextRecv;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_DATA_RECEIVED:
				String text = (String) msg.obj;
				mEditTextRecv.append(text);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mEditTextRecv = (EditText) findViewById(R.id.editTextRecv);
		mEditTextRecv.setOnLongClickListener(this);

		mEditTextSend = (EditText) findViewById(R.id.editTextSend);
		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);

		CavanBleScanActivity.show(this, BLE_SCAN_RESULT);
	}

	public boolean sendText(String text) {
		return mBleUart != null && mBleUart.sendText(text);
	}

	private boolean otaUpgrade() {
		CavanHexFile file = new CavanHexFile("/data/local/tmp/dialog.hex");
		byte[] bytes = file.parse();
		if (bytes == null) {
			CavanAndroid.eLog("Failed to parse hex file");
			return false;
		}

		int length = (bytes.length + 7) & (~0x07);
		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };
		if (mBleUart == null || mBleUart.writeOta(header) == false) {
			return false;
		}

		return mBleUart != null && mBleUart.writeOta(bytes);
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonSend:
			sendText(mEditTextSend.getText().toString());
			break;

		case R.id.buttonUpgrade:
			CavanAndroid.showToast(this, R.string.text_upgrade_start);
			if (otaUpgrade()) {
				CavanAndroid.showToast(this, R.string.text_upgrade_successfull);
			} else {
				CavanAndroid.showToast(this, R.string.text_upgrade_failed);
			}
			break;
		}
	}

	@Override
	public boolean onLongClick(View v) {
		switch (v.getId()) {
		case R.id.editTextRecv:
			mEditTextRecv.setText(new String());
			break;
		}

		return false;
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.eLog("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (requestCode == BLE_SCAN_RESULT && resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			}

			try {
				mBleUart = new CavanBleUart(device) {

					@Override
					protected void onConnectionStateChange(boolean connected) {
						if (connected) {
						} else {
							CavanBleScanActivity.show(MainActivity.this, BLE_SCAN_RESULT);
						}
					}
				};

				if (!mBleUart.connect()) {
					finish();
				}

				mBleUart.setDataListener(new CavanBleDataListener() {

					@Override
					public void onDataReceived(byte[] data) {
						String text = new String(data);
						CavanAndroid.eLog("onDataReceived: " + text);
						mHandler.obtainMessage(EVENT_DATA_RECEIVED, text).sendToTarget();
					}
				});
			} catch (Exception e) {
				e.printStackTrace();
				finish();
			}
		} else {
			finish();
		}
	}
}
