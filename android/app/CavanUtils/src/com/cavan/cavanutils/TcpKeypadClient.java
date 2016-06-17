package com.cavan.cavanutils;

import java.net.InetAddress;

import com.cavan.ByteCache;

import android.net.LocalSocketAddress;

public class TcpKeypadClient extends TcpInputClient {

	private static final short EVENT_TYPE_SYNC = 0;
	private static final short EVENT_TYPE_KEY = 1;

	public TcpKeypadClient(InetAddress address, int port) {
		super(address, port, TCP_KEYPAD_EVENT);
	}

	public TcpKeypadClient(LocalSocketAddress address) {
		super(address, TCP_KEYPAD_EVENT);
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

	@Override
	public boolean sendKeyEvent(int code) {
		ByteCache cache = new ByteCache(32);
		if (!writeKeyEvent(cache, code)) {
			return false;
		}

		return sendData(cache.getBytes());
	}

	@Override
	public boolean sendKeyEvent(int code, int value) {
		ByteCache cache = new ByteCache(16);
		if (!writeKeyEvent(cache, code, value)) {
			return false;
		}

		return sendData(cache.getBytes());
	}
}
