package com.cavan.cavanutils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;

public class CavanBleScanner extends Activity implements LeScanCallback {

	private BluetoothManager mBluetoothManager;
	private BluetoothAdapter mBluetoothAdapter;

	private ListView mListViewDevices;
	private HashMap<String, MyBluetoothDevice> mHashMapDevices = new HashMap<String, MyBluetoothDevice>();
	private List<MyBluetoothDevice> mListDevices = new ArrayList<MyBluetoothDevice>();
	private DeviceAdapter mDeviceAdapter = new DeviceAdapter();

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			MyBluetoothDevice device = (MyBluetoothDevice) msg.obj;
			device.updateTextSync();
		}
	};

	class MyBluetoothDevice implements OnClickListener {
		private int mIndex;
		private int mRssi;
		private BluetoothDevice mDevice;

		public MyBluetoothDevice(BluetoothDevice device, int rssi) {
			super();
			mRssi = rssi;
			mDevice = device;
		}

		public Button createView(View convertView, int index) {
			Button button;

			mIndex = index;

			if (convertView != null) {
				button = (Button) convertView;
			} else {
				button = new Button(CavanBleScanner.this);
			}

			updateText(button);
			button.setOnClickListener(this);

			return button;
		}

		public void updateText(Button button) {
			button.setText(toString());
		}

		public void updateTextSync() {
			Button button = (Button) mListViewDevices.getChildAt(mIndex);
			if (button != null) {
				updateText(button);
			}
		}

		public void updateText() {
			Message message = mHandler.obtainMessage(0, this);
			message.sendToTarget();
		}

		public void setRssi(int rssi) {
			mRssi = rssi;
			updateText();
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder(mDevice.getAddress());
			String name = mDevice.getName();
			if (name != null) {
				builder.append(" - ");
				builder.append(name);
			}

			switch (mDevice.getBondState()) {
			case BluetoothDevice.BOND_BONDED:
				builder.append(" - ");
				builder.append("BONDED");
				break;

			case BluetoothDevice.BOND_BONDING:
				builder.append(" - ");
				builder.append("BONDING");
				break;
			}

			builder.append(", RSSI = " + mRssi);

			return builder.toString();
		}

		@Override
		public void onClick(View v) {
			Intent intent = new Intent();
			intent.putExtra("device", mDevice);
			setResult(RESULT_OK, intent);
			finish();
		}
	}

	class DeviceAdapter extends BaseAdapter {

		@Override
		public int getCount() {
			return mListDevices.size();
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			MyBluetoothDevice device = mListDevices.get(position);
			if (device == null) {
				return null;
			}

			return device.createView(convertView, position);
		}

		public void updateDeviceList() {
			mListDevices.clear();

			for (MyBluetoothDevice device : mHashMapDevices.values()) {
				mListDevices.add(device);
			}

			notifyDataSetChanged();
		}
	};

	public static boolean show(Activity activity, int requestCode) {
		if (activity.isDestroyed()) {
			return false;
		}

		Intent intent = new Intent(activity, CavanBleScanner.class);
		activity.startActivityForResult(intent, requestCode);

		return true;
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.ble_scanner);

		setTitle(R.string.text_scanning);

		mBluetoothManager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
		mBluetoothAdapter = mBluetoothManager.getAdapter();

		mListViewDevices = (ListView) findViewById(R.id.listViewDevices);
		mListViewDevices.setAdapter(mDeviceAdapter);

		mBluetoothAdapter.enable();
		mBluetoothAdapter.startLeScan(this);
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onDestroy() {
		mBluetoothAdapter.stopLeScan(this);
		super.onDestroy();
	}

	@Override
	public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
		String address = device.getAddress();
		MyBluetoothDevice myDevice = mHashMapDevices.get(address);
		if (myDevice == null) {
			myDevice = new MyBluetoothDevice(device, rssi);
			mHashMapDevices.put(address, myDevice);

			mHandler.post(new Runnable() {

				@Override
				public void run() {
					mDeviceAdapter.updateDeviceList();
				}
			});
		} else {
			myDevice.setRssi(rssi);
		}
	}
}
