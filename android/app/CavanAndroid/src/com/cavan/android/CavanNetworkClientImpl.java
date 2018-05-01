package com.cavan.android;

import java.io.InputStream;
import java.io.OutputStream;

public abstract class CavanNetworkClientImpl extends CavanAndroid {

	public abstract boolean openSocket();
	public abstract void closeSocket();
	public abstract InputStream getInputStream();
	public abstract OutputStream getOutputStream();

	public Object getAddress() {
		return null;
	}

	public int getPort() {
		return 0;
	}

	public void setAddress(Object address) {
		/* empty */
	}

	public void setPort(int port) {
		/* empty */
	}

	public Object getRemoteAddress() {
		return null;
	}

	public int getRemotePort() {
		return 0;
	}

	public Object getSocket() {
		return null;
	}
}