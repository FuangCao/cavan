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

		CavanJava.printD("ip = %d.%d.%d.%d", addrBytes[0], addrBytes[1], addrBytes[2], addrBytes[3]);

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
			}
		}

		@Override
		public void run() {
			InputStream stream = null;

			try {
				mSocket = new Socket(mAddress, mPort);
				stream = mSocket.getInputStream();

				byte[] bytes = new byte[1024];
				int length = stream.read(bytes);
				if (length > 0) {
					String message = new String(bytes, 0, length);
					if (message.equals(mMessage)) {
						mSockets.add(mSocket);
						mSocket = null;
					}
				}
			} catch (Exception e) {
				// e.printStackTrace();
			}

			if (mSocket != null) {
				if (stream != null) {
					try {
						stream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

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
