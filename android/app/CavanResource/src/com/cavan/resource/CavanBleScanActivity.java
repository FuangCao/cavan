package com.cavan.resource;

import android.app.Activity;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.widget.ListView;

import com.cavan.android.CavanBleDevice;
import com.cavan.android.CavanBleDeviceAdapter;
import com.cavan.android.CavanBleDeviceView;
import com.cavan.android.CavanBleScanner;

public class CavanBleScanActivity extends Activity {

	private CavanBleScanner mScanner;
	private CavanBleDeviceAdapter mAdapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.ble_scanner);

		ListView view = (ListView) findViewById(R.id.listViewDevices);
		mAdapter = new CavanBleDeviceAdapter(view) {

			@Override
			protected void onItemClicked(CavanBleDeviceView view) {
				mScanner.stopScan();

				Intent intent = new Intent();
				intent.putExtra("device", view.getDevice().getDevice());
				setResult(RESULT_OK, intent);
				finish();
			}
		};

		BluetoothManager manager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
		mScanner = new CavanBleScanner(manager.getAdapter()) {

			@Override
			protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {
				mAdapter.setDevices(devices, device);
			}
		};

		mScanner.startScan();
	}

	@Override
	protected void onDestroy() {
		mScanner.stopScan();
		super.onDestroy();
	}

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanBleScanActivity.class);
	}

	public static void show(Context context) {
		context.startActivity(getIntent(context));
	}

	public static void show(Activity activity, int requestCode) {
		activity.startActivityForResult(getIntent(activity), requestCode);
	}

	public static void show(Activity activity) {
		show(activity, 0);
	}
}
