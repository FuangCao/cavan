package com.cavan.android;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;

public class CavanUnixClient extends CavanNetworkClientImpl {

	private LocalSocket mSocket;
	private LocalSocketAddress mSocketAddress;

	public CavanUnixClient(LocalSocketAddress address) {
		super();
		mSocketAddress = address;
	}

	@Override
	public boolean openSocket() {
		mSocket = new LocalSocket();

		try {
			mSocket.connect(mSocketAddress);
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		mSocket = null;

		return false;
	}

	@Override
	public void closeSocket() {
		if (mSocket == null) {
			return;
		}

		try {
			mSocket.shutdownInput();
		} catch (IOException e) {
			// e.printStackTrace();
		}

		try {
			mSocket.shutdownOutput();
		} catch (IOException e) {
			// e.printStackTrace();
		}

		try {
			mSocket.close();
		} catch (IOException e) {
			// e.printStackTrace();
		}

		mSocket = null;
	}

	@Override
	public InputStream getInputStream() {
		try {
			return mSocket.getInputStream();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	@Override
	public OutputStream getOutputStream() {
		try {
			return mSocket.getOutputStream();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	@Override
	public Object getAddress() {
		return mSocketAddress;
	}

	@Override
	public void setAddress(Object address) {
		mSocketAddress = (LocalSocketAddress) address;
	}
}
