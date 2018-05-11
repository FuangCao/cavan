package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class CavanTcpClient {

	public interface CavanTcpClientListener {
		void onTcpClientRunning();
		void onTcpClientStopped();
		void onTcpConnecting(InetSocketAddress address);
		boolean onTcpConnected(Socket socket);
		void onTcpDisconnected();
		boolean onTcpConnFailed(int times);
		boolean onDataReceived(byte[] bytes, int length);
		void onTcpRecvTimeout();
	}

	private Socket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;
	private InetSocketAddress mConnAddress;
	private InetSocketAddress mCurrAddress;
	private CavanTcpClientListener mTcpClientListener;
	private List<InetSocketAddress> mAddresses = new ArrayList<InetSocketAddress>();

	private long mRecvTime;
	private int mRecvTimeout;

	private boolean mConnEnabled;
	private boolean mConnected;
	private int mConnTimes;

	private CavanThread mConnThread = new CavanThread() {

		@Override
		public void run() {
			while (true) {
				try {
					runConnThread();
				} catch (Exception e) {
					e.printStackTrace();
				}

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	private CavanThread mRecvThread = new CavanThread() {

		@Override
		public void run() {
			while (true) {
				try {
					InputStream stream = getInputStream();
					if (stream != null) {
						runRecvThread(stream);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}

				setConnected(false);

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	public synchronized void setRecvTimeout(int timeout) {
		mRecvTimeout = timeout;
	}

	public synchronized int getRecvTimeout() {
		return mRecvTimeout;
	}

	public synchronized void setRecvTime(long time) {
		mRecvTime = time;
	}

	public synchronized void setRecvTime() {
		if (mRecvTimeout > 0) {
			setRecvTime(System.currentTimeMillis());
		}
	}

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
		mConnThread.wakeup();
	}

	public synchronized boolean isConnEnabled() {
		return mConnEnabled;
	}

	public synchronized void setConnEnable(boolean enable) {
		mConnEnabled = enable;
	}

	public synchronized List<InetSocketAddress> getAddresses() {
		return mAddresses;
	}

	public synchronized InetSocketAddress getFirstAddress() {
		if (mAddresses.isEmpty()) {
			return null;
		}

		return mAddresses.get(0);
	}

	public synchronized InetSocketAddress getCurrentAddress() {
		return mCurrAddress;
	}

	public synchronized String getCurrentAddressString() {
		InetSocketAddress address = mCurrAddress;
		if (address == null) {
			return null;
		}

		return address.getHostString() + ':' + address.getPort();
	}

	public synchronized boolean hasAddress(InetSocketAddress address) {
		for (InetSocketAddress node : mAddresses) {
			if (node.equals(address)) {
				return true;
			}
		}

		return false;
	}

	public synchronized boolean addAddresses(InetSocketAddress address) {
		if (address == null) {
			return false;
		}

		if (hasAddress(address)) {
			return false;
		}

		mAddresses.add(address);

		if (mConnEnabled) {
			mConnThread.wakeup();
		}

		return true;
	}

	public synchronized void setAddresses(InetSocketAddress... addresses) {
		mAddresses.clear();

		boolean reconn = true;

		for (InetSocketAddress address : addresses) {
			if (address == null) {
				continue;
			}

			mAddresses.add(address);

			if (address.equals(mConnAddress)) {
				reconn = false;
			}
		}

		if (reconn) {
			reconnect();
		}
	}

	public void setAddresses(List<InetSocketAddress> addresses) {
		InetSocketAddress[] array = new InetSocketAddress[addresses.size()];
		addresses.toArray(array);
		setAddresses(array);
	}

	public void setAddress(InetSocketAddress address) {
		setAddresses(address);
	}

	public synchronized void setAddress(String host, int port) {
		setAddress(new InetSocketAddress(host, port));
	}

	public synchronized CavanTcpClientListener getTcpClientListener() {
		return mTcpClientListener;
	}

	public synchronized void setTcpClientListener(CavanTcpClientListener listener) {
		mTcpClientListener = listener;
	}

	protected Socket createSocket() {
		return new Socket();
	}

	public synchronized void connect() {
		mConnTimes = 0;
		mConnEnabled = true;
		mConnThread.wakeup();
	}

	public synchronized void reconnect() {
		mConnected = false;
		mConnThread.wakeup();
	}

	public synchronized void connect(InetSocketAddress address) {
		setAddress(address);
		connect();
	}

	public synchronized void connect(String host, int port) {
		connect(new InetSocketAddress(host, port));
	}

	public synchronized void disconnect() {
		mConnected = false;
		mConnEnabled = false;
		mConnThread.wakeup();
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

	private boolean openSocket(InetSocketAddress address) {
		CavanJava.dLog("openSocket: " + address);

		closeSocket();

		Socket socket = createSocket();
		if (socket == null) {
			CavanJava.eLog("createSocket");
			return false;
		}

		synchronized (this) {
			mSocket = socket;
			mCurrAddress = address;
		}

		onTcpConnecting(address);

		try {
			socket.connect(address, 6000);

			synchronized (this) {
				mOutputStream = socket.getOutputStream();
				mInputStream = socket.getInputStream();
				mConnAddress = address;
				mConnected = true;

				if (onTcpConnected(socket)) {
					return true;
				}

				mConnected = false;
				mConnAddress = null;
			}
		} catch (IOException e) {
			// e.printStackTrace();
		}

		closeSocket();

		return false;
	}

	private synchronized void closeSocket() {
		mConnected = false;

		if (mConnAddress != null) {
			mConnAddress = null;
			onTcpDisconnected();
		}

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

	protected void runConnThread() {
		if (isConnEnabled()) {
			onTcpClientRunning();
		}

		mConnTimes = 0;

		while (true) {
			InetSocketAddress[] addresses;

			synchronized (this) {
				if (!mConnEnabled) {
					break;
				}

				int count = mAddresses.size();
				if (count <= 0) {
					break;
				}

				addresses = new InetSocketAddress[count];
				mAddresses.toArray(addresses);

				if (mConnAddress != null && count > 1) {
					for (int i = 0; i < count; i++) {
						if (addresses[i].equals(mConnAddress)) {
							while (i > 0) {
								addresses[i] = addresses[i - 1];
								i--;
							}

							addresses[0] = mConnAddress;
							break;
						}
					}
				}
			}

			boolean success = false;

			for (int i = 0; i < addresses.length; i++) {
				success = openSocket(addresses[i]);
				if (success) {
					break;
				}
			}

			if (success) {
				mRecvThread.wakeup();

				if (mRecvTimeout > 0) {
					mRecvTime = System.currentTimeMillis();

					while (isConnected() && isConnEnabled()) {
						long overtime = System.currentTimeMillis() - mRecvTime;

						if (overtime < mRecvTimeout) {
							long delay = mRecvTimeout - overtime;

							synchronized (mConnThread) {
								try {
									mConnThread.wait(delay);
								} catch (InterruptedException e) {
									e.printStackTrace();
								}
							}
						} else {
							onTcpRecvTimeout();
							break;
						}
					}
				} else {
					while (isConnected() && isConnEnabled()) {
						synchronized (mConnThread) {
							try {
								mConnThread.wait();
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						}
					}
				}

				closeSocket();

				mConnTimes = 0;

				if (!isConnEnabled()) {
					break;
				}
			} else {
				if (!isConnEnabled()) {
					break;
				}

				if (onTcpConnFailed(++mConnTimes)) {
					int delay;

					if (mConnTimes < 15) {
						delay = 1 << mConnTimes;
					} else {
						delay = 1 << 15;
					}

					CavanJava.dLog("delay = " + delay);

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

		setConnEnable(false);
		onTcpClientStopped();
	}

	protected void runRecvThread(InputStream stream) {
		byte[] bytes = new byte[4096];

		while (true) {
			try {
				int length = stream.read(bytes);
				if (length > 0) {
					setRecvTime();
					onDataReceived(bytes, length);
				} else {
					break;
				}
			} catch (IOException e) {
				// e.printStackTrace();
				break;
			}
		}

		synchronized (this) {
			mConnected = false;
			mConnThread.wakeup();
		}
	}

	protected void onTcpClientRunning() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientRunning();
		} else {
			CavanJava.dLog("onTcpClientRunning");
		}
	}

	protected void onTcpClientStopped() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientStopped();
		} else {
			CavanJava.dLog("onTcpClientStopped");
		}
	}

	protected void onTcpConnecting(InetSocketAddress address) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpConnecting(address);
		} else {
			CavanJava.dLog("onTcpClientRunning");
		}
	}

	protected boolean onTcpConnected(Socket socket) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnected(socket);
		}

		CavanJava.dLog("onTcpConnected");

		return true;
	}

	protected void onTcpDisconnected() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpDisconnected();
		} else {
			CavanJava.dLog("onTcpDisconnected");
		}
	}

	protected boolean onTcpConnFailed(int times) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnFailed(times);
		}

		CavanJava.dLog("onTcpConnFailed");

		return true;
	}

	protected void onTcpRecvTimeout() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpRecvTimeout();
		} else {
			CavanJava.dLog("onTcpRecvTimeout");
		}
	}

	protected boolean onDataReceived(byte[] bytes, int length) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onDataReceived(bytes, length);
		}

		CavanJava.dLog("onDataReceived");

		return true;
	}

	public static void main(String[] args) {
		CavanTcpClient tcp = new CavanTcpClient() {

			@Override
			protected boolean onTcpConnected(Socket socket) {
				send("123456".getBytes());
				return true;
			}

			@Override
			protected boolean onDataReceived(byte[] bytes, int length) {
				CavanJava.dLog("onDataReceived: " + new String(bytes, 0, length));
				return true;
			}
		};

		tcp.setRecvTimeout(10000);
		tcp.connect("127.0.0.1", 9901);
	}
}
