package com.cavan.cavanutils;

import java.net.InetAddress;

import android.net.LocalSocketAddress;

public class TcpKeypadClient extends TcpDdClient {

	private static final short EVENT_TYPE_SYNC = 0;
	private static final short EVENT_TYPE_KEY = 1;

	public TcpKeypadClient(InetAddress address, int port) {
		super(address, port);
	}

	public TcpKeypadClient(LocalSocketAddress address) {
		super(address);
	}

	@Override
	public boolean sendRequest() {
		TcpDdPackage req = new TcpDdPackage(TCP_KEYPAD_EVENT);
		if (!sendPackage(req)) {
			logE("Failed to sendPackage TCP_KEYPAD_EVENT");
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

	public boolean writeInputEvent(ByteCache cache, int type, int code, int value) {
		if (!cache.writeValue16((short) type)) {
			return false;
		}

		if (!cache.writeValue16((short) code)) {
			return false;
		}

		return cache.writeValue32(value);
	}

	public boolean writeSyncEvent(ByteCache cache) {
		return writeInputEvent(cache, EVENT_TYPE_SYNC, 0, 0);
	}

	public boolean writeKeyEvent(ByteCache cache, int code, int value) {
		if (!writeInputEvent(cache, EVENT_TYPE_KEY, code, value)) {
			return false;
		}

		return writeSyncEvent(cache);
	}

	public boolean writeKeyEvent(ByteCache cache, int code) {
		if (!writeKeyEvent(cache, code, 1)) {
			return false;
		}

		return writeKeyEvent(cache, code, 0);
	}

	public boolean sendKeyEvent(int code) {
		ByteCache cache = new ByteCache(32);
		if (!writeKeyEvent(cache, code)) {
			return false;
		}

		return sendData(cache.getBytes());
	}

	public boolean sendKeyEvent(int code, int value) {
		ByteCache cache = new ByteCache(16);
		if (!writeKeyEvent(cache, code, value)) {
			return false;
		}

		return sendData(cache.getBytes());
	}
}
