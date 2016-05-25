package com.cavan.cavanutils;

import java.net.InetAddress;

import android.net.LocalSocketAddress;

public class RemoteCtrlClient extends TcpInputClient {

	public RemoteCtrlClient(InetAddress address, int port) {
		super(address, port, TCP_REMOTE_CTRL);
	}

	public RemoteCtrlClient(LocalSocketAddress address) {
		super(address, TCP_REMOTE_CTRL);
	}

	@Override
	public boolean sendKeyEvent(int code, int value) {
		return sendValue16((short) (code | (value << 15)));
	}
}