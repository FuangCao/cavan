package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class TcpConnector extends NetworkConnector {

	private int mPort;
	private String mHostname;

	protected Socket mSocket;
	protected InputStream mInStream;
	protected OutputStream mOutStream;

	public TcpConnector(String hostname, int port) {
		mHostname = hostname;
		mPort = port;
	}

	protected boolean onDataReceived(byte[] bytes, int length) {
		CavanJava.dLog("onDataReceived: length = " + length);
		return true;
	}

	@Override
	protected boolean doConnect() {
		mSocket = new Socket();

		try {
			mSocket.connect(new InetSocketAddress(mHostname, mPort), 6000);
			mOutStream = mSocket.getOutputStream();
			mInStream = mSocket.getInputStream();
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		doDisconnect();

		return false;
	}

	@Override
	protected void doDisconnect() {
		if (mInStream != null) {
			try {
				mInStream.close();
				mInStream = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (mOutStream != null) {
			try {
				mOutStream.close();
				mOutStream = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (mSocket != null) {
			try {
				mSocket.close();
				mSocket = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	protected void onDaemonRun() {
		byte[] bytes = new byte[2048];

		while (mInStream != null) {
			try {
				int length = mInStream.read(bytes);

				if (!onDataReceived(bytes, length)) {
					break;
				}
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}
		}
	}
}
