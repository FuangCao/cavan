package com.cavan.cavanutils;

import java.net.InetAddress;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class TcpDdDiscoveryService extends TcpDiscoveryService {

	private Pattern mPattern = Pattern.compile("TCP_DD:\\s*hostname\\s*=\\s*(.*)");

	@Override
	protected void addScanResult(CavanNetworkClient client, InetAddress address, int port) {
		TcpDdClient dd = (TcpDdClient) client;
		TcpDdPackage pkg = new TcpDdPackage(TcpDdClient.TCP_DD_DISCOVERY);
		if (!dd.sendPackage(pkg)) {
			return;
		}

		byte[] bytes = new byte[1024];
		int length = dd.recvData(bytes);
		if (length <= 0) {
			return;
		}

		Matcher matcher = mPattern.matcher(new String(bytes, 0, length));
		if (matcher.find()) {
			String hostname = matcher.group(1);
			addScanResult(port, hostname, address);
		}
	}

	@Override
	protected CavanNetworkClient openClient(InetAddress address, int port) {
		return new TcpDdClient(address, port);
	}
}
