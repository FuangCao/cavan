package com.cavan.cavanutils;

import java.net.Socket;

public class SuClient {
	private Socket mSocket;

	public SuClient() {
		mSocket = new Socket();
	}

	public int doCommand(String... args) {
		return 0;
	}
}
