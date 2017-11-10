package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class CavanTcpClient implements Runnable {

	private Socket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;
	private InetSocketAddress mAddress;

	private Thread mConnThread = new Thread(this);
	private boolean mConnDisabled = true;
	private boolean mConnected;

	public synchronized Socket getSocket() {
		return mSocket;
	}

	public synchronized void setSocket(Socket socket) {
		mSocket = socket;
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

	public synchronized boolean isConnected() {
		return mConnected;
	}

	public synchronized boolean isConnDisabled() {
		return mConnDisabled;
	}

	public synchronized boolean isConnEnabled() {
		return !mConnDisabled;
	}

	private synchronized void setConnDisabled(boolean disabled) {
		mConnDisabled = disabled;
	}

	public synchronized InetSocketAddress getAddress() {
		return mAddress;
	}

	public synchronized boolean setAddress(InetSocketAddress address) {
		if (mAddress != null && mAddress.equals(address)) {
			return false;
		}

		mAddress = address;

		closeSocket();

		return true;
	}

	public synchronized boolean setAddress(String host, int port) {
		return setAddress(new InetSocketAddress(host, port));
	}

	public void prErrInfo(String message) {
		CavanJava.eLog(message);
	}

	public void prWarnInfo(String message) {
		prErrInfo(message);
	}

	public void prDbgInfo(String message) {
		CavanJava.dLog(message);
	}

	protected Socket createSocket() {
		return new Socket();
	}

	public synchronized boolean connect() {
		mConnDisabled = false;

		try {
			synchronized (mConnThread) {
				if (mConnThread.isAlive()) {
					mConnThread.notify();
				} else {
					mConnThread.start();
				}
			}

			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	public synchronized boolean connect(InetSocketAddress address) {
		setAddress(address);
		return connect();
	}

	public synchronized boolean connect(String host, int port) {
		return connect(new InetSocketAddress(host, port));
	}

	public synchronized void disconnect() {
		mConnDisabled = true;
		mConnected = false;

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

			mSocket = null;
		}
	}

	public boolean join() {
		try {
			mConnThread.join();
			return true;
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean send(byte[] bytes, int offset, int length) {
		OutputStream stream = getOutputStream();

		if (stream != null) {
			try {
				synchronized (stream) {
					stream.write(bytes, offset, length);
					return true;
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public boolean send(byte[] bytes, int length) {
		return send(bytes, 0, length);
	}

	public boolean send(byte[] bytes) {
		return send(bytes, 0, bytes.length);
	}

	public boolean send(String text) {
		return send(text.getBytes());
	}

	public int read(byte[] bytes, int offset, int length) {
		InputStream stream = getInputStream();

		if (stream != null) {
			try {
				synchronized (stream) {
					return stream.read(bytes, offset, length);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return -1;
	}

	public int read(byte[] bytes, int length) {
		return read(bytes, 0, length);
	}

	public int read(byte[] bytes) {
		return read(bytes, 0, bytes.length);
	}

	public boolean fill(byte[] bytes, int offset, int length) {
		InputStream stream = getInputStream();

		if (stream != null) {
			synchronized (stream) {
				return CavanArray.fill(stream, bytes, offset, length);
			}
		}

		return false;
	}

	public boolean openSocket(InetSocketAddress address) {
		closeSocket();

		Socket socket = createSocket();
		if (socket == null) {
			prErrInfo("createSocket");
			return false;
		}

		synchronized (this) {
			mSocket = socket;
		}

		try {
			socket.connect(address, 10000);

			synchronized (this) {
				mOutputStream = socket.getOutputStream();
				mInputStream = socket.getInputStream();

				mConnected = true;

				if (onTcpConnected(socket)) {
					return true;
				}

				mConnected = false;
			}
		} catch (IOException e) {
			e.printStackTrace();
		}

		closeSocket();

		return false;
	}

	public synchronized void closeSocket() {
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

			mSocket = null;
		}
	}

	@Override
	public void run() {
		while (true) {
			int times;

			if (isConnEnabled()) {
				onTcpClientRunning();
			}

			times = 0;

			while (true) {
				InetSocketAddress address;

				synchronized (this) {
					if (mConnDisabled) {
						break;
					}

					address = mAddress;
				}

				prDbgInfo("address = " + address);

				if (address == null) {
					break;
				}

				if (openSocket(address)) {
					InputStream stream = getInputStream();
					if (stream != null) {
						mainLoop(stream);
					}

					times = 0;

					synchronized (this) {
						mConnected = false;
						onTcpDisconnected();

						if (mConnDisabled) {
							break;
						}
					}
				} else {
					if (isConnDisabled()) {
						break;
					}

					if (onTcpConnFailed(++times)) {
						int delay;

						if (times < 15) {
							delay = 1 << times;
						} else {
							delay = 1 << 15;
						}

						prDbgInfo("delay = " + delay);

						synchronized (mConnThread) {
							try {
								mConnThread.wait(delay);
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						}

					} else {
						break;
					}
				}
			}

			setConnDisabled(true);
			onTcpClientStopped();

			synchronized (mConnThread) {
				try {
					mConnThread.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	protected void onTcpClientRunning() {}

	protected void onTcpClientStopped() {}

	protected boolean onTcpConnected(Socket socket) {
		return true;
	}

	protected void onTcpDisconnected() {
	}

	protected boolean onTcpConnFailed(int times) {
		return true;
	}

	protected void onDataReceived(byte[] bytes, int length) {
	}

	protected void mainLoop(InputStream stream) {
		byte[] bytes = new byte[4096];

		while (true) {
			try {
				int length = stream.read(bytes);
				if (length > 0) {
					onDataReceived(bytes, length);
				}
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}
		}
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
