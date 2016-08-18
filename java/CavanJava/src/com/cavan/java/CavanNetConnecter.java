package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;


public class CavanNetConnecter {

	private int mPort;
	private String mMessage;
	private ArrayList<Socket> mSockets = new ArrayList<Socket>();

	public CavanNetConnecter(int port, String message) {
		mPort = port;
		mMessage = message;
	}

	public int getPort() {
		return mPort;
	}

	public void setPort(int port) {
		mPort = port;
	}

	synchronized public ArrayList<Socket> doConnect(long timeout) {
		mSockets.clear();

		InetAddress localAddr = CavanJava.getIpAddress();
		if (localAddr == null) {
			return null;
		}

		ConnectThread[] threads = new ConnectThread[255];

		byte[] addrBytes = localAddr.getAddress();

		CavanJava.dfLog("ip = %d.%d.%d.%d", addrBytes[0] & 0xFF, addrBytes[1] & 0xFF, addrBytes[2], addrBytes[3]);

		for (int i = 2; i < 255; i++) {
			addrBytes[3] = (byte) i;

			try {
				threads[i] = new ConnectThread(InetAddress.getByAddress(addrBytes));
				threads[i].start();
			} catch (UnknownHostException e) {
				e.printStackTrace();
				break;
			}
		}

		try {
			wait(timeout);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		for (int i = 2; i < 255; i++) {
			threads[i].close();
		}

		return mSockets;
	}

	protected boolean doIdentify(Socket socket) {
		if (mMessage == null) {
			return true;
		}

		InputStream stream = null;

		try {
			stream = socket.getInputStream();
		} catch (IOException e) {
			return false;
		}

		if (stream == null) {
			return false;
		}

		byte[] bytes = new byte[mMessage.length()];
		int length;

		try {
			length = stream.read(bytes);
		} catch (IOException e) {
			return false;
		}

		if (length < bytes.length) {
			return false;
		}

		String message = new String(bytes, 0, length);
		if (message.equals(mMessage)) {
			return true;
		}

		return false;
	}

	public class ConnectThread extends Thread {

		private Socket mSocket;
		private InetAddress mAddress;

		public ConnectThread(InetAddress address) {
			super();
			mAddress = address;
		}

		public void close() {
			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

				mSocket = null;
			}
		}

		@Override
		public void run() {
			try {
				mSocket = new Socket(mAddress, mPort);
				if (doIdentify(mSocket)) {
					synchronized (mSockets) {
						mSockets.add(mSocket);
					}

					mSocket = null;
				}
			} catch (Exception e) {
				// e.printStackTrace();
			}

			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

				mSocket = null;
			}
		}
	}
}
