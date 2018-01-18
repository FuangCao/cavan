package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class CavanTcpClient implements Runnable {

	public interface CavanTcpClientListener {
		void onTcpClientRunning();
		void onTcpClientStopped();
		void onTcpConnecting(InetSocketAddress address);
		boolean onTcpConnected(Socket socket);
		void onTcpDisconnected();
		boolean onTcpConnFailed(int times);
		boolean onDataReceived(byte[] bytes, int length);
	}

	private Socket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;
	private InetSocketAddress mConnAddress;
	private InetSocketAddress mCurrAddress;
	private CavanTcpClientListener mTcpClientListener;
	private List<InetSocketAddress> mAddresses = new ArrayList<InetSocketAddress>();

	private Thread mConnThread = new Thread(this);
	private boolean mConnDisabled = true;
	private boolean mConnected;
	private int mConnTimes;

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

		if (mConnected) {
			return true;
		}

		return reconnect();
	}

	public synchronized boolean setAddress(InetSocketAddress address) {
		if (mAddresses.size() == 1 && mAddresses.get(0).equals(address)) {
			return false;
		}

		mAddresses.clear();

		return addAddresses(address);
	}

	public synchronized boolean setAddress(String host, int port) {
		return setAddress(new InetSocketAddress(host, port));
	}

	public synchronized boolean setAddresses(InetSocketAddress... addresses) {
		mAddresses.clear();

		boolean reconn = mConnected;

		for (InetSocketAddress address : addresses) {
			if (address == null) {
				continue;
			}

			mAddresses.add(address);

			if (reconn && address.equals(mCurrAddress)) {
				reconn = false;
			}
		}

		if (reconn) {
			return reconnect();
		}

		return true;
	}

	public synchronized boolean setAddresses(List<InetSocketAddress> addresses) {
		InetSocketAddress[] array = new InetSocketAddress[addresses.size()];
		addresses.toArray(array);
		return setAddresses(array);
	}

	public synchronized CavanTcpClientListener getTcpClientListener() {
		return mTcpClientListener;
	}

	public synchronized void setTcpClientListener(CavanTcpClientListener listener) {
		mTcpClientListener = listener;
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

	public synchronized boolean startConnThread() {
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

	public synchronized boolean connect() {
		mConnDisabled = false;
		mConnTimes = 0;
		return startConnThread();
	}

	public synchronized boolean reconnect() {
		closeSocket();

		if (mConnDisabled) {
			return true;
		}

		return startConnThread();
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
		CavanJava.dLog("openSocket: " + address);

		closeSocket();

		Socket socket = createSocket();
		if (socket == null) {
			prErrInfo("createSocket");
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

				mConnected = true;

				if (onTcpConnected(socket)) {
					mConnAddress = address;
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
			if (isConnEnabled()) {
				onTcpClientRunning();
			}

			mConnTimes = 0;

			while (true) {
				InetSocketAddress[] addresses;

				synchronized (this) {
					if (mConnDisabled) {
						break;
					}

					int count = mAddresses.size();
					if (count <= 0) {
						break;
					}

					addresses = new InetSocketAddress[count];
					mAddresses.toArray(addresses);

					if (mConnAddress != null) {
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
					InputStream stream = getInputStream();
					if (stream != null) {
						mainLoop(stream);
					}

					mConnTimes = 0;

					synchronized (this) {
						mConnected = false;
						onTcpDisconnected();

						if (mConnDisabled) {
							break;
						}
					}
				} else {
					mCurrAddress = null;

					if (isConnDisabled()) {
						break;
					}

					if (onTcpConnFailed(++mConnTimes)) {
						int delay;

						if (mConnTimes < 15) {
							delay = 1 << mConnTimes;
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

	protected void onTcpClientRunning() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientRunning();
		}
	}

	protected void onTcpClientStopped() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientStopped();
		}
	}

	protected void onTcpConnecting(InetSocketAddress address) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpConnecting(address);
		}
	}

	protected boolean onTcpConnected(Socket socket) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnected(socket);
		}

		return true;
	}

	protected void onTcpDisconnected() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpDisconnected();
		}
	}

	protected boolean onTcpConnFailed(int times) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnFailed(times);
		}

		return true;
	}

	protected boolean onDataReceived(byte[] bytes, int length) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onDataReceived(bytes, length);
		}

		return true;
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
