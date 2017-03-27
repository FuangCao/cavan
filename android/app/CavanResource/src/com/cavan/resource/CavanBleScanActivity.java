package com.cavan.resource;

import java.util.UUID;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleDevice;
import com.cavan.android.CavanBleDeviceAdapter;
import com.cavan.android.CavanBleScanner;

public class CavanBleScanActivity extends CavanBleActivity implements OnClickListener {

	private UUID[] mUuids;
	private String[] mNames;
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

	public void startScan() {
		if (mScanner != null) {
			mScanner.startScan(mUuids, mNames);
			CavanAndroid.showToast(this, R.string.text_scanning);
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_DOWN && mScanner != null) {
			mScanner.setAutoSelect(0);
		}

		return super.onTouchEvent(event);
	}

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.ble_scanner);

		Intent intent = getIntent();
		mNames = intent.getStringArrayExtra("names");

		Object[] objects = (Object[]) intent.getSerializableExtra("uuids");
		if (objects != null && objects.length > 0) {
			mUuids = new UUID[objects.length];

			for (int i = 0; i < objects.length; i++) {
				mUuids[i] = (UUID) objects[i];
			}
		}

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

		if (mUuids != null || mNames != null) {
			mScanner.setAutoSelect(3000);
		}

		if (mScanner.isAdapterEnabled()) {
			startScan();
		} else {
			AlertDialog.Builder builder = new AlertDialog.Builder(CavanBleScanActivity.this);
			builder.setMessage(R.string.text_bluetooth_open_prompt);
			builder.setCancelable(false);
			builder.setPositiveButton(R.string.open, CavanBleScanActivity.this);
			builder.setNegativeButton(R.string.exit, CavanBleScanActivity.this);

			Dialog dialog = builder.create();
			dialog.show();
		}
	}

	@Override
	protected void onDestroy() {
		if (mScanner != null) {
			mScanner.stopScan();
		}

		super.onDestroy();
	}

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanBleScanActivity.class);
	}

	public static void show(Context context) {
		context.startActivity(getIntent(context));
	}

	public static void show(Activity activity, int requestCode, UUID[] uuids, String[] names) {
		if (Build.VERSION.SDK_INT > Build.VERSION_CODES.JELLY_BEAN && activity.isDestroyed()) {
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

	@Override
	public void onClick(DialogInterface dialog, int which) {
		if (which == DialogInterface.BUTTON_POSITIVE) {
			mScanner.setAdapterEnable(true);
			startScan();
		} else {
			finish();
		}
	}
}
