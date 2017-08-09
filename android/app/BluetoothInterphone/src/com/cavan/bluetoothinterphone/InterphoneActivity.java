package com.cavan.bluetoothinterphone;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanDaemonThread;

public class InterphoneActivity extends Activity implements OnClickListener {

	private BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

	private Button mButtonStart;
	private Button mButtonStop;
	private Button mButtonConn;
	private Button mButtonDisconn;

	private Spinner mSpinnerDevices;
	private BaseAdapter mDeviceAdapter = new BaseAdapter() {

		private BluetoothDevice[] mBondedDevices = new BluetoothDevice[0];

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(InterphoneActivity.this);
			}

			BluetoothDevice device = mBondedDevices[position];
			StringBuilder builder = new StringBuilder();

			builder.append(device.getAddress());

			String name = device.getName();
			if (name != null && name.length() > 0) {
				builder.append(" - ").append(name);
			}

			view.setText(builder);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mBondedDevices[position];
		}

		@Override
		public int getCount() {
			return mBondedDevices.length;
		}

		@Override
		public void notifyDataSetChanged() {
			Set<BluetoothDevice> devices = mBluetoothAdapter.getBondedDevices();
			mBondedDevices = new BluetoothDevice[devices.size()];
			devices.toArray(mBondedDevices);
			super.notifyDataSetChanged();
		}
	};

	private IInterphoneService mService;
	private ServiceConnection mServiceConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			CavanAndroid.pLog();
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			CavanAndroid.pLog();
			mService = IInterphoneService.Stub.asInterface(service);
		}
	};

	private BluetoothSocket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;

	private CavanDaemonThread mConnThread = new CavanDaemonThread() {

		private int mConnTimes;

		@Override
		public synchronized void startDaemon() {
			mConnTimes = 0;
			super.startDaemon();
		}

		@Override
		protected void onDaemonStop() {
			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		@Override
		protected boolean runDaemon() {
			BluetoothDevice device = (BluetoothDevice) mSpinnerDevices.getSelectedItem();
			CavanAndroid.dLog("device = " + device);

			for (ParcelUuid uuid : device.getUuids()) {
				CavanAndroid.dLog("uuid: " + uuid);
			}

			BluetoothSocket socket = null;
			InputStream istream = null;
			OutputStream ostream = null;

			try {
				socket = device.createInsecureRfcommSocketToServiceRecord(InterphoneService.SERVICE_UUID);
				if (socket == null) {
					return false;
				}

				socket.connect();

				istream = socket.getInputStream();
				if (istream == null) {
					return false;
				}

				ostream = socket.getOutputStream();
				if (ostream == null) {
					return false;
				}

				synchronized (this) {
					mSocket = socket;
					mInputStream = istream;
					mOutputStream = ostream;
				}

				try {
					double total = 0;
					long time = System.currentTimeMillis();

					while (true) {
						byte[] bytes = new byte[1024];

						ostream.write(bytes);

						int length = istream.read(bytes);
						if (length < 0) {
							break;
						}

						total += length;

						double speed = ((total * 1000 / (System.currentTimeMillis() - time)) / 1024);
						CavanAndroid.dLog("speed = " + speed + " KB/S");
					}
				} catch (Exception e) {
					e.printStackTrace();
				}

				mConnTimes = 0;
			} catch (IOException e) {
				if (isSuspended()) {
					return true;
				}

				e.printStackTrace();

				int delay;

				if (++mConnTimes < 15) {
					delay = 1 << mConnTimes;
				} else {
					delay = 1 << 15;
				}

				CavanAndroid.dLog("delay = " + delay);

				synchronized (this) {
					try {
						wait(delay);
					} catch (InterruptedException e1) {
						e1.printStackTrace();
					}
				}
			} finally {
				synchronized (this) {
					mSocket = null;
					mInputStream = null;
					mOutputStream = null;
				}

				if (socket != null) {
					try {
						socket.close();
					} catch (IOException e1) {
						e1.printStackTrace();
					}
				}

				if (istream != null) {
					try {
						istream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				if (ostream != null) {
					try {
						ostream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}

			return false;
		}
	};

	public InputStream getInputStream() {
		return mInputStream;
	}

	public OutputStream getOutputStream() {
		return mOutputStream;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_interphone);

		mSpinnerDevices = (Spinner) findViewById(R.id.spinnerDevices);
		mSpinnerDevices.setAdapter(mDeviceAdapter);
		mDeviceAdapter.notifyDataSetChanged();

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setOnClickListener(this);

		mButtonStop = (Button) findViewById(R.id.buttonStop);
		mButtonStop.setOnClickListener(this);

		mButtonConn = (Button) findViewById(R.id.buttonConn);
		mButtonConn.setOnClickListener(this);

		mButtonDisconn = (Button) findViewById(R.id.buttonDisconn);
		mButtonDisconn.setOnClickListener(this);

		Intent service = new Intent(this, InterphoneService.class);
		startService(service);
		bindService(service, mServiceConnection, 0);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonStart:
			if (mService != null) {
				try {
					mService.start();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
			break;

		case R.id.buttonStop:
			if (mService != null) {
				try {
					mService.stop();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
			break;

		case R.id.buttonConn:
			mConnThread.startDaemon();
			break;

		case R.id.buttonDisconn:
			mConnThread.stopDaemon();
			break;
		}
	}
}
