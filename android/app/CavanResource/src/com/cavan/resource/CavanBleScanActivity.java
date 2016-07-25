package com.cavan.resource;

import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleDevice;
import com.cavan.android.CavanBleDeviceAdapter;
import com.cavan.android.CavanBleDeviceView;
import com.cavan.android.CavanBleScanner;

public class CavanBleScanActivity extends Activity {

	private CavanBleScanner mScanner;
	private CavanBleDeviceAdapter mAdapter;

	protected void onScanComplete(CavanBleDevice device) {
		mScanner.stopScan();
		CavanAndroid.logE("onScanComplete: " + device);

		Intent intent = new Intent();
		intent.putExtra("device", device.getDevice());
		setResult(RESULT_OK, intent);
		finish();
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			mScanner.setAutoSelect(0);
		}

		return super.onTouchEvent(event);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.ble_scanner);

		Intent intent = getIntent();
		String name = intent.getStringExtra("name");
		UUID[] uuids = (UUID[]) intent.getSerializableExtra("uuids");

		ListView view = (ListView) findViewById(R.id.listViewDevices);
		mAdapter = new CavanBleDeviceAdapter(view) {

			@Override
			protected void onItemClicked(CavanBleDeviceView view) {
				onScanComplete(view.getDevice());
			}
		};

		BluetoothManager manager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
		mScanner = new CavanBleScanner(manager.getAdapter()) {

			@Override
			protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {
				mAdapter.setDevices(devices, device);
			}

			@Override
			protected void onAutoSelected(CavanBleDevice device) {
				onScanComplete(device);
			}
		};

		if (uuids != null || name != null) {
			mScanner.setAutoSelect(5000);
		}

		mScanner.startScan(uuids, name);
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

	public static void show(Activity activity, int requestCode, UUID[] uuids, String name) {
		if (activity.isDestroyed()) {
			return;
		}

		Intent intent = getIntent(activity);

		if (uuids != null) {
			intent.putExtra("uuids", uuids);
		}

		if (name != null) {
			intent.putExtra("name", name);
		}

		activity.startActivityForResult(intent, requestCode);
	}

	public static void show(Activity activity, String name, UUID[] uuids) {
		show(activity, 0, uuids, name);
	}

	public static void show(Activity activity, String name) {
		show(activity, 0, null, name);
	}

	public static void show(Activity activity, UUID[] uuids) {
		show(activity, 0, uuids, null);
	}

	public static void show(Activity activity) {
		show(activity, 0, null, null);
	}

	public static void show(Activity activity, int requestCode) {
		show(activity, requestCode, null, null);
	}
}
