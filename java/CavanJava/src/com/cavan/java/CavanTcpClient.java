package com.cavan.java;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

public class CavanTcpClient {

	public interface CavanTcpClientListener {
		void onTcpClientRunning();
		void onTcpClientStopped();
		void onTcpConnecting(InetSocketAddress address);
		boolean onTcpConnected(Socket socket);
		void onTcpDisconnected();
		long onTcpConnFailed(int times);
		boolean onDataReceived(byte[] bytes, int length);
		void onKeepAliveFailed();
	}

	public class ConnThread extends CavanThread {

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

	public class RecvThread extends CavanThread {

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
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		}
	}

	public class SendThread extends CavanThread {

		private LinkedBlockingQueue<byte[]> mQueue = new LinkedBlockingQueue<byte[]>();

		@Override
		public void run() {
			while (true) {
				try {
					byte[] bytes = mQueue.take();

					if (isConnected() && CavanTcpClient.this.send(bytes)) {
						continue;
					}

					mQueue.clear();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		public boolean send(byte[] bytes) {
			try {
				if (isConnected()) {
					mQueue.put(bytes);
					return true;
				}
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			return false;
		}

		public boolean send(String text) {
			return send(text.getBytes());
		}
	}

	public static InetSocketAddress newSocketAddress(String host, int port) {
		return InetSocketAddress.createUnresolved(host, port);
	}

	public static InetSocketAddress newSocketAddressByUrl(String url, int port) {
		int index = url.indexOf(':');
		String host;

		if (index < 0) {
			host = url.trim();
		} else if (index > 0) {
			host = url.substring(0, index).trim();
			port = CavanJava.parseInt(url.substring(index + 1).trim());
		} else {
			host = "127.0.0.1";
			port = CavanJava.parseInt(url.trim());
		}

		return newSocketAddress(host, port);
	}

	public static InetSocketAddress[] newSocketAddresses(int port, Collection<String> urls) {
		InetSocketAddress[] addresses = new InetSocketAddress[urls.size()];
		int i = 0;

		for (String url : urls) {
			addresses[i++] = newSocketAddressByUrl(url, port);
		}

		return addresses;
	}

	public static InetSocketAddress[] newSocketAddresses(int port, String... urls) {
		InetSocketAddress[] addresses = new InetSocketAddress[urls.length];
		int i = 0;

		for (String url : urls) {
			addresses[i++] = newSocketAddressByUrl(url, port);
		}

		return addresses;
	}

	public static InetSocketAddress resolveSocketAddress(InetSocketAddress address) throws UnknownHostException {
		String host = address.getHostName();
		if (host == null) {
			return address;
		}

		return new InetSocketAddress(InetAddress.getByName(host), address.getPort());
	}

	private Socket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;
	private InetSocketAddress mConnAddress;
	private InetSocketAddress mCurrAddress;
	private CavanTcpClientListener mTcpClientListener;
	private List<InetSocketAddress> mAddresses = new ArrayList<InetSocketAddress>();

	private int mKeepAliveTimes;

	private boolean mConnEnabled;
	private boolean mConnected;
	private int mConnTimes;

	private ConnThread mConnThread = new ConnThread();
	private RecvThread mRecvThread = new RecvThread();

	public synchronized void touchKeepAlive() {
		mKeepAliveTimes = 0;
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

	public SendThread newSendThread() {
		SendThread thread = new SendThread();
		thread.start();
		return thread;
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

		if (isConnEnabled()) {
			mConnThread.wakeup();
		}

		return true;
	}

	public synchronized int setAddresses(InetSocketAddress... addresses) {
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

		return addresses.length;
	}

	public void setAddresses(List<InetSocketAddress> addresses) {
		InetSocketAddress[] array = new InetSocketAddress[addresses.size()];
		addresses.toArray(array);
		setAddresses(array);
	}

	public int setAddressesByUrl(int port, Collection<String> urls) {
		InetSocketAddress[] addresses = newSocketAddresses(port, urls);
		return setAddresses(addresses);
	}

	public int setAddressesByUrl(int port, String... urls) {
		InetSocketAddress[] addresses = newSocketAddresses(port, urls);
		return setAddresses(addresses);
	}

	public void setAddress(InetSocketAddress address) {
		setAddresses(address);
	}

	public synchronized void setAddress(String host, int port) {
		setAddress(newSocketAddress(host, port));
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
		connect();
	}

	public synchronized void connect(InetSocketAddress address) {
		setAddress(address);
		connect();
	}

	public synchronized void connect(String host, int port) {
		connect(newSocketAddress(host, port));
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
			} catch (Exception e) {
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
		prDbgInfo("openSocket: " + address);

		closeSocket();

		Socket socket = createSocket();
		if (socket == null) {
			return false;
		}

		synchronized (this) {
			mSocket = socket;
			mCurrAddress = address;
		}

		onTcpConnecting(address);

		try {
			socket.connect(resolveSocketAddress(address), onGetConnOvertime());

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
		} catch (Exception e) {
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
			} catch (Exception e) {
				e.printStackTrace();
			}

			mInputStream = null;
		}

		if (mOutputStream != null) {
			try {
				mOutputStream.close();
			} catch (Exception e) {
				e.printStackTrace();
			}

			mOutputStream = null;
		}

		if (mSocket != null) {
			try {
				mSocket.close();
			} catch (Exception e) {
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
				if (!isConnEnabled()) {
					break;
				}

				List<InetSocketAddress> list = getAddresses();
				if (list == null) {
					break;
				}

				int count = list.size();
				if (count <= 0) {
					break;
				}

				addresses = new InetSocketAddress[count];
				list.toArray(addresses);

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

				int delay = onGetKeepAliveDelay();
				if (delay > 0) {
					long time = System.currentTimeMillis();

					mKeepAliveTimes = 0;

					while (isConnected() && isConnEnabled()) {
						long now = System.currentTimeMillis();
						long overtime = now - time;
						long remain;

						if (overtime < delay) {
							remain = delay - overtime;
							if (remain > delay) {
								remain = delay;
							}
						} else {
							if (mKeepAliveTimes > 0) {
								onKeepAliveFailed();
								break;
							}

							int times = doTcpKeepAlive(1);
							if (times < 0) {
								onKeepAliveFailed();
								break;
							}

							mKeepAliveTimes = times;
							remain = delay;
							time = now;
						}

						synchronized (mConnThread) {
							try {
								mConnThread.wait(remain);
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
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

				long delay = onTcpConnFailed(++mConnTimes);
				if (delay < 0) {
					break;
				}

				if (delay == 0) {
					delay = getConnDelay(mConnTimes);
				}

				prDbgInfo("delay = " + delay);

				synchronized (mConnThread) {
					try {
						mConnThread.wait(delay);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}

		setConnEnable(false);
		onTcpClientStopped();
	}

	protected int doTcpKeepAlive(int times) {
		return 0;
	}

	protected int onGetConnOvertime() {
		return 10000;
	}

	protected int onGetKeepAliveDelay() {
		return 0;
	}

	protected void runRecvThread(InputStream stream) {
		byte[] bytes = new byte[4096];

		while (true) {
			try {
				int length = stream.read(bytes);
				if (length > 0) {
					touchKeepAlive();
					onDataReceived(bytes, length);
				} else {
					break;
				}
			} catch (Exception e) {
				// e.printStackTrace();
				break;
			}
		}

		synchronized (this) {
			mConnected = false;
			mConnThread.wakeup();
		}
	}

	public void prDbgInfo(String message) {
		CavanJava.dLog(message);
	}

	public void prWarnInfo(String message) {
		CavanJava.wLog(message);
	}

	public void prErrInfo(String message) {
		CavanJava.eLog(message);
	}

	protected void onTcpClientRunning() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientRunning();
		} else {
			prDbgInfo("onTcpClientRunning");
		}
	}

	protected void onTcpClientStopped() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpClientStopped();
		} else {
			prDbgInfo("onTcpClientStopped");
		}
	}

	protected void onTcpConnecting(InetSocketAddress address) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpConnecting(address);
		} else {
			prDbgInfo("onTcpClientRunning");
		}
	}

	protected boolean onTcpConnected(Socket socket) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnected(socket);
		}

		prDbgInfo("onTcpConnected");

		return true;
	}

	protected void onTcpDisconnected() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onTcpDisconnected();
		} else {
			prDbgInfo("onTcpDisconnected");
		}
	}

	protected long getConnDelay(int times) {
		if (times < 12) {
			return (1 << times);
		}

		return (1 << 12);
	}

	protected long onTcpConnFailed(int times) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onTcpConnFailed(times);
		}

		if (times > 20) {
			return -1;
		}

		prDbgInfo("onTcpConnFailed");

		return 0;
	}

	protected void onKeepAliveFailed() {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			listener.onKeepAliveFailed();
		} else {
			prDbgInfo("onKeepAliveFailed");
		}
	}

	protected boolean onDataReceived(byte[] bytes, int length) {
		CavanTcpClientListener listener = getTcpClientListener();
		if (listener != null) {
			return listener.onDataReceived(bytes, length);
		}

		prDbgInfo("onDataReceived");

		return true;
	}

	public static void main(String[] args) {
		CavanTcpClient tcp = new CavanTcpClient() {

			@Override
			protected boolean onTcpConnected(Socket socket) {
				super.onTcpConnected(socket);
				send("123456");
				return true;
			}

			@Override
			protected boolean onDataReceived(byte[] bytes, int length) {
				prDbgInfo("onDataReceived: " + new String(bytes, 0, length));
				return true;
			}

			@Override
			protected int doTcpKeepAlive(int times) {
				CavanJava.pLog();
				send("KeepAlive");
				return 0;
			}

			@Override
			protected int onGetKeepAliveDelay() {
				return 5000;
			}
		};

		tcp.connect("127.0.0.1", 9901);
	}
}
