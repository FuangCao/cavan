package com.cavan.cavanutils;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;

public abstract class CavanNetworkClientImpl extends CavanUtils {
	public abstract boolean openSocket();
	public abstract void closeSocket();
	public abstract InputStream getInputStream();
	public abstract OutputStream getOutputStream();

	public DatagramPacket getPacket() {
		return null;
	}
}