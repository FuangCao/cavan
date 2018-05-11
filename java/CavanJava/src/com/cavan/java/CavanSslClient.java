package com.cavan.java;

import java.io.IOException;
import java.net.Socket;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class CavanSslClient extends CavanTcpClient {

	private SSLContext mSslContext;

	public CavanSslClient(SSLContext context) {
		mSslContext = context;
	}

	public CavanSslClient() {
		mSslContext = null;
	}

	public synchronized SSLContext getSslContext() {
		if (mSslContext == null) {
			TrustManager manager = new X509TrustManager() {

				@Override
				public X509Certificate[] getAcceptedIssuers() {
					CavanJava.dLog("getAcceptedIssuers");
					return null;
				}

				@Override
				public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
					CavanJava.dLog("checkServerTrusted");
				}

				@Override
				public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
					CavanJava.dLog("checkClientTrusted");
				}
			};

			try {
				SSLContext context = SSLContext.getInstance("TLS");
				context.init(null, new TrustManager[] { manager }, null);
				mSslContext = context;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		return mSslContext;
	}

	public synchronized void setSslContext(SSLContext context) {
		mSslContext = context;
	}

	@Override
	protected Socket createSocket() {
		SSLContext context = getSslContext();
		if (context == null) {
			CavanJava.eLog("getSslContext");
			return null;
		}

		SSLSocketFactory factory = context.getSocketFactory();
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
		CavanTcpClient tcp = new CavanSslClient() {

			@Override
			protected boolean onTcpConnected(Socket socket) {
				return send("123456\n".getBytes());
			}

			@Override
			protected boolean onDataReceived(byte[] bytes, int length) {
				CavanJava.dLog("onDataReceived: " + new String(bytes, 0, length));
				return true;
			}
		};

		tcp.connect("127.0.0.1", 9901);
	}
}
