package com.cavan.cavanutils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;

import android.net.LocalSocketAddress;

public class CavanNetworkClient extends CavanUtils {

	interface ICavanNetworkClient {
		public boolean openSocket();
		public void closeSocket();
		public InputStream getInputStream();
		public OutputStream getOutputStream();
	}

	protected InputStream mInputStream;
	protected OutputStream mOutputStream;

	private boolean mConnected;
	private ICavanNetworkClient mClient;

	public CavanNetworkClient(ICavanNetworkClient client) {
		mClient = client;
	}

	public CavanNetworkClient(LocalSocketAddress address) {
		this(new CavanUnixClient(address));
	}

	public CavanNetworkClient(InetAddress address, int port) {
		this(new CavanTcpClient(address, port));
	}

	public CavanNetworkClient(String pathname) {
		this(new LocalSocketAddress(pathname, LocalSocketAddress.Namespace.FILESYSTEM));
	}

	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}

	protected void OnDisconnected() {
	}

	protected void OnConnected() {
	}

	protected boolean sendRequest() {
		return true;
	}

	synchronized private boolean openInputStream() {
		if (mInputStream != null) {
			return true;
		}

		if (connect()) {
			mInputStream = mClient.getInputStream();
			return mInputStream != null;
		}

		return false;
	}

	synchronized public void closeInputStream() {
		if (mInputStream != null) {
			try {
				mInputStream.close();
			} catch (IOException e) {
				// e.printStackTrace();
			}

			mInputStream = null;
		}
	}

	synchronized private boolean openOutputStream() {
		if (mOutputStream != null) {
			return true;
		}

		if (connect()) {
			mOutputStream = mClient.getOutputStream();
			return mOutputStream != null;
		}

		return false;
	}

	synchronized public void closeOutputStream() {
		if (mOutputStream != null) {
			try {
				mOutputStream.close();
			} catch (IOException e) {
				// e.printStackTrace();
			}

			mOutputStream = null;
		}
	}

	public final void disconnect() {
		synchronized (mClient) {
			if (!mConnected) {
				return;
			}

			closeInputStream();
			closeOutputStream();
			mClient.closeSocket();

			mConnected = false;
		}

		OnDisconnected();
	}

	public final boolean connect() {
		synchronized (mClient) {
			if (mConnected) {
				return true;
			}

			if (!mClient.openSocket()) {
				return false;
			}

			mConnected = true;

			if (sendRequest()) {
				OnConnected();
				return true;
			}

			mConnected = false;
			mClient.closeSocket();
		}

		return false;
	}

	public final void connectNoSync() {
		new Thread() {

			@Override
			public void run() {
				connect();
			}
		}.start();
	}

	public boolean isConnected() {
		synchronized (mClient) {
			return mConnected;
		}
	}

	synchronized public boolean sendData(byte[] data) {
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

	synchronized public int recvData(byte[] bytes) {
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
