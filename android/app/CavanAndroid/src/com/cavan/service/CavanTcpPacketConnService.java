package com.cavan.service;

import com.cavan.java.CavanTcpClient;
import com.cavan.java.CavanTcpPacketClient;
import com.cavan.java.CavanTcpPacketClient.CavanTcpPacketClientListener;

public class CavanTcpPacketConnService extends CavanTcpConnService implements CavanTcpPacketClientListener {

	@Override
	protected CavanTcpClient doCreateTcpClient() {
		CavanTcpPacketClient client = new CavanTcpPacketClient();
		client.setTcpPacketClientListener(this);
		return client;
	}

	@Override
	public boolean onPacketReceived(byte[] bytes, int length) {
		return false;
	}
}
