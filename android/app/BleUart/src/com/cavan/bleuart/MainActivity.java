package com.cavan.bleuart;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.cavanutils.CavanGattCharacteristic;
import com.cavan.cavanutils.CavanHexFile;
import com.cavan.cavanutils.CavanUtils;

@SuppressLint("HandlerLeak")
public class MainActivity extends Activity implements OnClickListener, LeScanCallback, OnLongClickListener {

    public static final UUID SERVICE_UUID	= UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb7");
    public static final UUID RX_UUID		= UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cba");
    public static final UUID TX_UUID		= UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb8");
    public static final UUID OTA_UUID		= UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cbb");

	private BluetoothManager mBluetoothManager;
	private BluetoothAdapter mBluetoothAdapter;

	private BluetoothGatt mBluetoothGatt;
	private BluetoothGattService mGattService;
	private CavanGattCharacteristic mCharacteristicTx = new CavanGattCharacteristic();
	private CavanGattCharacteristic mCharacteristicRx = new CavanGattCharacteristic();
	private CavanGattCharacteristic mCharacteristicOta = new CavanGattCharacteristic();

	private boolean mScanning;
	private boolean mConnected;
	private Button mButtonScan;
	private Button mButtonSend;
	private Button mButtonUpgrade;
	private EditText mEditTextSend;
	private EditText mEditTextRecv;
	private ListView mListViewDevices;
	private HashMap<String, MyBluetoothDevice> mHashMapDevices = new HashMap<String, MyBluetoothDevice>();
	private List<MyBluetoothDevice> mListDevices = new ArrayList<MyBluetoothDevice>();
	private MyBluetoothDevice mDeviceCurrent;
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
		private boolean mConnected;
		private boolean mDiscovered;
		private BluetoothGattCallback mBluetoothGattCallback = new BluetoothGattCallback() {

			@Override
			public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
				CavanUtils.logE("onConnectionStateChange: " + status + " => " + newState);

				switch (newState) {
				case BluetoothProfile.STATE_CONNECTED:
					mConnected = true;
					mDiscovered = false;
					updateText();
					gatt.discoverServices();
					break;

				case BluetoothProfile.STATE_DISCONNECTED:
					disconnect();
					break;
				}
			}

			@Override
			public void onServicesDiscovered(BluetoothGatt gatt, int status) {
				CavanUtils.logE("onServicesDiscovered: status = " + status);

				int serviceIndex = 0;

				for (BluetoothGattService service : mBluetoothGatt.getServices()) {
					CavanUtils.logE((++serviceIndex) + ". service = " + service.getUuid());

					int characteristicIndex = 0;

					for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
						CavanUtils.logE("\t" + (++characteristicIndex) + ". characteristic = " + characteristic.getUuid());

						int descriptorIndex = 0;

						for (BluetoothGattDescriptor descriptor : characteristic.getDescriptors()) {
							CavanUtils.logE("\t\t" + (++descriptorIndex) + " .descriptor = " + descriptor.getUuid());
						}
					}
				}

				if (linkService()) {
					mDiscovered = true;
					updateText();
				} else {
					disconnect();
				}

				super.onServicesDiscovered(gatt, status);
			}

			@Override
			public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
				CavanUtils.logE("onCharacteristicRead: characteristic = " + characteristic.getUuid() + ", status = " + status);
				super.onCharacteristicRead(gatt, characteristic, status);
			}

			@Override
			public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
				CavanUtils.logE("onCharacteristicWrite: characteristic = " + characteristic.getUuid() + ", status = " + status);
				setWriteStatus(characteristic, status);
				super.onCharacteristicWrite(gatt, characteristic, status);
			}

			@Override
			public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
				String text = new String(characteristic.getValue());
				CavanUtils.logE("onCharacteristicChanged: text = " + text);
				mEditTextRecv.append(text);
				super.onCharacteristicChanged(gatt, characteristic);
			}

			@Override
			public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
				CavanUtils.logE("onDescriptorRead: descriptor = " + descriptor.getUuid() + ", status = " + status);
				super.onDescriptorRead(gatt, descriptor, status);
			}

			@Override
			public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
				CavanUtils.logE("onDescriptorWrite: descriptor = " + descriptor.getUuid() + ", status = " + status);
				super.onDescriptorWrite(gatt, descriptor, status);
			}

			@Override
			public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
				CavanUtils.logE("onReliableWriteCompleted: " + ", status = " + status);
				super.onReliableWriteCompleted(gatt, status);
			}

			@Override
			public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
				CavanUtils.logE("onReadRemoteRssi: rssi = " + rssi + ", status = " + status);
				super.onReadRemoteRssi(gatt, rssi, status);
			}
		};

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
				button = new Button(MainActivity.this);
			}

			updateText(button);
			button.setOnClickListener(this);

			return button;
		}

		public void updateText(Button button) {
			CavanUtils.logE("mConnected = " + mConnected + ", mDiscovered = " + mDiscovered);

			int color;

			if (mConnected) {
				if (mDiscovered) {
					color = Color.GREEN;
				} else {
					color = Color.BLUE;
				}
			} else {
				color = Color.RED;
			}

			button.setTextColor(color);
			button.setText(toString());
		}

		public void updateTextSync() {
			Button button = (Button) mListViewDevices.getChildAt(mIndex);
			if (button != null) {
				updateText(button);
			}

			if (mDeviceCurrent == this) {
				setConnectState(mConnected && mDiscovered);
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

		public boolean linkService() {
			mGattService = mBluetoothGatt.getService(SERVICE_UUID);
			if (mGattService == null) {
				return false;
			}

			if (!mCharacteristicTx.init(mBluetoothGatt, mGattService, TX_UUID)) {
				return false;
			}

			if (!mCharacteristicRx.init(mBluetoothGatt, mGattService, RX_UUID)) {
				return false;
			}

			mCharacteristicOta.init(mBluetoothGatt, mGattService, OTA_UUID);

			return true;
		}

		public void connect() {
			closeGatt();
			setScanState(false);

			mBluetoothGatt = mDevice.connectGatt(MainActivity.this, false, mBluetoothGattCallback);
		}

		public void disconnect() {
			closeGatt();
			mConnected = false;
			mDiscovered = false;
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
			mDeviceCurrent = this;

			if (mConnected && mBluetoothGatt != null) {
				closeGatt();
				mConnected = false;
				updateTextSync();
			} else {
				connect();
			}
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

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mBluetoothManager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
		mBluetoothAdapter = mBluetoothManager.getAdapter();

		mEditTextRecv = (EditText) findViewById(R.id.editTextRecv);
		mEditTextRecv.setOnLongClickListener(this);
		mButtonScan = (Button) findViewById(R.id.buttonScan);
		mButtonScan.setOnClickListener(this);

		mEditTextSend = (EditText) findViewById(R.id.editTextSend);
		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);
		mButtonSend.setEnabled(false);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);
		mButtonUpgrade.setEnabled(false);

		mListViewDevices = (ListView) findViewById(R.id.listViewDevices);
		mListViewDevices.setAdapter(mDeviceAdapter);
	}

	public void closeGatt() {
		if (mBluetoothGatt != null) {
			mBluetoothGatt.disconnect();
			mBluetoothGatt.close();
			mBluetoothGatt = null;
		}
	}

	public boolean setWriteStatus(BluetoothGattCharacteristic characteristic, int status) {
		if (mCharacteristicRx.setWriteStatus(characteristic, status)) {
			return true;
		}

		if (mCharacteristicOta.setWriteStatus(characteristic, status)) {
			return true;
		}

		return false;
	}

	public boolean sendText(String text) {
		return mCharacteristicRx.writeData(text.getBytes(), true);
	}

	@SuppressWarnings("deprecation")
	private void setScanState(boolean enable) {
		if (enable) {
			mBluetoothAdapter.enable();
			mHashMapDevices.clear();

			closeGatt();

			mDeviceAdapter.updateDeviceList();

			mBluetoothAdapter.startLeScan(this);
			mButtonScan.setText(R.string.text_scan_stop);

			mHandler.postDelayed(new Runnable() {

				@Override
				public void run() {
					setScanState(false);
				}
			}, 10 * 1000);
		} else {
			mBluetoothAdapter.stopLeScan(this);
			mButtonScan.setText(R.string.text_scan_start);
		}

		mScanning = enable;
	}

	private void switchScanState() {
		setScanState(!mScanning);
	}

	private boolean otaUpgrade() {
		CavanHexFile file = new CavanHexFile("/data/local/tmp/dialog.hex");
		byte[] bytes = file.parse();
		if (bytes == null) {
			CavanUtils.logE("Failed to parse hex file");
			return false;
		}

		int length = (bytes.length + 7) & (~0x07);
		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };
		if (!mCharacteristicOta.writeData(header, true)) {
			return false;
		}

		return mCharacteristicOta.writeData(bytes, 128);
	}

	private void setConnectState(boolean connected) {
		if (mConnected == connected) {
			return;
		}

		mConnected = connected;

		mHandler.post(new Runnable() {

			@Override
			public void run() {
				mButtonSend.setEnabled(mConnected);
				mButtonUpgrade.setEnabled(mConnected && mCharacteristicOta != null);
			}
		});
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonScan:
			switchScanState();
			break;

		case R.id.buttonSend:
			sendText(mEditTextSend.getText().toString());
			break;

		case R.id.buttonUpgrade:
			CavanUtils.showToast(this, R.string.text_upgrade_start);
			if (otaUpgrade()) {
				CavanUtils.showToast(this, R.string.text_upgrade_successfull);
			} else {
				CavanUtils.showToast(this, R.string.text_upgrade_failed);
			}
			break;
		}
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

	@Override
	public boolean onLongClick(View v) {
		switch (v.getId()) {
		case R.id.editTextRecv:
			mEditTextRecv.setText(new String());
			break;
		}

		return false;
	}
}
