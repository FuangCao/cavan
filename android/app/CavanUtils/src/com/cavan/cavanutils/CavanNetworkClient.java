package com.cavan.cavanutils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public abstract class CavanNetworkClient extends CavanUtils {
	protected InputStream mInputStream;
	protected OutputStream mOutputStream;
	private boolean mConnected;

	protected abstract boolean openSocket();
	protected abstract void closeSocket();
	protected abstract InputStream getInputStream();
	protected abstract OutputStream getOutputStream();

	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}

	private boolean openInputStream() {
		if (mInputStream != null) {
			return true;
		}

		if (connect()) {
			mInputStream = getInputStream();
			return mInputStream != null;
		}

		return false;
	}

	public void closeInputStream() {
		if (mInputStream != null) {
			try {
				mInputStream.close();
			} catch (IOException e) {
				// e.printStackTrace();
			}

			mInputStream = null;
		}
	}

	private boolean openOutputStream() {
		if (mOutputStream != null) {
			return true;
		}

		if (connect()) {
			mOutputStream = getOutputStream();
			return mOutputStream != null;
		}

		return false;
	}

	public void closeOutputStream() {
		if (mOutputStream != null) {
			try {
				mOutputStream.close();
			} catch (IOException e) {
				// e.printStackTrace();
			}

			mOutputStream = null;
		}
	}

	public void disconnect() {
		closeInputStream();
		closeOutputStream();
		closeSocket();

		mConnected = false;
	}

	public boolean connect() {
		if (mConnected) {
			return true;
		}

		mConnected = openSocket();

		return mConnected;
	}

	public boolean sendData(byte[] data) {
		if (!openOutputStream()) {
			return false;
		}

		try {
			mOutputStream.write(data);
			mOutputStream.flush();
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		disconnect();

		return false;
	}

	public int recvData(byte[] bytes) {
		if (!openInputStream()) {
			return -1;
		}

		try {
			return mInputStream.read(bytes);
		} catch (IOException e) {
			e.printStackTrace();
		}

		disconnect();

		return -1;
	}
}
