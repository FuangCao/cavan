package com.cavan.cavanutils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.InetAddress;

import android.net.LocalSocketAddress;

public class CavanNetworkClient extends CavanUtils {

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

	public DatagramPacket getPacket() {
		return mClientImpl.getPacket();
	}

	public InetAddress getRemoteAddress() {
		DatagramPacket packet = mClientImpl.getPacket();
		if (packet == null) {
			return null;
		}

		return packet.getAddress();
	}

	public int getRemotePort() {
		DatagramPacket packet = mClientImpl.getPacket();
		if (packet == null) {
			return 0;
		}

		return packet.getPort();
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

		if (connect()) {
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

	public final boolean connect() {
		synchronized (mClientImpl) {
			if (mConnected) {
				return true;
			}

			if (!mClientImpl.openSocket()) {
				return false;
			}

			mConnected = true;

			if (sendRequest()) {
				OnConnected();
				return true;
			}

			mConnected = false;
			mClientImpl.closeSocket();
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
