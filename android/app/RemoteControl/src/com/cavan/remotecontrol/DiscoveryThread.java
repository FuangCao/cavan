package com.cavan.remotecontrol;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

import android.os.AsyncTask;
import android.util.Log;

public class DiscoveryThread extends AsyncTask {

	private static final String TAG = "Cavan";

	private MulticastSocket mSocket;

	public DiscoveryThread() {
		// TODO Auto-generated constructor stub
	}

	@Override
	protected Object doInBackground(Object... params) {
		try {
			mSocket = new MulticastSocket();
			byte[] bytes = ((String) params[0]).getBytes();
			InetAddress address = InetAddress.getByName("192.168.1.255");
			Log.d(TAG , "isMulticastAddress = " + address.isMulticastAddress());
			DatagramPacket pack = new DatagramPacket(bytes, bytes.length, address, 1122);
			mSocket.send(pack);
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}
}
