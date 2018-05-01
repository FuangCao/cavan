package com.cavan.android;

import com.cavan.java.CavanByteCache;


class TcpDdExecReq extends TcpDdPackage {

	private short mLines;
	private short mColumns;
	private byte[] mCommand;

	public TcpDdExecReq(int lines, int columns, String command) {
		super(TcpDdClient.TCP_DD_EXEC);

		mLines = (short) lines;
		mColumns = (short) columns;
		mCommand = command.getBytes();
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
	protected boolean encodeBody(CavanByteCache cache) {
		if (!cache.writeValue16(mLines)) {
			CavanAndroid.dLog("Failed to writeValue16(mLines)");
			return false;
		}

		if (!cache.writeValue16(mColumns)) {
			CavanAndroid.dLog("Failed to writeValue16(mColumns)");
			return false;
		}

		if (!cache.writeBytes(mCommand)) {
			CavanAndroid.dLog("Failed to writeBytes(mCommand)");
			return false;
		}

		if (!cache.writeValue8((byte) 0)) {
			CavanAndroid.dLog("Failed to writeValue8(0)");
			return false;
		}

		return true;
	}

	@Override
	public byte[] encode() {
		return encode(8 + mCommand.length + 1);
	}
}