package com.cavan.cavanutils;

import android.annotation.SuppressLint;

@SuppressLint("DefaultLocale")
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

	protected boolean encodeBody(ByteCache cache) {
		return true;
	}

	protected byte[] encode(ByteCache cache) {
		if (!cache.writeValue32(TcpDdClient.TCP_DD_VERSION)) {
			CavanUtils.logE("Failed to writeValue32(TCP_DD_VERSION)");
			return null;
		}

		if (!cache.writeValue16(mType)) {
			CavanUtils.logE("Failed to writeValue16(mType)");
			return null;
		}

		if (!cache.writeValue16((short) ~mType)) {
			CavanUtils.logE("Failed to writeValue16(mType)");
			return null;
		}

		if (!cache.writeValue32(mFlags)) {
			CavanUtils.logE("Failed to writeValue32(mFlags)");
			return null;
		}

		if (!encodeBody(cache)) {
			CavanUtils.logE("Failed to encodeBody");
			return null;
		}

		return cache.getBytes();
	}

	protected byte[] encode(int bodySize) {
		byte[] bytes = new byte[TcpDdClient.TCP_DD_HEADER_LENGTH + bodySize];
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
		if (version != TcpDdClient.TCP_DD_VERSION) {
			CavanUtils.logE(String.format("Invalid version: 0x%08x", version));
			return false;
		}

		mType = cache.readValue16();
		short type_inverse = cache.readValue16();

		if ((mType ^ type_inverse) != 0xFFFF) {
			CavanUtils.logE(String.format("Invalid type = %d, type_inversion = %d", mType, type_inverse));
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