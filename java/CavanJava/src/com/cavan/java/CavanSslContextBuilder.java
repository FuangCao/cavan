package com.cavan.java;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

public class CavanSslContextBuilder {

	private String mCertPath;
	private char[] mPassword;
	private InputStream mCertStream;

	public CavanSslContextBuilder(String path) {
		mCertPath = path;
	}

	public CavanSslContextBuilder(InputStream stream) {
		mCertStream = stream;
	}

	public String getCertPath() {
		return mCertPath;
	}

	public void setCertPath(String certPath) {
		mCertPath = certPath;
	}

	public char[] getPassword() {
		return mPassword;
	}

	public void setPassword(String password) {
		if (password == null) {
			mPassword = null;
		} else {
			mPassword = password.toCharArray();
		}
	}

	public SSLContext build(InputStream certStream) {
		try {
			KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
			ks.load(certStream, mPassword);

			TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
			tmf.init(ks);

			SSLContext context = SSLContext.getInstance("TLS");
			context.init(null, tmf.getTrustManagers(), null);
			return context;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public SSLContext build(String certPath) {
		FileInputStream stream = null;

		try {
			stream = new FileInputStream(certPath);
			return build(stream);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} finally {
			if (stream != null) {
				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return null;
	}

	public SSLContext build() {
		if (mCertStream != null) {
			return build(mCertStream);
		}

		if (mCertPath != null) {
			return build(mCertPath);
		}

		return null;
	}
}
