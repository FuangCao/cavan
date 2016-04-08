package com.cavan.remotecontrol;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.List;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

@SuppressLint({ "HandlerLeak", "UseSparseArrays" })
public class MainActivity extends ActionBarActivity implements OnClickListener, OnItemSelectedListener {

	private static final String TAG = "Cavan";
	private static final int TCP_DD_VERSION = 0x20151223;
	private static final short TCP_DD_REQ_KEYPAD = 7;
	private static final short EVENT_TYPE_SYNC = 0;
	private static final short EVENT_TYPE_KEY = 1;
	private static final HashMap<Integer, Integer> sKeyMap = new HashMap<Integer, Integer>();

	static {
		sKeyMap.put(R.id.buttonBack, 158);
		sKeyMap.put(R.id.buttonDown, 108);
		sKeyMap.put(R.id.buttonEnter, 28);
		sKeyMap.put(R.id.buttonHome, 172);
		sKeyMap.put(R.id.buttonLeft, 105);
		sKeyMap.put(R.id.buttonMenu, 139);
		sKeyMap.put(R.id.buttonRight, 106);
		sKeyMap.put(R.id.buttonUp, 103);
		sKeyMap.put(R.id.buttonVolumeUp, 115);
		sKeyMap.put(R.id.buttonVolumeDown, 114);
		sKeyMap.put(R.id.buttonBrightUp, 225);
		sKeyMap.put(R.id.buttonBrightDown, 224);
	}

	private Socket mSocket;
	private OutputStream mOutputStream;
	private HashMap<Button, Integer> mButtonMap = new HashMap<Button, Integer>();

	private IDiscoveryService mDiscoveryService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mDiscoveryService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mDiscoveryService = IDiscoveryService.Stub.asInterface(service);

			try {
				mDiscoveryService.startDiscovery(0);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	};

	private Spinner mSpinner;
	private Button mButtonScan;
	private ScanResult mScanResult;
	private List<ScanResult> mScanResults;
	private Adapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(getApplicationContext());
			}

			view.setText(mScanResults.get(position).toShortString());
			view.setTextSize(28);
			view.setTextColor(Color.BLACK);

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
			return mScanResults.size();
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(DiscoveryService.ACTION_SCAN_RESULT_CHANGED)) {
				if (mDiscoveryService == null) {
					return;
				}

				try {
					mSpinner.setAdapter(null);
					mScanResults = mDiscoveryService.getScanResult();
					mSpinner.setAdapter((SpinnerAdapter) mAdapter);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mSpinner = (Spinner) findViewById(R.id.spinner1);
		mSpinner.setOnItemSelectedListener(this);

		mButtonScan = (Button) findViewById(R.id.buttonScan);
		mButtonScan.setOnClickListener(this);

		for (int id : sKeyMap.keySet()) {
			Button button = (Button) findViewById(id);
			if (button != null) {
				button.setOnClickListener(this);
				mButtonMap.put(button, sKeyMap.get(id));
			}
		}

		Intent service = new Intent(this, DiscoveryService.class);
		bindService(service, mConnection, BIND_AUTO_CREATE);
	}

	@Override
	protected void onDestroy() {
		unbindService(mConnection);
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter(DiscoveryService.ACTION_SCAN_RESULT_CHANGED);
		registerReceiver(mReceiver, filter );
		super.onResume();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onClick(View v) {
		int id = v.getId();
		if (id == R.id.buttonScan) {
			try {
				if (mDiscoveryService != null) {
					mDiscoveryService.startDiscovery(0);
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		} else {
			Integer keycode = sKeyMap.get(id);
			if (keycode != null) {
				sendKeyEvent(keycode);
			}
		}
	}

	private int writeValue8(byte value, byte[] buff, int offset) {
		buff[offset] = value;
		return offset + 1;
	}

	private int writeValue16(short value, byte[] buff, int offset) {
		offset = writeValue8((byte) (value & 0xFF), buff, offset);
		return writeValue8((byte) ((value >> 8) & 0xFF), buff, offset);
	}

	private int writeValue32(int value, byte[] buff, int offset) {
		offset = writeValue16((short) (value & 0xFFFF), buff, offset);
		return writeValue16((short) ((value >> 16) & 0xFFFF), buff, offset);
	}

	private int writeInputEvent(int type, int code, int value, byte[] buff, int offset) {
		offset = writeValue16((short) type, buff, offset);
		offset = writeValue16((short) code, buff, offset);
		return writeValue32(value, buff, offset);
	}

	private int writeSyncEvent(byte[] buff, int offset) {
		return writeInputEvent(EVENT_TYPE_SYNC, 0, 0, buff, offset);
	}

	private int writeKeyEvent(int code, int value, byte[] buff, int offset) {
		offset = writeInputEvent(EVENT_TYPE_KEY, code, value, buff, offset);
		return writeSyncEvent(buff, offset);
	}

	private int writeKeyEvent(int code, byte[] buff, int offset) {
		offset = writeKeyEvent(code, 1, buff, offset);
		return writeKeyEvent(code, 0, buff, offset);
	}

	private boolean sendData(byte[] data) {
		if (mOutputStream == null) {
			return false;
		}

		try {
			mOutputStream.write(data);
			mOutputStream.flush();
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

	private boolean sendKeyEvent(int code) {
		byte[] data = new byte[32];
		writeKeyEvent(code, data, 0);
		return sendData(data);
	}

	private boolean sendTcpDdPackage(short type, byte[] body) {
		byte[] data;

		if (body == null) {
			data = new byte[12];
		} else {
			data = new byte[body.length + 12];

			for (int i = body.length - 1; i >= 0; i++) {
				data[i + 12] = body[i];
			}
		}

		int offset = writeValue32(TCP_DD_VERSION, data, 0);
		offset = writeValue16(type, data, offset);
		offset = writeValue16((short) ~type, data, offset);
		offset = writeValue32(0, data, offset);

		return sendData(data);
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		mScanResult = mScanResults.get(mSpinner.getSelectedItemPosition());
		if (mScanResult == null) {
			return;
		}

		Log.d(TAG, "onItemSelected: mScanResult = " + mScanResult);

		Thread thread = new Thread() {

			@Override
			public void run() {
				if (mOutputStream != null) {
					try {
						mOutputStream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				if (mSocket != null) {
					try {
						mSocket.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				try {
					mSocket = new Socket(mScanResult.getAddress(), mScanResult.getPort());
					mSocket.setTcpNoDelay(true);
					mOutputStream = mSocket.getOutputStream();
					sendTcpDdPackage(TCP_DD_REQ_KEYPAD, null);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		};

		thread.start();
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
	}
}
