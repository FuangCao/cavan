package com.cavan.cavanutils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketAddress;

public class CavanTcpClient extends CavanNetworkClient {

	private Socket mSocket;
	private SocketAddress mSocketAddress;

	public CavanTcpClient(SocketAddress address) {
		mSocketAddress = address;
	}

	@Override
	protected boolean openSocket() {
		mSocket = new Socket();

		try {
			mSocket.connect(mSocketAddress);
			mSocket.setTcpNoDelay(true);
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
