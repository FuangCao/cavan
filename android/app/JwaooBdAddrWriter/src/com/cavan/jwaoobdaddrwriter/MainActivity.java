package com.cavan.jwaoobdaddrwriter;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import android.os.Bundle;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;
import com.cavan.resource.JwaooToyActivity;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	public static final String KEY_SERVER_IP = "server_ip";
	public static final String KEY_SERVER_PORT = "server_port";

	private static final int ALLOC_COUNT = 100;
	private static final String ALLOC_REQ = "AllocBdAddr: " + ALLOC_COUNT;

	private static final String DEFAULT_SERVER_IP = "192.168.1.42";
	private static final String DEFAULT_SERVER_PORT = "12345";

	private static final int MSG_UPDATE_VIEW = 10;
	private static final int MSG_ADDR_ALLOC_COMPLETE = 11;
	private static final int MSG_ADDR_REQUEST_COMPLETE = 12;

	private byte[] mBdAddrBytes;

	private Button mButtonComplete;
	private Button mButtonBurn;
	private Button mButtonRequest;

	private EditText mEditTextIp;
	private EditText mEditTextPort;
	private EditText mEditTextBdAddr;
	private EditText mEditTextAddrRemain;

	private JwaooBdAddr allocFromServer() {
		Socket socket = null;
		InputStream inStream = null;
		OutputStream outStream = null;

		try {
			int port = Integer.parseInt(mEditTextPort.getText().toString());

			socket = new Socket(mEditTextIp.getText().toString(), port);

			inStream = socket.getInputStream();
			if (inStream == null) {
				return null;
			}

			byte[] bytes = new byte[64];
			int length = inStream.read(bytes);
			if (length < 0) {
				return null;
			}

			String text = new String(bytes, 0, length);
			CavanAndroid.dLog("text = " + text);

			if (!text.equals("JwaooBdAddrServer")) {
				return null;
			}

			outStream = socket.getOutputStream();
			if (outStream == null) {
				return null;
			}

			outStream.write(ALLOC_REQ.getBytes());

			bytes = new byte[4];

			length = inStream.read(bytes);
			if (length != 4) {
				return null;
			}

			JwaooBdAddr addr = new JwaooBdAddr(CavanJava.buildValue32(bytes, 0), ALLOC_COUNT);
			addr.fflush(getContentResolver());

			return addr;
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (outStream != null) {
				try {
					outStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			if (inStream != null) {
				try {
					inStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			if (socket != null) {
				try {
					socket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return null;
	}

	private JwaooBdAddr allocBdAddr() {
		JwaooBdAddr addr = JwaooBdAddr.query(getContentResolver());
		if (addr == null) {
			return null;
		}

		return addr.alloc(getContentResolver());
	}

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		mEditTextIp = (EditText) findViewById(R.id.editTextServerIp);
		mEditTextIp.setText(CavanAndroid.getPreference(this, KEY_SERVER_IP, DEFAULT_SERVER_IP));

		mEditTextPort = (EditText) findViewById(R.id.editTextServerPort);
		mEditTextPort.setText(CavanAndroid.getPreference(this, KEY_SERVER_PORT, DEFAULT_SERVER_PORT));

		mEditTextBdAddr = (EditText) findViewById(R.id.editTextBdAddr);
		mEditTextBdAddr.setKeyListener(null);

		mEditTextAddrRemain = (EditText) findViewById(R.id.editTextAddrRemain);
		mEditTextAddrRemain.setKeyListener(null);

		mButtonComplete = (Button) findViewById(R.id.buttonComplete);
		mButtonComplete.setOnClickListener(this);

		mButtonBurn = (Button) findViewById(R.id.buttonBurn);
		mButtonBurn.setOnClickListener(this);

		mButtonRequest = (Button) findViewById(R.id.buttonRequest);
		mButtonRequest.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	public boolean handleMessage(Message msg) {
		if (super.handleMessage(msg)) {
			return true;
		}

		switch (msg.what) {
		case MSG_UPDATE_VIEW:
			if (mBdAddrBytes == null) {
				mEditTextBdAddr.setText(null);
			} else {
				mEditTextBdAddr.setText(CavanString.fromBdAddr(mBdAddrBytes));
			}

			long count = JwaooBdAddr.queryAddrCount(getContentResolver());
			mEditTextAddrRemain.setText(Long.toString(count));
			break;

		case MSG_ADDR_ALLOC_COMPLETE:
			JwaooBdAddr addr = (JwaooBdAddr) msg.obj;
			if (addr == null) {
				CavanAndroid.showToast(this, R.string.text_no_addr);
			} else {
				byte[] bytes = addr.getBytes();
				if (mBleToy.writeBdAddress(bytes)) {
					mBdAddrBytes = bytes;
					mHandler.sendEmptyMessage(MSG_UPDATE_VIEW);

					CavanAndroid.showToast(this, R.string.text_burn_success);
				} else {
					addr.recycle(getContentResolver());
					CavanAndroid.showToast(this, R.string.text_burn_failed);
				}
			}

			mButtonBurn.setEnabled(true);
			break;

		case MSG_ADDR_REQUEST_COMPLETE:
			addr = (JwaooBdAddr) msg.obj;
			if (addr == null) {
				CavanAndroid.showToast(this, R.string.text_request_addr_failed);
			} else {
				CavanAndroid.putPreference(this, KEY_SERVER_IP, mEditTextIp.getText().toString());
				CavanAndroid.putPreference(this, KEY_SERVER_PORT, mEditTextPort.getText().toString());

				mHandler.sendEmptyMessage(MSG_UPDATE_VIEW);

				String text = getResources().getString(R.string.text_request_addr_success, addr.getCount());
				CavanAndroid.showToast(this, text);
			}

			mButtonRequest.setEnabled(true);
			break;
		}

		return true;
	}

	@Override
	protected boolean onInitialize() {
		mBdAddrBytes = mBleToy.readBdAddress();
		if (mBdAddrBytes == null) {
			return false;
		}

		mHandler.sendEmptyMessage(MSG_UPDATE_VIEW);

		return super.onInitialize();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonComplete:
			if (mBleToy != null) {
				mBleToy.doReboot();
				mBleToy.disconnect();
			}

			showScanActivity();
			break;

		case R.id.buttonBurn:
			if (JwaooBdAddr.isValidBdAddr(mBdAddrBytes)) {
				CavanAndroid.showToast(this, R.string.text_already_burn);
				break;
			}

			mButtonBurn.setEnabled(false);

			new Thread() {

				@Override
				public void run() {
					JwaooBdAddr addr = allocBdAddr();
					mHandler.obtainMessage(MSG_ADDR_ALLOC_COMPLETE, addr).sendToTarget();
				}
			}.start();
			break;

		case R.id.buttonRequest:
			mButtonRequest.setEnabled(false);

			new Thread() {

				@Override
				public void run() {
					JwaooBdAddr addr = allocFromServer();
					mHandler.obtainMessage(MSG_ADDR_REQUEST_COMPLETE, addr).sendToTarget();
				}
			}.start();
			break;
		}
	}
}
