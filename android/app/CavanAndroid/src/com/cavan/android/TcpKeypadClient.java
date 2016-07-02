package com.cavan.android;

import java.net.InetAddress;

import android.net.LocalSocketAddress;

import com.cavan.java.CavanByteCache;

public class TcpKeypadClient extends TcpInputClient {

	private static final short EVENT_TYPE_SYNC = 0;
	private static final short EVENT_TYPE_KEY = 1;

	public TcpKeypadClient(InetAddress address, int port) {
		super(address, port, TCP_KEYPAD_EVENT);
	}

	public TcpKeypadClient(LocalSocketAddress address) {
		super(address, TCP_KEYPAD_EVENT);
	}

	public boolean writeInputEvent(CavanByteCache cache, int type, int code, int value) {
		if (!cache.writeValue16((short) type)) {
			return false;
		}

		if (!cache.writeValue16((short) code)) {
			return false;
		}

		return cache.writeValue32(value);
	}

	public boolean writeSyncEvent(CavanByteCache cache) {
		return writeInputEvent(cache, EVENT_TYPE_SYNC, 0, 0);
	}

	public boolean writeKeyEvent(CavanByteCache cache, int code, int value) {
		if (!writeInputEvent(cache, EVENT_TYPE_KEY, code, value)) {
			return false;
		}

		return writeSyncEvent(cache);
	}

	public boolean writeKeyEvent(CavanByteCache cache, int code) {
		if (!writeKeyEvent(cache, code, 1)) {
			return false;
		}

		return writeKeyEvent(cache, code, 0);
	}

	@Override
	public boolean sendKeyEvent(int code) {
		CavanByteCache cache = new CavanByteCache(32);
		if (!writeKeyEvent(cache, code)) {
			return false;
		}

		return sendData(cache.getBytes());
	}

	@Override
	public boolean sendKeyEvent(int code, int value) {
		CavanByteCache cache = new CavanByteCache(16);
		if (!writeKeyEvent(cache, code, value)) {
			return false;
		}

		return sendData(cache.getBytes());
	}
}
