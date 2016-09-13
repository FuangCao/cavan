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

	private static final int ALLOC_COUNT = 100;
	private static final String ALLOC_REQ = "AllocBdAddr: " + ALLOC_COUNT;

	private static final int MSG_UPDATE_BD_ADDR = 10;
	private static final int MSG_REQUEST_COMPLETE = 11;

	private byte[] mBdAddrBytes;

	private Button mButtonExit;
	private Button mButtonBurn;
	private Button mButtonRequest;

	private EditText mEditTextIp;
	private EditText mEditTextPort;
	private EditText mEditTextBdAddr;

	private JwaooBdAddr getBdAddrFromServer() {
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

			outStream.write(ALLOC_REQ.getBytes());

			bytes = new byte[4];

			length = inStream.read(bytes);
			if (length != 4) {
				return null;
			}

			return new JwaooBdAddr(CavanJava.buildValue32(bytes, 0), ALLOC_COUNT);
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
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mEditTextIp = (EditText) findViewById(R.id.editTextServerIp);
		mEditTextIp.setText("192.168.1.42");

		mEditTextPort = (EditText) findViewById(R.id.editTextServerPort);
		mEditTextPort.setText("12345");

		mEditTextBdAddr = (EditText) findViewById(R.id.editTextBdAddr);

		mButtonExit = (Button) findViewById(R.id.buttonExit);
		mButtonExit.setOnClickListener(this);

		mButtonBurn = (Button) findViewById(R.id.buttonBurn);
		mButtonBurn.setOnClickListener(this);

		mButtonRequest = (Button) findViewById(R.id.buttonRequest);
		mButtonRequest.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	protected void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_UPDATE_BD_ADDR:
			mEditTextBdAddr.setText(CavanString.fromBdAddr(mBdAddrBytes));
			break;

		case MSG_REQUEST_COMPLETE:
			mButtonRequest.setEnabled(true);
			break;
		}
	}

	@Override
	protected boolean onInitialize() {
		mBdAddrBytes = mBleToy.readBdAddress();
		if (mBdAddrBytes == null) {
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
			CavanAndroid.pLog("count = " + JwaooBdAddr.queryAddrCount(getContentResolver()));
			JwaooBdAddr addr = allocBdAddr();
			CavanAndroid.pLog("count = " + JwaooBdAddr.queryAddrCount(getContentResolver()));
			CavanAndroid.eLog("addr = " + addr);
			if (addr != null) {
				addr.recycle(getContentResolver());
			}
			CavanAndroid.pLog("count = " + JwaooBdAddr.queryAddrCount(getContentResolver()));
			break;

		case R.id.buttonRequest:
			mButtonRequest.setEnabled(false);
			new Thread() {

				@Override
				public void run() {
					JwaooBdAddr addr = getBdAddrFromServer();
					if (addr != null) {
						addr.fflush(getContentResolver());
					}

					CavanAndroid.eLog("addr = " + addr);
					mHandler.sendEmptyMessage(MSG_REQUEST_COMPLETE);
				}
			}.start();
			break;
		}
	}
}
