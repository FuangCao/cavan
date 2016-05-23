package com.cavan.bluetoothsetting;

import java.util.Collection;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import com.android.settings.bluetooth.BluetoothCallback;
import com.android.settings.bluetooth.CachedBluetoothDevice;
import com.android.settings.bluetooth.CachedBluetoothDevice.Callback;
import com.android.settings.bluetooth.LocalBluetoothAdapter;
import com.android.settings.bluetooth.LocalBluetoothManager;
import com.cavan.cavanutils.CavanUtils;

public class MainActivity extends Activity implements BluetoothCallback, OnClickListener {

	private LocalBluetoothAdapter mLocalAdapter;
	private LocalBluetoothManager mLocalManager;

	private Button mButtonScan;
	private ListView mListViewDevices;

	class DeviceView extends Button implements Callback {

		private CachedBluetoothDevice mDevice;

		public DeviceView(Context context, CachedBluetoothDevice device) {
			super(context);

			mDevice = device;

			// setTextColor(Color.BLACK);
			setTextSize(12);
			setGravity(Gravity.CENTER_VERTICAL);

			mDevice.registerCallback(this);

			onDeviceAttributesChanged();
		}

		@Override
		public void onDeviceAttributesChanged() {
			CavanUtils.logE("onDeviceAttributesChanged");

			String name = mDevice.getDevice().getName();
			String address = mDevice.getDevice().getAddress();

			StringBuilder builder = new StringBuilder();

			builder.append(address);

			if (name != null && name.length() > 0) {
				builder.append(" - ");
				builder.append(name);
			}

			builder.append(" - ");

			if (mDevice.isConnected()) {
				builder.append("CONNECTED");
			} else {
				switch (mDevice.getBondState()) {
				case BluetoothDevice.BOND_BONDED:
					builder.append("BONDED");
					break;
				case BluetoothDevice.BOND_BONDING:
					builder.append("BONDING");
					break;
				default:
					builder.append("NOTBOND");
					break;
				}
			}

			setText(builder.toString());
			setEnabled(true);
		}

		public void onClick() {
			if (mDevice.isConnected()) {
				Toast.makeText(getApplicationContext(), "Disconnect " + mDevice.getName(), Toast.LENGTH_SHORT).show();
				mDevice.disconnect();
			} else if (mDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
				Toast.makeText(getApplicationContext(), "Connect to " + mDevice.getName(), Toast.LENGTH_SHORT).show();
				mDevice.connect(true);
			} else if (mDevice.getBondState() == BluetoothDevice.BOND_NONE) {
				Toast.makeText(getApplicationContext(), "Pair to " + mDevice.getName(), Toast.LENGTH_SHORT).show();
				mDevice.startPairing();
			}
		}

		public boolean onLongClick() {
			if (mDevice.isConnected()) {
				Toast.makeText(getApplicationContext(), "Disconnect " + mDevice.getName(), Toast.LENGTH_SHORT).show();
				mDevice.disconnect();
			} else if (mDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
				Toast.makeText(getApplicationContext(), "Unpair " + mDevice.getName(), Toast.LENGTH_SHORT).show();
				mDevice.unpair();
			}

			return false;
		}
	}

	class DeviceListAdapter extends BaseAdapter implements OnClickListener, OnLongClickListener {

		private CachedBluetoothDevice[] mDevices;

		DeviceListAdapter() {
			Collection<CachedBluetoothDevice> devices = mLocalManager.getCachedDeviceManager().getCachedDevicesCopy();
			if (devices != null) {
				int index = 0;

				mDevices = new CachedBluetoothDevice[devices.size()];
				for (CachedBluetoothDevice device : devices) {
					mDevices[index++] = device;
				}
			}
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			if (convertView != null && convertView instanceof DeviceView) {
				return convertView;
			}

			DeviceView view = new DeviceView(getApplicationContext(), mDevices[position]);
			view.setOnClickListener(this);
			view.setOnLongClickListener(this);
			return view;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public int getCount() {
			return mDevices.length;
		}

		@Override
		public void onClick(View v) {
			CavanUtils.logE("onClick: view = " + v);

			((DeviceView) v).onClick();
		}

		@Override
		public boolean onLongClick(View v) {
			CavanUtils.logE("onLongClick: view = " + v);

			return ((DeviceView) v).onLongClick();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonScan = (Button) findViewById(R.id.buttonScan);
		mButtonScan.setOnClickListener(this);
		mListViewDevices = (ListView) findViewById(R.id.listViewDevices);

		mLocalManager = LocalBluetoothManager.getInstance(this);

		if (mLocalManager != null) {
			mLocalAdapter = mLocalManager.getBluetoothAdapter();
			mLocalManager.getEventManager().registerCallback(this);
			updateContent(mLocalAdapter.getBluetoothState());
		}
	}

	private void startScan() {
		mLocalAdapter.enable();
		mLocalManager.getCachedDeviceManager().clearNonBondedDevices();
		mLocalAdapter.startScanning(true);

		Toast.makeText(getApplicationContext(), "Scanning", Toast.LENGTH_SHORT).show();
		updateContent(-1);
	}

	private void updateContent(int state) {
		if (state < 0) {
			state = mLocalAdapter.getBluetoothState();
		}

		switch (state) {
		case BluetoothAdapter.STATE_ON:
			mListViewDevices.setAdapter(new DeviceListAdapter());
			break;

		case BluetoothAdapter.STATE_TURNING_OFF:
			break;

		case BluetoothAdapter.STATE_OFF:
			mListViewDevices.setAdapter(null);
			break;

		case BluetoothAdapter.STATE_TURNING_ON:
			break;
		}
	}

	@Override
	public void onBluetoothStateChanged(int state) {
		CavanUtils.logE("onBluetoothStateChanged: state = " + state);
		updateContent(state);
	}

	@Override
	public void onDeviceAdded(CachedBluetoothDevice device) {
		CavanUtils.logE("onDeviceAdded: device = " + device);
		updateContent(-1);
	}

	@Override
	public void onDeviceBondStateChanged(CachedBluetoothDevice device, int state) {
		CavanUtils.logE("onDeviceBondStateChanged: device = " + device + ", state = " + state);
		updateContent(-1);

	}

	@Override
	public void onDeviceDeleted(CachedBluetoothDevice device) {
		CavanUtils.logE("onDeviceDeleted: device = " + device);
		updateContent(-1);
	}

	@Override
	public void onScanningStateChanged(boolean state) {
		CavanUtils.logE("onScanningStateChanged: state = " + state);
	}

	@Override
	public void onClick(View v) {
		CavanUtils.logE("onClick: view = " + v);
		startScan();
	}
}
