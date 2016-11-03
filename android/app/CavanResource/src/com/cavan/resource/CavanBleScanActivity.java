package com.cavan.resource;

import java.util.UUID;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleDevice;
import com.cavan.android.CavanBleDeviceAdapter;
import com.cavan.android.CavanBleScanner;

public class CavanBleScanActivity extends Activity {

	private CavanBleScanner mScanner;
	private CavanBleDeviceAdapter mAdapter;

	protected void onScanComplete(CavanBleDevice device) {
		mScanner.stopScan();
		CavanAndroid.dLog("onScanComplete: " + device);

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
		String[] names = intent.getStringArrayExtra("names");
		UUID[] uuids = (UUID[]) intent.getSerializableExtra("uuids");

		ListView view = (ListView) findViewById(R.id.listViewDevices);
		mAdapter = new CavanBleDeviceAdapter(view) {

			@Override
			protected void selectDevice(View view, CavanBleDevice device) {
				onScanComplete(device);
			}
		};

		mScanner = new CavanBleScanner(this) {

			@Override
			protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {
				CavanBleScanActivity.this.mAdapter.setDevices(devices, device);
			}

			@Override
			protected void onAutoSelected(CavanBleDevice device) {
				onScanComplete(device);
			}
		};

		if (uuids != null || names != null) {
			mScanner.setAutoSelect(3000);
		}

		mScanner.startScan(uuids, names);
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

	public static void show(Activity activity, int requestCode, UUID[] uuids, String[] names) {
		if (activity.isDestroyed()) {
			return;
		}

		Intent intent = getIntent(activity);

		if (uuids != null) {
			intent.putExtra("uuids", uuids);
		}

		if (names != null) {
			intent.putExtra("names", names);
		}

		activity.startActivityForResult(intent, requestCode);
	}

	public static void show(Activity activity, String[] names, UUID[] uuids) {
		show(activity, 0, uuids, names);
	}

	public static void show(Activity activity, String[] names) {
		show(activity, 0, null, names);
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
