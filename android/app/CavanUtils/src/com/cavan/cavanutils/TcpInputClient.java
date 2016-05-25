package com.cavan.cavanutils;

import java.net.InetAddress;

import android.net.LocalSocketAddress;

public abstract class TcpInputClient extends TcpDdClient {

	private short mType;

	abstract public boolean sendKeyEvent(int code, int value);

	public TcpInputClient(InetAddress address, int port, short type) {
		super(address, port);
		mType = type;
	}

	public TcpInputClient(LocalSocketAddress address, short type) {
		super(address);
		mType = type;
	}

	@Override
	public boolean sendRequest() {
		TcpDdPackage req = new TcpDdPackage(mType);
		if (!sendPackage(req)) {
			logE("Failed to sendPackage " + mType);
			return false;
		}

		TcpDdResponse response = new TcpDdResponse();
		if (!recvPackage(response)) {
			logE("Failed to recvPackage TcpDdResponse");
			return false;
		}

		if (response.getCode() < 0) {
			logE("Invalid code = " + response.getCode());
			return false;
		}

		return true;
	}

	public boolean sendKeyEvent(int code) {
		return sendKeyEvent(code, 1) && sendKeyEvent(code, 0);
	}
}
