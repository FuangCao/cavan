package com.cavan.java;

import java.io.IOException;
import java.net.Socket;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;

public class CavanSslClient extends CavanTcpClient {

	private SSLContext mSslContext;

	public CavanSslClient(SSLContext context) {
		mSslContext = context;
	}

	public CavanSslClient() {
		mSslContext = null;
	}

	public synchronized SSLContext getSslContext() {
		return mSslContext;
	}

	public synchronized void setSslContext(SSLContext context) {
		mSslContext = context;
	}

	@Override
	protected synchronized Socket createSocket() {
		if (mSslContext == null) {
			return null;
		}

		SSLSocketFactory factory = mSslContext.getSocketFactory();
		if (factory != null) {
			try {
				return factory.createSocket();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return null;
	}

	public static void main(String[] args) {
		CavanSslContextBuilder builder = new CavanSslContextBuilder("/cavan/config/ssl/cert.jks");
		CavanTcpClient tcp = new CavanSslClient(builder.build());

		if (tcp.connect("127.0.0.1", 9901)) {
			tcp.send("123456\n".getBytes());

			byte[] bytes = new byte[1024];
			int length = tcp.read(bytes);

			CavanJava.dLog("length = " + length);

			if (length > 0) {
				CavanJava.dLog("read: " + new String(bytes));
			}
		}
	}
}
