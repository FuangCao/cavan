package com.cavan.cavanutils;

import java.net.InetAddress;

import android.annotation.SuppressLint;
import android.net.LocalSocketAddress;

@SuppressLint("DefaultLocale") public class TcpExecClient extends TcpDdClient {

	public TcpExecClient(InetAddress address, int port) {
		super(address, port);
	}

	public TcpExecClient(LocalSocketAddress address) {
		super(address);
	}

	public TcpExecClient(CavanNetworkClientImpl client) {
		super(client);
	}

	public TcpExecClient(String pathname) {
		super(pathname);
	}

	public boolean runCommand(String command) {
		if (!connect()) {
			return false;
		}

		TcpDdExecReq req = new TcpDdExecReq(command);
		if (!sendPackage(req)) {
			return false;
		}

		TcpDdResponse response = new TcpDdResponse();
		if (!recvPackage(response)) {
			return false;
		}

		if (response.getCode() < 0) {
			return false;
		}

		while (true) {
			byte[] data = new byte[1024];
			int length = recvData(data);
			if (length <= 0) {
				break;
			}

			logD(new String(data, 0, length));
		}

		disconnect();

		return true;
	}
}