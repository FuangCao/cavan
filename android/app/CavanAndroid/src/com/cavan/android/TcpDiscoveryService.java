package com.cavan.android;

import java.net.InetAddress;
import java.net.UnknownHostException;

import com.cavan.java.CavanJava;

public class TcpDiscoveryService extends DiscoveryService {

	private DiscoveryThread[] mThreads = new DiscoveryThread[255];

	@Override
	protected boolean startScan(int port) {
		InetAddress address = CavanJava.getIpAddress();
		if (address == null) {
			return false;
		}

		byte[] bytes = address.getAddress();

		for (int i = 1; i < mThreads.length; i++) {
			if (mThreads[i] != null) {
				continue;
			}

			bytes[3] = (byte) i;

			try {
				mThreads[i] = new DiscoveryThread(i, InetAddress.getByAddress(bytes), port);
				mThreads[i].start();
			} catch (UnknownHostException e) {
				e.printStackTrace();
				return false;
			}
		}

		return true;
	}

	protected void addScanResult(CavanNetworkClient client, InetAddress address, int port) {
		addScanResult(port, "unknown", address);
	}

	protected CavanNetworkClient openClient(InetAddress address, int port) {
		return new CavanNetworkClient(address, port);
	}

	class DiscoveryThread extends Thread {

		private int mIndex;
		private int mPort;
		private InetAddress mAddress;
		private CavanNetworkClient mClient;

		public DiscoveryThread(int index, InetAddress address, int port) {
			super();

			mIndex = index;
			mPort = port;
			mAddress = address;
		}

		public void disconnect() {
			if (mClient != null) {
				mClient.disconnect();
			}
		}

		@Override
		public void run() {
			mClient = openClient(mAddress, mPort);
			if (mClient.connectSync()) {
				addScanResult(mClient, mAddress, mPort);
				mClient.disconnect();
			}
			mThreads[mIndex] = null;
		}
	}
}
