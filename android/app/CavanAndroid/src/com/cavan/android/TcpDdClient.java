package com.cavan.android;

import java.net.InetAddress;

import android.annotation.SuppressLint;
import android.net.LocalSocketAddress;

@SuppressLint("DefaultLocale") public class TcpDdClient extends CavanNetworkClient {

	public static final int TCP_DD_VERSION = 0x20151223;
	public static final int TCP_DD_HEADER_LENGTH = 12;

	public static final short TCP_DD_RESPONSE = 0;
	public static final short TCP_DD_WRITE = 1;
	public static final short TCP_DD_READ = 2;
	public static final short TCP_DD_EXEC = 3;
	public static final short TCP_ALARM_ADD = 4;
	public static final short TCP_ALARM_REMOVE = 5;
	public static final short TCP_ALARM_LIST = 6;
	public static final short TCP_KEYPAD_EVENT = 7;
	public static final short TCP_DD_MKDIR = 8;
	public static final short TCP_DD_RDDIR = 9;
	public static final short TCP_DD_FILE_STAT = 10;
	public static final short TCP_DD_BREAKPOINT = 11;
	public static final short TCP_DD_DISCOVERY = 12;
	public static final short TCP_REMOTE_CTRL = 13;

	public TcpDdClient(CavanNetworkClientImpl client) {
		super(client);
	}

	public TcpDdClient(InetAddress address, int port) {
		super(address, port);
	}

	public TcpDdClient(LocalSocketAddress address) {
		super(address);
	}

	public TcpDdClient(String pathname) {
		super(pathname);
	}

	public boolean sendPackage(TcpDdPackage pkg) {
		byte[] bytes = pkg.encode();
		if (bytes == null) {
			dLog("sendPackage: bytes is null");
			return false;
		}

		return sendData(bytes);
	}

	public boolean recvPackage(TcpDdPackage pkg) {
		byte[] bytes = new byte[2048];
		int length = recvData(bytes);
		if (length < 0) {
			dLog("recvData length = " + length);
			return false;
		}

		return pkg.decode(bytes, length);
	}
}
