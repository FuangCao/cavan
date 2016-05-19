package com.cavan.cavanutils;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class UdpDiscoveryService extends DiscoveryService {

	private CavanNetworkClient mClient;
	private boolean mThreadStopped = true;
	private Pattern mPattern = Pattern.compile("TCP_DD:\\s*port\\s*=\\s*([0-9]+),\\s*hostname\\s*=\\s*(.*)");

	@Override
	public void onCreate() {

		try {
			mClient = new CavanNetworkClient(InetAddress.getByName("224.0.0.1"), 8888, true);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mClient != null) {
			mClient.disconnect();
		}

		super.onDestroy();
	}

	class DiscoveryThread extends Thread {

		public DiscoveryThread() {
			super();
		}

		@Override
		public void run() {
			if (!mClient.sendData("cavan-discovery".getBytes())) {
				return;
			}

			if (mThreadStopped) {
				mThreadStopped = false;

				byte[] bytes = new byte[1024];

				while (mClient != null) {
					int length = mClient.recvData(bytes);
					if (length <= 0) {
						break;
					}

					InetAddress address = (InetAddress) mClient.getRemoteAddress();
					if (address == null) {
						break;
					}

					Matcher matcher = mPattern.matcher(new String(bytes, 0, length));
					if (matcher.find()) {
						String port = matcher.group(1);
						String hostname = matcher.group(2);
						addScanResult(Integer.valueOf(port), hostname, address);
					}
				}

				mThreadStopped = true;
			}
		}
	}

	@Override
	protected boolean startScan(int port) {
		if (mClient == null) {
			return false;
		}

		if (port > 0) {
			mClient.setPort(port);
		}

		DiscoveryThread thread = new DiscoveryThread();
		thread.start();

		return true;
	}
}
