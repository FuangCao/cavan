package com.cavan.java;

public class ByteCache {
	private byte[] mBytes;
	private int mOffset;
	private int mLength;

	public ByteCache(byte[] bytes, int offset, int length) {
		mBytes = bytes;
		mOffset = offset;
		mLength = mOffset + length;
	}

	public ByteCache(byte[] bytes, int length) {
		this(bytes, 0, length);
	}

	public ByteCache(byte[] bytes) {
		this(bytes, bytes.length);
	}

	public ByteCache(int length) {
		this(new byte[length], length);
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public int getLength() {
		return mOffset;
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
		return (short) (readValue8() | (((short) readValue8()) << 8));
	}

	public int readValue32() {
		return readValue16() | (((int) readValue16()) << 16);
	}
}
