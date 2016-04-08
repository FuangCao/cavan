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
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

@SuppressLint({ "HandlerLeak", "UseSparseArrays", "NewApi" })
public class MainActivity extends ActionBarActivity implements OnClickListener {

	public static final String TAG = "Cavan";

	private static final int EVENT_LINK_CHANGED = 0;
	private static final int EVENT_AUTO_CONNECT = 1;

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
		sKeyMap.put(R.id.buttonPower, 116);
		sKeyMap.put(R.id.buttonMute, 113);
		sKeyMap.put(R.id.buttonDel, 14);
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

	private Button mButtonScan;
	private ScanResult mScanResult;
	private List<ScanResult> mScanResults;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(DiscoveryService.ACTION_SCAN_RESULT_CHANGED)) {
				if (mDiscoveryService == null) {
					return;
				}

				try {
					mScanResults = mDiscoveryService.getScanResult();
					invalidateOptionsMenu();
					mHandler.removeMessages(EVENT_AUTO_CONNECT);
					mHandler.sendEmptyMessageDelayed(EVENT_AUTO_CONNECT, 2000);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_LINK_CHANGED:
				ScanResult result = (ScanResult) msg.obj;

				if (result != null) {
					setTitle(result.getShortString());
					Toast.makeText(getApplicationContext(), R.string.text_connected, Toast.LENGTH_SHORT).show();
				} else {
					try {
						mDiscoveryService.startDiscovery(0);
					} catch (RemoteException e) {
						e.printStackTrace();
					}

					setTitle(R.string.text_disconnected);
					Toast.makeText(getApplicationContext(), R.string.text_disconnected, Toast.LENGTH_SHORT).show();
				}
				break;

			case EVENT_AUTO_CONNECT:
				if (mOutputStream == null) {
					if (mScanResults.size() == 1) {
						connect(mScanResults.get(0));
					} else {
						setTitle(R.string.text_manul_connect);
					}
				}
				break;
			}

			super.handleMessage(msg);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		setTitle(R.string.text_not_connect);

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
	public boolean onPrepareOptionsMenu(Menu menu) {
		menu.clear();

		if (mScanResults != null) {
			for (int i = 0; i < mScanResults.size(); i++) {
				ScanResult result = mScanResults.get(i);
				menu.add(Menu.NONE, i, 0, result.getShortString());
			}
		}

		return super.onPrepareOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		ScanResult result = mScanResults.get(item.getItemId());
		connect(result);

		return true;
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

			try {
				mOutputStream.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}

			mScanResult = null;
			mOutputStream = null;
			mHandler.sendEmptyMessage(EVENT_LINK_CHANGED);
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

	private void connect(ScanResult result) {
		if (mScanResult != null && mScanResult.equals(result)) {
			setTitle(result.getShortString());
		} else {
			ConnectThread thread = new ConnectThread();
			thread.start();
		}

		mScanResult = result;
		Log.e(TAG, "mScanResult = " + mScanResult);
	}

	class ConnectThread extends Thread {

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
				if (sendTcpDdPackage(TCP_DD_REQ_KEYPAD, null)) {
					Message message = mHandler.obtainMessage(EVENT_LINK_CHANGED, mScanResult);
					message.sendToTarget();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
