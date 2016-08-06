package com.cavan.android;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;

import android.net.LocalSocketAddress;

public class CavanNetworkClient extends CavanAndroid {

	protected InputStream mInputStream;
	protected OutputStream mOutputStream;

	private boolean mConnected;
	private CavanNetworkClientImpl mClientImpl;

	public CavanNetworkClient() {
		super();
	}

	public CavanNetworkClient(CavanNetworkClientImpl client) {
		this();
		setClient(client);
	}

	public CavanNetworkClient(LocalSocketAddress address) {
		this(new CavanUnixClient(address));
	}

	public CavanNetworkClient(String pathname) {
		this(new LocalSocketAddress(pathname, LocalSocketAddress.Namespace.FILESYSTEM));
	}

	public CavanNetworkClient(InetAddress address, int port) {
		this(new CavanTcpClient(address, port));
	}

	public CavanNetworkClient(InetAddress address, int port, boolean udp) {
		this();

		if (udp) {
			setClient(new CavanUdpClient(address, port));
		} else {
			setClient(new CavanTcpClient(address, port));
		}
	}

	public void setClient(CavanNetworkClientImpl client) {
		mClientImpl = client;
	}

	public CavanNetworkClientImpl getClient() {
		return mClientImpl;
	}

	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}

	public Object getRemoteAddress() {
		return mClientImpl.getRemoteAddress();
	}

	public int getRemotePort() {
		return mClientImpl.getRemotePort();
	}

	public void setAddress(Object address) {
		mClientImpl.setAddress(address);
	}

	public void setPort(int port) {
		mClientImpl.setPort(port);
	}

	protected void OnDisconnected() {
		/* empty */
	}

	protected boolean sendRequest() {
		return true;
	}

	synchronized private boolean openInputStream() {
		if (mInputStream != null) {
			return true;
		}

		if (mConnected) {
			mInputStream = mClientImpl.getInputStream();
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

		if (mConnected) {
			mOutputStream = mClientImpl.getOutputStream();
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
		synchronized (mClientImpl) {
			if (!mConnected) {
				return;
			}

			closeInputStream();
			closeOutputStream();
			mClientImpl.closeSocket();

			mConnected = false;
		}

		OnDisconnected();
	}

	public final boolean connectSync() {
		synchronized (mClientImpl) {
			if (mConnected) {
				return true;
			}

			if (!mClientImpl.openSocket()) {
				return false;
			}

			mConnected = true;

			if (sendRequest()) {
				return true;
			}

			mConnected = false;
			mClientImpl.closeSocket();
		}

		return false;
	}

	public final boolean connectThreaded() {
		synchronized (mClientImpl) {
			if (mConnected) {
				return true;
			}
		}

		Thread thread = new Thread() {

			@Override
			public void run() {
				connectSync();
			}
		};

		thread.start();

		try {
			thread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		return mConnected;
	}

	public boolean isConnected() {
		synchronized (mClientImpl) {
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

	public boolean sendValue8(byte value) {
		return sendData(new byte[] { value } );
	}

	public boolean sendValue16(short value) {
		return sendData(new byte[] { (byte) (value & 0xFF), (byte) (value >> 8) } );
	}

	public boolean sendValue32(int value) {
		byte[] bytes = {
			(byte) (value & 0xFF),
			(byte) ((value >> 8) & 0xFF),
			(byte) ((value >> 16) & 0xFF),
			(byte) ((value >> 24) & 0xFF)
		};

		return sendData(bytes);
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
