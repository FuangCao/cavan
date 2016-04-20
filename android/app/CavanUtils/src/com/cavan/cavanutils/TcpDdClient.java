package com.cavan.cavanutils;

import java.net.SocketAddress;

import android.annotation.SuppressLint;
import android.net.LocalSocketAddress;

@SuppressLint("DefaultLocale") public class TcpDdClient extends CavanUtils {

	public static final int TCP_DD_VERSION = 0x20151223;
	public static final int TCP_DD_HEADER_LENGTH = 12;

	public static final short TCP_DD_RESPONSE = 0;
	public static final short TCP_DD_WRITE = 1;
	public static final short TCP_DD_READ = 2;
	public static final short TCP_DD_EXEC = 3;
	public static final short TCP_ALARM_ADD = 4;
	public static final short TCP_ALARM_REMOVE = 5;
	public static final short TCP_ALARM_LIST = 6;
	public static final short TCP_KEYPAD_EVENT = 7;
	public static final short TCP_DD_MKDIR = 8;
	public static final short TCP_DD_RDDIR = 9;
	public static final short TCP_DD_FILE_STAT = 10;
	public static final short TCP_DD_BREAKPOINT = 11;

	protected CavanNetworkClient mClient;

	public TcpDdClient(SocketAddress address) {
		super();
		mClient = new CavanTcpClient(address);
	}

	public TcpDdClient(LocalSocketAddress address) {
		super();
		mClient = new CavanUnixClient(address);
	}

	public boolean sendPackage(TcpDdPackage pkg) {
		byte[] bytes = pkg.encode();
		if (bytes == null) {
			logE("sendPackage: bytes is null");
			return false;
		}

		return mClient.sendData(bytes);
	}

	public boolean recvPackage(TcpDdPackage pkg) {
		byte[] bytes = new byte[2048];
		int length = mClient.recvData(bytes);
		if (length < 0) {
			return false;
		}

		return pkg.decode(bytes, length);
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

	class TcpDdPackage {
		protected short mType;
		protected int mFlags;

		public TcpDdPackage(short type, int bodySize) {
			setType(type);
		}

		public TcpDdPackage(short type) {
			this(type, 0);
		}

		public TcpDdPackage() {
			this(TCP_DD_RESPONSE);
		}

		public short getType() {
			return mType;
		}

		public void setType(short type) {
			mType = type;
		}

		public int getFlags() {
			return mFlags;
		}

		public void setFlags(int flags) {
			mFlags = flags;
		}

		@Override
		public String toString() {
			return String.format("type = %d, flags = 0x%08x", mType, mFlags);
		}

		protected boolean encodeBody(ByteCache cache) {
			return true;
		}

		protected byte[] encode(ByteCache cache) {
			if (!cache.writeValue32(TCP_DD_VERSION)) {
				logE("Failed to writeValue32(TCP_DD_VERSION)");
				return null;
			}

			if (!cache.writeValue16(mType)) {
				logE("Failed to writeValue16(mType)");
				return null;
			}

			if (!cache.writeValue16((short) ~mType)) {
				logE("Failed to writeValue16(mType)");
				return null;
			}

			if (!cache.writeValue32(mFlags)) {
				logE("Failed to writeValue32(mFlags)");
				return null;
			}

			if (!encodeBody(cache)) {
				logE("Failed to encodeBody");
				return null;
			}

			return cache.getBytes();
		}

		protected byte[] encode(int bodySize) {
			byte[] bytes = new byte[TCP_DD_HEADER_LENGTH + bodySize];
			return encode(new ByteCache(bytes));
		}

		public byte[] encode() {
			return encode(0);
		}

		protected boolean decodeBody(ByteCache cache) {
			return true;
		}

		protected boolean decode(ByteCache cache) {
			int version = cache.readValue32();
			if (version != TCP_DD_VERSION) {
				logE(String.format("Invalid version: 0x%08x", version));
				return false;
			}

			mType = cache.readValue16();
			short type_inverse = cache.readValue16();

			if ((mType ^ type_inverse) != 0xFFFF) {
				logE(String.format("Invalid type = %d, type_inversion = %d", mType, type_inverse));
				return false;
			}

			mFlags = cache.readValue32();

			return decodeBody(cache);
		}

		public boolean decode(byte[] bytes, int offset, int length) {
			length -= TCP_DD_HEADER_LENGTH;
			if (length < 0) {
				return false;
			}

			return true;
		}

		public boolean decode(byte[] bytes, int length) {
			return decode(bytes, 0, length);
		}

		public boolean decode(byte[] bytes) {
			return decode(bytes, 0, bytes.length);
		}
	}

	class TcpDdResponse extends TcpDdPackage {
		private int mCode;
		private int mErrno;
		private String mMessage;

		public TcpDdResponse(int code, int number, String message) {
			super(TCP_DD_RESPONSE);
			mCode = code;
			mErrno = number;
			mMessage = message;
		}

		public TcpDdResponse() {
			super(TCP_DD_RESPONSE);
		}

		@Override
		public String toString() {
			return String.format("code = %d, errno = %d, message = %s", mCode, mErrno, mMessage);
		}

		public int getCode() {
			return mCode;
		}

		public int getErrno() {
			return mErrno;
		}

		public String getMessage() {
			return mMessage;
		}

		@Override
		protected boolean encodeBody(ByteCache cache) {
			if (!cache.writeValue32(mCode)) {
				logE("Failed to writeValue32(mCode)");
				return false;
			}

			if (!cache.writeValue32(mErrno)) {
				logE("Failed to writeValue32(mErrno)");
				return false;
			}

			if (mMessage != null) {
				if (!cache.writeBytes(mMessage.getBytes())) {
					logE("Failed to writeBytes(mMessage)");
					return false;
				}
			}

			return true;
		}

		@Override
		public byte[] encode() {
			if (mMessage == null) {
				encode(8);
			}

			return encode(mMessage.length() + 8);
		}

		@Override
		protected boolean decodeBody(ByteCache cache) {
			if (mType != TCP_DD_RESPONSE) {
				logE("Not a response type = " + mType);
				return false;
			}

			mCode = cache.readValue32();
			mErrno = cache.readValue32();
			byte[] message = cache.readBytes();
			if (message != null) {
				mMessage = new String(message);
			}

			return true;
		}
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
			return encode(8 + mCommand.length());
		}
	}
}
