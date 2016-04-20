package com.cavan.cavanutils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;

public class CavanUnixClient extends CavanNetworkClient {

	private LocalSocket mSocket;
	private LocalSocketAddress mSocketAddress;

	public CavanUnixClient(LocalSocketAddress address) {
		super();
		mSocketAddress = address;
	}

	@Override
	protected boolean openSocket() {
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
	protected void closeSocket() {
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
	protected InputStream getInputStream() {
		try {
			return mSocket.getInputStream();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	@Override
	protected OutputStream getOutputStream() {
		try {
			return mSocket.getOutputStream();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}
}
