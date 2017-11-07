package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class CavanTcpPacketService extends CavanTcpService {

	public CavanTcpPacketService(int port) {
		super(port);
	}

	@Override
	protected Client doCreateClient(Socket socket) {
		return new PacketClient(socket);
	}

	@Override
	public void doClientMainLoop(Client client, InputStream stream) {
		byte[] header = new byte[2];

		while (isEnabled() && CavanArray.fill(stream, header, 0, 2)) {
			int length = CavanJava.buildValueU16(header, 0);
			byte[] bytes = client.getBytes(length);

			if (length > 0) {
				if (!CavanArray.fill(stream, bytes, 0, length)) {
					break;
				}
			}

			if (!onPacketReceived(client, bytes, length)) {
				break;
			}
		}
	}

	protected boolean onPacketReceived(Client client, byte[] bytes, int length) {
		return false;
	}

	public class PacketClient extends CavanTcpService.Client {

		public PacketClient(Socket socket) {
			super(socket);
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
	}

	public static void main(String[] args) {
		CavanTcpPacketService service = new CavanTcpPacketService(1234) {

			@Override
			protected boolean onPacketReceived(Client client, byte[] bytes, int length) {
				String text = new String(bytes, 0, length);
				CavanJava.dLog("CavanTcpPacketService::onPacketReceived: " + text);
				return client.send(bytes, length);
			}
		};

		service.open();

		CavanTcpPacketClient client = new CavanTcpPacketClient() {

			@Override
			protected boolean onPacketReceived(byte[] bytes, int length) {
				String text = new String(bytes, 0, length);
				CavanJava.dLog("CavanTcpPacketClient:onPacketReceived: " + text);
				return true;
			}

			@Override
			protected boolean onTcpConnected(Socket socket) {
				CavanJava.pLog();
				return send("ABCDEF");
			}
		};

		client.connect("localhost", 1234);

		while (true) {
			CavanJava.msleep(2000);
		}
	}
}
