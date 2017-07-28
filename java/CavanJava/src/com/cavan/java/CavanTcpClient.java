package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class CavanTcpClient {

	private Socket mSocket;
	private InetSocketAddress mAddress;
	private InputStream mInputStream;
	private OutputStream mOutputStream;

	public synchronized Socket getSocket() {
		return mSocket;
	}

	public synchronized void setSocket(Socket socket) {
		mSocket = socket;
	}

	public synchronized InetSocketAddress getAddress() {
		return mAddress;
	}

	public synchronized void setAddress(InetSocketAddress address) {
		mAddress = address;
	}

	public synchronized void setAddress(String host, int port) {
		mAddress = new InetSocketAddress(host, port);
	}

	public synchronized InputStream getInputStream() {
		return mInputStream;
	}

	public synchronized void setInputStream(InputStream inputStream) {
		mInputStream = inputStream;
	}

	public synchronized OutputStream getOutputStream() {
		return mOutputStream;
	}

	public synchronized void setOutputStream(OutputStream outputStream) {
		mOutputStream = outputStream;
	}

	protected synchronized Socket createSocket() {
		return new Socket();
	}

	public synchronized boolean connect() {
		if (mAddress == null) {
			return false;
		}

		disconnect();

		mSocket = createSocket();
		if (mSocket == null) {
			return false;
		}

		try {
			mSocket.connect(mAddress);
			mInputStream = mSocket.getInputStream();
			mOutputStream = mSocket.getOutputStream();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	public synchronized boolean connect(String host, int port) {
		mAddress = new InetSocketAddress(host, port);
		return connect();
	}

	public synchronized void disconnect() {
		if (mInputStream != null) {
			try {
				mInputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			mInputStream = null;
		}

		if (mOutputStream != null) {
			try {
				mOutputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			mOutputStream = null;
		}

		if (mSocket != null) {
			try {
				mSocket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public boolean send(byte[] bytes) {
		OutputStream stream = getOutputStream();

		if (stream != null) {
			try {
				stream.write(bytes);
				return true;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public int read(byte[] bytes) {
		InputStream stream = getInputStream();

		if (stream != null) {
			try {
				return stream.read(bytes);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return -1;
	}

	public boolean fill(byte[] bytes) {
		InputStream stream = getInputStream();

		if (stream == null) {
			return false;
		}

		int offset = 0;

		while (offset < bytes.length) {
			try {
				int length = stream.read(bytes, offset, bytes.length - offset);
				if (length < 0) {
					return false;
				}

				offset += length;
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}
		}

		return true;
	}

	public static void main(String[] args) {
		CavanTcpClient tcp = new CavanTcpClient();

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
