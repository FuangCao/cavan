package com.cavan.remotecontrol;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.SuppressLint;
import android.util.Log;

@SuppressLint("DefaultLocale")
public class DiscoveryThread extends Thread {

	private static final String TAG = "Cavan";

	private int mPort;
	InetAddress mAddress;
	private MulticastSocket mSocket;
	private DatagramPacket mCommand;
	private List<ScanResult> mResults = new ArrayList<DiscoveryThread.ScanResult>();
	private Pattern mPattern = Pattern.compile("TCP_DD:\\s*port\\s*=\\s*([0-9]+),\\s*hostname\\s*=\\s*(.*)");

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

	public void startDiscoveryAsync() {
		Thread thread = new Thread() {

			@Override
			public void run() {
				startDiscovery();
			}
		};

		thread.start();
	}

	private boolean startDiscovery() {
		if (mSocket == null || mCommand == null) {
			return false;
		}

		Log.e(TAG, "startDiscovery");

		mResults.clear();

		try {
			mSocket.send(mCommand);
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

	private void onDiscovery(InetAddress address, String text) {
		Matcher matcher = mPattern.matcher(text);
		if (matcher.find()) {
			String port = matcher.group(1);
			String hostname = matcher.group(2);

			ScanResult result = new ScanResult(Integer.valueOf(port), hostname, address);
			Log.e(TAG, "result = " + result);

			mResults.add(result);
			onDiscovery();
		}
	}

	protected void onDiscovery() {
		Log.e(TAG, "onDiscovery", new Throwable());
	}

	public List<ScanResult> getScanResults() {
		return mResults;
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
					onDiscovery(pack.getAddress(), text);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	class ScanResult {
		private int mPort;
		private String mHostname;
		private InetAddress mAddress;

		public ScanResult(int port, String hostname, InetAddress address) {
			super();
			mPort = port;
			mHostname = hostname;
			mAddress = address;
		}

		public int getPort() {
			return mPort;
		}

		public void setPort(int port) {
			mPort = port;
		}

		public String getHostname() {
			return mHostname;
		}

		public void setHostname(String hostname) {
			mHostname = hostname;
		}

		public InetAddress getAddress() {
			return mAddress;
		}

		public void setAddress(InetAddress address) {
			mAddress = address;
		}

		@Override
		public String toString() {
			return String.format("%s:%d - %s", mAddress.getHostAddress(), mPort, mHostname);
		}

		public String toShortString() {
			return String.format("%s - %s", mAddress.getHostAddress(), mHostname);
		}
	}
}
