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
import com.cavan.java.CavanString;
import com.cavan.resource.JwaooToyActivity;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	private static final int MSG_UPDATE_BD_ADDR = 10;

	private byte[] mBdAddr;

	private Button mButtonExit;
	private Button mButtonBurn;

	private EditText mEditTextIp;
	private EditText mEditTextPort;
	private EditText mEditTextBdAddr;

	private byte[] getBdAddrFromServer() {
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
			CavanAndroid.eLog("text = " + text);

			if (!text.equals("JwaooBdAddrServer")) {
				return null;
			}

			outStream = socket.getOutputStream();
			if (outStream == null) {
				return null;
			}

			outStream.write("AllocBdAddr: 1".getBytes());

			byte[] addr = new byte[6];

			length = inStream.read(addr);
			if (length != 6) {
				return null;
			}

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

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mEditTextIp = (EditText) findViewById(R.id.editTextServerIp);
		mEditTextIp.setText("192.168.1.59");

		mEditTextPort = (EditText) findViewById(R.id.editTextServerPort);
		mEditTextPort.setText("12345");

		mEditTextBdAddr = (EditText) findViewById(R.id.editTextBdAddr);

		mButtonExit = (Button) findViewById(R.id.buttonExit);
		mButtonExit.setOnClickListener(this);

		mButtonBurn = (Button) findViewById(R.id.buttonBurn);
		mButtonBurn.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	protected void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_UPDATE_BD_ADDR:
			mEditTextBdAddr.setText(CavanString.fromBdAddr(mBdAddr));
			break;
		}
	}

	@Override
	protected boolean onInitialize() {
		mBdAddr = mBleToy.readBdAddress();
		if (mBdAddr == null) {
			return false;
		}

		mHandler.sendEmptyMessage(MSG_UPDATE_BD_ADDR);

		return super.onInitialize();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonExit:
			finish();
			break;

		case R.id.buttonBurn:
			mBdAddr = getBdAddrFromServer();
			break;
		}
	}
}
