package com.cavan.remotecontrol;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import android.util.Log;

public class DiscoveryThread extends Thread {

	private static final String TAG = "Cavan";

	private int mPort;
	InetAddress mAddress;
	private MulticastSocket mSocket;
	private DatagramPacket mCommand;

	public DiscoveryThread(int port) {
		mPort = port;

		try {
			mSocket = new MulticastSocket();

			byte[] bytes = "cavan-discovery".getBytes();
			mAddress = InetAddress.getByName("224.0.0.1");
			mCommand = new DatagramPacket(bytes, bytes.length, mAddress, mPort);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public DiscoveryThread() {
		this(8888);
	}

	public boolean startDiscovery() {
		if (mSocket == null || mCommand == null) {
			return false;
		}

		Log.e(TAG, "startDiscovery");

		try {
			mSocket.send(mCommand);
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

	protected void onDiscovery(String text) {
		Log.e(TAG, "text = " + text, new Throwable());
	}

	@Override
	public void run() {
		if (startDiscovery()) {
			byte[] bytes = new byte[1024];
			DatagramPacket pack = new DatagramPacket(bytes, bytes.length, mAddress, mPort);
			while (true) {
				try {
					mSocket.receive(pack);
					String text = new String(pack.getData(), 0, pack.getLength());
					// Log.e(TAG, "text = " + text);
					onDiscovery(text);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
