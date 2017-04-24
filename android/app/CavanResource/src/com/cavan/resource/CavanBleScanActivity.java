package com.cavan.resource;

import java.io.IOException;
import java.util.UUID;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanAndroidListeners.CavanKeyboardViewListener;
import com.cavan.android.CavanAndroidListeners.CavanQrCodeViewListener;
import com.cavan.android.CavanBleDevice;
import com.cavan.android.CavanBleDeviceAdapter;
import com.cavan.android.CavanBleScanner;
import com.cavan.android.CavanMacAddressView;
import com.cavan.android.CavanQrCodeView;
import com.cavan.java.CavanMacAddress;
import com.google.zxing.Result;

@SuppressLint("ClickableViewAccessibility")
@SuppressWarnings("deprecation")
public class CavanBleScanActivity extends CavanBleActivity implements OnClickListener, Callback, CavanQrCodeViewListener {

	private UUID[] mUuids;
	private String[] mAddresses;
	private String[] mNames;

	private boolean mConnEnable;
	private CavanBleScanner mScanner;
	private CavanBleDeviceAdapter mAdapter;

	private ListView mListView;
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private Button mButtonQrCodeScan;
	private CavanQrCodeView mQrCodeView;
	private CavanMacAddressView mMacView;
	private CavanKeyboardViewNumber mKeyboardView;

	protected void onScanComplete(CavanBleDevice device) {
		CavanAndroid.dLog("onScanComplete: " + device);
		finishScan(device.getDevice());
	}

	public void finishScan(BluetoothDevice device) {
		mScanner.stopScan();
		mQrCodeView.closeCamera();

		Intent intent = new Intent();
		intent.putExtra("device", device);
		setResult(RESULT_OK, intent);
		finish();
	}

	public void startScan() {
		if (mScanner != null) {
			mScanner.startScan(mUuids, mNames, mAddresses);
		}
	}

	public void setConnEnable(boolean enable) {
		if (mQrCodeView.getVisibility() == View.VISIBLE) {
			mConnEnable = true;
		} else {
			mConnEnable = enable;
		}

		if (mConnEnable) {
			mButtonQrCodeScan.setText(R.string.connect);
		} else {
			mButtonQrCodeScan.setText(R.string.qrcode_scan);
		}
	}

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.ble_scanner);

		mKeyboardView = (CavanKeyboardViewNumber) findViewById(R.id.keyboardView);
		mKeyboardView.setInputEnable(false, 1000);

		mKeyboardView.setEventListener(new CavanKeyboardViewListener() {

			@Override
			public void onStopInput(EditText view) {
				mKeyboardView.onStopInput(view);
				setConnEnable(false);
			}

			@Override
			public void onStartInput(EditText view) {
				mKeyboardView.onStartInput(view);

				mScanner.setAutoSelect(0);

				CavanMacAddress address = mMacView.getAddress();
				setConnEnable(address.notZero());
			}

			@Override
			public void onEditTextClick(EditText view) {
				mKeyboardView.onEditTextClick(view);
			}

			@Override
			public void onEditTextLongClick(EditText view) {
				mKeyboardView.onEditTextLongClick(view);
			}
		});

		mMacView = (CavanMacAddressView) findViewById(R.id.macAddressView);
		mMacView.setKeyboardView(mKeyboardView, CavanKeyboardViewNumber.KEYBOARD_RADIX16);
		mMacView.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				CavanMacAddress address = mMacView.getAddress();
				setConnEnable(address.notZero());
			}
		});

		mQrCodeView = (CavanQrCodeView) findViewById(R.id.qrCodeView);
		mQrCodeView.setEventListener(this);

		mSurface = (SurfaceView) findViewById(R.id.surfaceViewQrCode);
		mHolder = mSurface.getHolder();
		mHolder.addCallback(this);

		mListView = (ListView) findViewById(R.id.listViewDevices);
		mListView.setOnTouchListener(new View.OnTouchListener() {

			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN && mScanner != null) {
					mKeyboardView.stopInput();
					mScanner.setAutoSelect(0);
				}

				return false;
			}
		});

		mButtonQrCodeScan = (Button) findViewById(R.id.buttonQrCodeScan);
		mButtonQrCodeScan.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				mScanner.setAutoSelect(0);

				if (mConnEnable) {
					BluetoothDevice device = mMacView.getBluetoothDevice(mScanner.getAdapter());
					if (device != null) {
						finishScan(device);
					}
				} else if (mQrCodeView.getVisibility() != View.VISIBLE) {
					if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
						mSurface.setVisibility(View.VISIBLE);
					} else {
						requestPermissions(new String[] { Manifest.permission.CAMERA }, 0);
					}
				}

				mKeyboardView.stopInput();
			}
		});

		Intent intent = getIntent();
		mNames = intent.getStringArrayExtra("names");
		mAddresses = intent.getStringArrayExtra("addresses");

		Object[] objects = (Object[]) intent.getSerializableExtra("uuids");
		if (objects != null && objects.length > 0) {
			mUuids = new UUID[objects.length];

			for (int i = 0; i < objects.length; i++) {
				mUuids[i] = (UUID) objects[i];
			}
		}

		mAdapter = new CavanBleDeviceAdapter(mListView) {

			@Override
			protected void selectDevice(View view, CavanBleDevice device) {
				onScanComplete(device);
			}
		};

		mScanner = new CavanBleScanner(this) {

			@Override
			protected void onScanStarted() {
				CavanAndroid.showToast(getApplicationContext(), R.string.scanning);
			}

			@Override
			protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {
				CavanBleScanActivity.this.mAdapter.setDevices(devices, device);
			}

			@Override
			protected void onAutoSelected(CavanBleDevice device) {
				onScanComplete(device);
			}
		};

		if (mAddresses != null) {
			mScanner.setAutoSelect(1000);
		} else if (mUuids != null || mNames != null) {
			mScanner.setAutoSelect(5000);
		}

		if (mScanner.isAdapterEnabled()) {
			startScan();
		} else {
			AlertDialog.Builder builder = new AlertDialog.Builder(CavanBleScanActivity.this);
			builder.setMessage(R.string.bluetooth_open_prompt);
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

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		CavanAndroid.pLog("requestCode = " + requestCode);

		for (int i = 0; i < permissions.length; i++) {
			CavanAndroid.dLog("permission = " + permissions[i] + ", grantResult = " + grantResults[i]);

			if (Manifest.permission.CAMERA.equals(permissions[i])) {
				if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
					mSurface.setVisibility(View.VISIBLE);
				} else {
					CavanAndroid.showToast(this, R.string.request_camera_permission_failed);
				}
			}
		}
	}

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanBleScanActivity.class);
	}

	public static void show(Context context) {
		context.startActivity(getIntent(context));
	}

	public static void show(Activity activity, int requestCode, UUID[] uuids, String[] names, String[] addresses) {
		if (Build.VERSION.SDK_INT > Build.VERSION_CODES.JELLY_BEAN && activity.isDestroyed()) {
			return;
		}

		Intent intent = getIntent(activity);

		if (uuids != null && uuids.length > 0) {
			intent.putExtra("uuids", uuids);
		}

		if (names != null && names.length > 0) {
			intent.putExtra("names", names);
		}

		if (addresses != null && addresses.length > 0) {
			intent.putExtra("addresses", addresses);
		}

		activity.startActivityForResult(intent, requestCode);
	}

	public static void show(Activity activity, UUID[] uuids, String[] names, String[] addresses) {
		show(activity, 0, uuids, names, addresses);
	}

	public static void show(Activity activity, UUID[] uuids, String[] names) {
		show(activity, uuids, names, null);
	}

	public static void show(Activity activity, String[] names, String[] addresses) {
		show(activity, null, names, addresses);
	}

	public static void show(Activity activity, String[] names) {
		show(activity, null, names, null);
	}

	public static void show(Activity activity, UUID[] uuids) {
		show(activity, uuids, null);
	}

	public static void show(Activity activity) {
		show(activity, null, null, null);
	}

	public static void show(Activity activity, int requestCode) {
		show(activity, requestCode, null, null, null);
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

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// CavanAndroid.pLog();
		mQrCodeView.openCamera(mSurface.getWidth(), mSurface.getHeight());
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		// CavanAndroid.pLog();
		mQrCodeView.setSurfaceSize(width, height);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// CavanAndroid.pLog();
		mQrCodeView.closeCamera();
	}

	@Override
	public boolean onDecodeComplete(Result result) {
		String text = result.getText();

		BluetoothDevice device = mScanner.getRemoteDevice(text);
		if (device != null) {
			finishScan(device);
			return true;
		}

		CavanAndroid.showToast(getApplicationContext(), R.string.mac_address_format_fault, text);

		return false;
	}

	@Override
	public void onDecodeStart() {
		// CavanAndroid.pLog();
	}

	@Override
	public boolean doCameraInit(Camera camera) {
		// CavanAndroid.pLog();

		try {
			camera.setPreviewDisplay(mHolder);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	@Override
	public void onCameraOpened(Camera camera) {
		// CavanAndroid.pLog();

		if (camera != null) {
			mQrCodeView.setVisibility(View.VISIBLE);
			mQrCodeView.startPreview();
			setConnEnable(true);
		} else {
			CavanAndroid.showToast(this, R.string.open_camera_failed);
		}
	}
}
