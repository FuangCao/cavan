package com.cavan.remotecontrol;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.List;
import android.annotation.SuppressLint;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
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
import com.cavan.remotecontrol.DiscoveryThread.ScanResult;

@SuppressLint("HandlerLeak")
public class MainActivity extends ActionBarActivity implements OnClickListener, OnItemSelectedListener {

	private static final String TAG = "Cavan";
	private static final int TCP_DD_VERSION = 0x20151223;
	private static final short TCP_DD_REQ_KEYPAD = 7;
	private static final short EVENT_TYPE_SYNC = 0;
	private static final short EVENT_TYPE_KEY = 1;

	private static final int[] BUTTON_IDS = {
		R.id.buttonScan,
		R.id.buttonBack,
		R.id.buttonDown,
		R.id.buttonEnter,
		R.id.buttonHome,
		R.id.buttonLeft,
		R.id.buttonMenu,
		R.id.buttonRight,
		R.id.buttonUp,
		R.id.buttonVolumeUp,
		R.id.buttonVolumeDown,
		R.id.buttonBrightUp,
		R.id.buttonBrightDown,
	};

	private Socket mSocket;
	private OutputStream mOutputStream;

	private Spinner mSpinner;
	private List<ScanResult> mScanResults;
	private ScanResult mScanResult;
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

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			mSpinner.setAdapter((SpinnerAdapter) mAdapter);
		}
	};

	private DiscoveryThread mThread = new DiscoveryThread() {

		@Override
		protected void onDiscovery() {
			mScanResults = getScanResults();
			mHandler.sendEmptyMessageDelayed(0, 500);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mSpinner = (Spinner) findViewById(R.id.spinner1);
		mSpinner.setOnItemSelectedListener(this);

		for (int id : BUTTON_IDS) {
			Button button = (Button) findViewById(id);
			if (button != null) {
				button.setOnClickListener(this);
			}
		}

		mThread.start();
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
		switch (v.getId()) {
		case R.id.buttonScan:
			mThread.startDiscoveryAsync();
			break;

		case R.id.buttonBack:
			sendKeyEvent(158);
			break;

		case R.id.buttonDown:
			sendKeyEvent(108);
			break;

		case R.id.buttonEnter:
			sendKeyEvent(28);
			break;

		case R.id.buttonHome:
			sendKeyEvent(172);
			break;

		case R.id.buttonLeft:
			sendKeyEvent(105);
			break;

		case R.id.buttonMenu:
			sendKeyEvent(139);
			break;

		case R.id.buttonRight:
			sendKeyEvent(106);
			break;

		case R.id.buttonUp:
			sendKeyEvent(103);
			break;

		case R.id.buttonVolumeUp:
			sendKeyEvent(115);
			break;

		case R.id.buttonVolumeDown:
			sendKeyEvent(114);
			break;

		case R.id.buttonBrightUp:
			sendKeyEvent(225);
			break;

		case R.id.buttonBrightDown:
			sendKeyEvent(224);
			break;
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
