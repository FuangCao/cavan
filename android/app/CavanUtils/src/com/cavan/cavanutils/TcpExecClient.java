package com.cavan.cavanutils;

import java.net.InetAddress;
import android.annotation.SuppressLint;
import android.net.LocalSocketAddress;

@SuppressLint("DefaultLocale") public class TcpExecClient extends TcpDdClient {

	public TcpExecClient(InetAddress address, int port) {
		super(address, port);
	}

	public TcpExecClient(LocalSocketAddress address) {
		super(address);
	}

	public boolean runCommand(String command) {
		if (!mClient.connect()) {
			return false;
		}

		TcpDdExecReq req = new TcpDdExecReq(command);
		if (!sendPackage(req)) {
			return false;
		}

		TcpDdResponse response = new TcpDdResponse();
		if (!recvPackage(response)) {
			return false;
		}

		if (response.getCode() < 0) {
			return false;
		}

		while (true) {
			byte[] data = new byte[1024];
			int length = mClient.recvData(data);
			if (length <= 0) {
				break;
			}

			logD(new String(data, 0, length));
		}

		mClient.disconnect();

		return true;
	}

	class TcpDdExecReq extends TcpDdPackage {
		private short mLines;
		private short mColumns;
		private String mCommand;

		public TcpDdExecReq(int lines, int columns, String command) {
			super(TCP_DD_EXEC);

			mLines = (short) lines;
			mColumns = (short) columns;
			mCommand = command;
		}

		public TcpDdExecReq(String command) {
			this((short) -1, (short) -1, command);
		}

		public TcpDdExecReq() {
			this("shell");
		}

		@Override
		public String toString() {
			return String.format("lines = %d, columns = %d, command = %s", mLines, mColumns, mCommand);
		}

		@Override
		protected boolean encodeBody(ByteCache cache) {
			if (!cache.writeValue16(mLines)) {
				logE("Failed to writeValue16(mLines)");
				return false;
			}

			if (!cache.writeValue16(mColumns)) {
				logE("Failed to writeValue16(mColumns)");
				return false;
			}

			if (!cache.writeBytes(mCommand.getBytes())) {
				logE("Failed to writeBytes(mCommand)");
				return false;
			}

			return true;
		}

		@Override
		public byte[] encode() {
			return encode(8 + mCommand.getBytes().length);
		}
	}
}