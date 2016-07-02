package com.cavan.java;

public class CavanByteCache {
	private byte[] mBytes;
	private int mOffset;
	private int mLength;

	public CavanByteCache(byte[] bytes, int offset, int length) {
		mBytes = bytes;
		mOffset = offset;
		mLength = mOffset + length;
	}

	public CavanByteCache(byte[] bytes, int length) {
		this(bytes, 0, length);
	}

	public CavanByteCache(byte[] bytes) {
		this(bytes, bytes.length);
	}

	public CavanByteCache(int length) {
		this(new byte[length], length);
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public int getLength() {
		return mOffset;
	}

	public void setOffset(int offset) {
		mOffset = offset;
	}

	public boolean writeValue8(byte value) {
		if (mOffset < mLength) {
			mBytes[mOffset++] = value;
			return true;
		}

		return false;
	}

	public boolean writeBytes(byte[] bytes, int offset, int count) {
		if (mOffset + count > mLength) {
			return false;
		}

		for (int end = offset + count; offset < end; offset++, mOffset++) {
			mBytes[mOffset] = bytes[offset];
		}

		return true;
	}

	public boolean writeBytes(byte[] bytes, int count) {
		return writeBytes(bytes, 0, count);
	}

	public boolean writeBytes(byte[] bytes) {
		return writeBytes(bytes, 0, bytes.length);
	}

	public boolean writeValue16(short value) {
		return writeValue8((byte) (value & 0xFF)) && writeValue8((byte) ((value >> 8) & 0xFF));
	}

	public boolean writeValue32(int value) {
		return writeValue16((short) (value & 0xFFFF)) && writeValue16((short) ((value >> 16) & 0xFFFF));
	}

	public boolean writeValueBe16(short value) {
		return writeValue8((byte) ((value >> 8) & 0xFF)) && writeValue8((byte) (value & 0xFF));
	}

	public boolean writeValueBe32(int value) {
		return writeValueBe16((short) ((value >> 16) & 0xFFFF)) && writeValueBe16((short) (value & 0xFFFF));
	}

	public byte readValue8() {
		if (mOffset < mLength) {
			return mBytes[mOffset++];
		}

		return 0;
	}

	public boolean readBytes(byte[] bytes, int offset, int count) {
		if (mOffset + count > mLength) {
			return false;
		}

		for (int last = offset + count; offset < last; offset++, mOffset++) {
			bytes[offset] = mBytes[mOffset];
		}

		return true;
	}

	public boolean readBytes(byte[] bytes, int count) {
		return readBytes(bytes, 0, count);
	}

	public boolean readBytes(byte[] bytes) {
		return readBytes(bytes, bytes.length);
	}

	public byte[] readBytes(int count) {
		byte[] bytes = new byte[count];
		if (readBytes(bytes)) {
			return bytes;
		}

		return null;
	}

	public byte[] readBytes() {
		int count = mLength - mOffset;
		if (count > 0) {
			return readBytes(count);
		}

		return null;
	}

	public short readValue16() {
		return (short) ((readValue8() & 0xFF) | ((readValue8() & 0xFF) << 8));
	}

	public int readValue32() {
		return (readValue16() & 0xFFFF) | ((readValue16() & 0xFFFF) << 16);
	}

	public short readValueBe16() {
		return (short) (((readValue8() & 0xFF) << 8) | (readValue8() & 0xFF));
	}

	public int readValueBe32() {
		return ((readValueBe16() & 0xFFFF) << 16) | (readValueBe16() & 0xFFFF);
	}
}
