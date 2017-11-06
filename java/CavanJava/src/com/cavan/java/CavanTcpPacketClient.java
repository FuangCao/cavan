package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class CavanTcpPacketClient extends CavanTcpClient {

	private byte[] mBytes = new byte[0];

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
	protected void mainLoop(InputStream stream) {
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
		return false;
	}
}
