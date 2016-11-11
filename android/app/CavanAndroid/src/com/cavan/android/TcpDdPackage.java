package com.cavan.android;

import com.cavan.java.CavanByteCache;

public class TcpDdPackage {
	protected short mType;
	protected int mFlags;

	public TcpDdPackage(short type, int bodySize) {
		setType(type);
	}

	public TcpDdPackage(short type) {
		this(type, 0);
	}

	public TcpDdPackage() {
		this(TcpDdClient.TCP_DD_RESPONSE);
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

	protected boolean encodeBody(CavanByteCache cache) {
		return true;
	}

	protected byte[] encode(CavanByteCache cache) {
		if (!cache.writeValue32(TcpDdClient.TCP_DD_VERSION)) {
			CavanAndroid.dLog("Failed to writeValue32(TCP_DD_VERSION)");
			return null;
		}

		if (!cache.writeValue16(mType)) {
			CavanAndroid.dLog("Failed to writeValue16(mType)");
			return null;
		}

		if (!cache.writeValue16((short) ~mType)) {
			CavanAndroid.dLog("Failed to writeValue16(mType)");
			return null;
		}

		if (!cache.writeValue32(mFlags)) {
			CavanAndroid.dLog("Failed to writeValue32(mFlags)");
			return null;
		}

		if (!encodeBody(cache)) {
			CavanAndroid.dLog("Failed to encodeBody");
			return null;
		}

		return cache.getBytes();
	}

	protected byte[] encode(int bodySize) {
		byte[] bytes = new byte[TcpDdClient.TCP_DD_HEADER_LENGTH + bodySize];
		return encode(new CavanByteCache(bytes));
	}

	public byte[] encode() {
		return encode(0);
	}

	protected boolean decodeBody(CavanByteCache cache) {
		return true;
	}

	protected boolean decode(CavanByteCache cache) {
		int version = cache.readValue32();
		if (version != TcpDdClient.TCP_DD_VERSION) {
			CavanAndroid.dLog(String.format("Invalid version: 0x%08x", version));
			return false;
		}

		mType = (short) cache.readValue16();
		short type_inverse = (short) cache.readValue16();

		if ((mType ^ type_inverse) != 0xFFFF) {
			CavanAndroid.dLog(String.format("Invalid type = %d, type_inversion = %d", mType, type_inverse));
			return false;
		}

		mFlags = cache.readValue32();

		return decodeBody(cache);
	}

	public boolean decode(byte[] bytes, int offset, int length) {
		length -= TcpDdClient.TCP_DD_HEADER_LENGTH;
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