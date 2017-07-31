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
	private boolean mConnDisabled;
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

	public synchronized void setConnected(boolean connected) {
		mConnected = connected;
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

	public boolean fill(byte[] bytes, int offset, int length) {
		InputStream stream = getInputStream();

		if (stream != null) {
			return CavanArray.fill(stream, bytes, offset, length);
		}

		return false;
	}

	public boolean openSocket(InetSocketAddress address) {
		closeSocket();

		Socket socket = createSocket();
		if (socket == null) {
			CavanJava.eLog("createSocket");
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
			}

			if (onTcpConnected(socket)) {
				return true;
			}

			synchronized (this) {
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
			int times = 0;

			while (true) {
				InetSocketAddress address;

				synchronized (this) {
					if (mConnDisabled) {
						break;
					}

					address = mAddress;
				}

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

						if (mConnDisabled) {
							break;
						}
					}

					onTcpDisconnected();
				} else {
					synchronized (this) {
						if (mConnDisabled) {
							break;
						}
					}

					if (onTcpConnFailed(++times)) {
						int delay;

						if (times < 15) {
							delay = 1 << times;
						} else {
							delay = 1 << 15;
						}

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

			synchronized (mConnThread) {
				try {
					mConnThread.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

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
