package com.cavan.java;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

public class CavanTcpPacketClient extends CavanTcpClient {

	public interface CavanTcpPacketClientListener extends CavanTcpClientListener {
		boolean onPacketReceived(byte[] bytes, int length);
	}

	private byte[] mBytes = new byte[0];
	private CavanTcpPacketClientListener mTcpPacketClientListener;

	public synchronized CavanTcpPacketClientListener getTcpPacketClientListener() {
		return mTcpPacketClientListener;
	}

	public synchronized void setTcpPacketClientListener(CavanTcpPacketClientListener listener) {
		mTcpPacketClientListener = listener;
		setTcpClientListener(listener);
	}

	@Override
	public boolean send(byte[] bytes, int offset, int length) {
		OutputStream stream = getOutputStream();

		if (stream != null) {
			synchronized (stream) {
				byte[] header = new byte[] { (byte) length, (byte) (length >> 8) };

				try {
					stream.write(header);

					if (length > 0) {
						stream.write(bytes, offset, length);
					}

					return true;
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return false;
	}

	@Override
	protected void runRecvThread(InputStream stream) {
		byte[] header = new byte[2];

		while (CavanArray.fill(stream, header, 0, 2)) {
			int length = CavanJava.buildValueU16(header, 0);

			if (length > 0) {
				if (length > mBytes.length) {
					mBytes = new byte[length];
				}

				if (!CavanArray.fill(stream, mBytes, 0, length)) {
					break;
				}
			}

			if (!onPacketReceived(mBytes, length)) {
				break;
			}
		}
	}

	protected boolean onPacketReceived(byte[] bytes, int length) {
		CavanTcpPacketClientListener listener = getTcpPacketClientListener();
		if (listener != null) {
			return listener.onPacketReceived(bytes, length);
		}

		return true;
	}

	public static void main(String[] args) {
		CavanTcpPacketClient client = new CavanTcpPacketClient() {

			@Override
			protected boolean onPacketReceived(byte[] bytes, int length) {
				String text = new String(bytes, 0, length);
				CavanJava.dLog("onPacketReceived: " + text);
				return true;
			}

			@Override
			protected boolean onTcpConnected(Socket socket) {
				CavanJava.pLog();
				return true;
			}

			@Override
			protected void onTcpDisconnected() {
				CavanJava.pLog();
			}
		};

		client.connect("127.0.0.1", 1234);

		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

		String lineBak = CavanString.EMPTY_STRING;

		while (true) {
			String line;

			try {
				line = reader.readLine().trim();
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}

			CavanJava.dLog("line = " + line);

			if (line.isEmpty()) {
				line = lineBak;
			} else {
				lineBak = line;
			}

			boolean success = client.send(line);
			CavanJava.dLog("send = " + success);
		}

		client.disconnect();
	}
}
