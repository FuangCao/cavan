package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashSet;

public class CavanTcpService extends Thread {

	private int mPort;
	private boolean mEnabled;
	private ServerSocket mServerSocket;
	private HashSet<Client> mClients = new HashSet<Client>();

	public CavanTcpService(int port) {
		mPort = port;
	}

	public synchronized int getPort() {
		return mPort;
	}

	public synchronized void setPort(int port) {
		if (mPort != port) {
			mPort = port;

			if (mServerSocket != null) {
				try {
					mServerSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

				mServerSocket = null;
			}
		}
	}

	public synchronized boolean isEnabled() {
		return mEnabled;
	}

	public synchronized ServerSocket getServerSocket() {
		return mServerSocket;
	}

	public synchronized HashSet<Client> getClients() {
		return mClients;
	}

	public synchronized int getClientCount() {
		return mClients.size();
	}

	public synchronized void open() {
		mEnabled = true;

		if (isAlive()) {
			notify();
		} else {
			start();
		}
	}

	public synchronized void close() {
		mEnabled = false;

		if (mServerSocket != null) {
			try {
				mServerSocket.close();
				mServerSocket = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	public void run() {
		while (true) {
			while (mEnabled) {
				ServerSocket socket = null;

				try {
					socket = new ServerSocket(mPort);

					synchronized (this) {
						mServerSocket = socket;
					}

					onServerOpened(socket);
					doServerMainLoop(socket);
					onServerClosed();
				} catch (IOException e) {
					CavanJava.eLog("port = " + mPort);
					e.printStackTrace();
				} finally {
					synchronized (this) {
						mServerSocket = null;
					}

					if (socket != null) {
						try {
							socket.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
				}

				synchronized (this) {
					try {
						wait(2000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
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

	protected void doServerMainLoop(ServerSocket socket) {
		while (isEnabled()) {
			try {
				Socket conn = socket.accept();
				Client client = doCreateClient(conn);
				if (client == null) {
					conn.close();
					break;
				}

				client.start();
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}
		}

		synchronized (mClients) {
			for (Client client : mClients) {
				client.close();
			}
		}
	}

	public void doClientMainLoop(Client client, InputStream stream) {
		byte[] bytes = client.getBytes(1024);

		while (isEnabled()) {
			try {
				int length = stream.read(bytes);
				if (length < 0) {
					break;
				}

				if (!onDataReceived(client, bytes, length)) {
					break;
				}
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}
		}
	}

	protected boolean onDataReceived(Client client, byte[] bytes, int length) {
		return false;
	}

	protected void onServerOpened(ServerSocket socket) {}

	protected void onServerClosed() {}

	protected void onClientDisconnected(Client client) {}

	protected void onClientConnected(Client client) {}

	protected Client doCreateClient(Socket socket) {
		return new Client(socket);
	}

	public class Client extends Thread {

		private Socket mSocket;
		private InputStream mInputStream;
		private OutputStream mOutputStream;
		private byte[] mBytes = new byte[0];

		public Client(Socket socket) {
			mSocket = socket;
		}

		public synchronized Socket getSocket() {
			return mSocket;
		}

		public synchronized InputStream getInputStream() {
			return mInputStream;
		}

		public synchronized OutputStream getOutputStream() {
			return mOutputStream;
		}

		public byte[] getBytes(int length) {
			if (length > mBytes.length) {
				mBytes = new byte[length];
			}

			return mBytes;
		}

		public byte[] getBytes() {
			return mBytes;
		}

		public boolean send(byte[] bytes, int offset, int length) {
			try {
				OutputStream stream = getOutputStream();
				if (stream != null) {
					stream.write(bytes, offset, length);
					return true;
				}
			} catch (IOException e) {
				e.printStackTrace();
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

		public synchronized void close() {
			if (mInputStream != null) {
				try {
					mInputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				} finally {
					mInputStream = null;
				}
			}

			if (mOutputStream != null) {
				try {
					mOutputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				} finally {
					mOutputStream = null;
				}
			}

			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				} finally {
					mSocket = null;
				}
			}
		}

		@Override
		public void run() {
			InputStream istream = null;
			OutputStream ostream = null;

			try {
				mSocket.setTcpNoDelay(true);

				istream = mSocket.getInputStream();
				ostream = mSocket.getOutputStream();

				mInputStream = istream;
				mOutputStream = ostream;

				onClientConnected(this);

				synchronized (mClients) {
					mClients.add(this);
				}

				doClientMainLoop(this, istream);

				synchronized (mClients) {
					mClients.remove(this);
				}

				onClientDisconnected(this);
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				mInputStream = null;
				mOutputStream = null;

				if (istream != null) {
					try {
						istream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				if (ostream != null) {
					try {
						ostream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
	}

	public static void main(String[] args) {
		CavanTcpService service = new CavanTcpService(1234) {

			@Override
			protected boolean onDataReceived(Client client, byte[] bytes, int length) {
				String text = new String(bytes, 0, length);
				CavanJava.dLog("onDataReceived: " + text);
				return client.send(bytes, length);
			}

			@Override
			protected void onServerOpened(ServerSocket socket) {
				CavanJava.pLog();
			}

			@Override
			protected void onServerClosed() {
				CavanJava.pLog();
			}

			@Override
			protected void onClientDisconnected(Client client) {
				CavanJava.pLog();
			}

			@Override
			protected void onClientConnected(Client client) {
				CavanJava.pLog();
			}
		};

		service.open();

		while (true) {
			CavanJava.msleep(2000);
		}
	}
}
