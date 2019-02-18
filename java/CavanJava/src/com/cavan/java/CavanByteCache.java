package com.cavan.java;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class CavanByteCache {
	private byte[] mBytes;
	private int mOffset;
	private int mLength;
	private boolean mAutoExtend = true;

	public CavanByteCache(byte[] bytes, int offset, int length) {
		setBytes(bytes, offset, length);
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

	public CavanByteCache() {
		this(0);
	}

	public boolean isAutoExtend() {
		return mAutoExtend;
	}

	public void setAutoExtend(boolean enable) {
		mAutoExtend = enable;
	}

	public void setBytes(byte[] bytes, int offset, int length) {
		mBytes = bytes;
		mOffset = offset;
		mLength = mOffset + length;
	}

	public void setBytes(byte[] bytes, int length) {
		setBytes(bytes, 0, length);
	}

	public void setBytes(byte[] bytes) {
		setBytes(bytes, bytes.length);
	}

	public void setData(int length) {
		setBytes(new byte[length], length);
	}

	public byte[] getBytes() {
		if (mAutoExtend && mOffset < mLength) {
			return CavanArray.cloneByLength(mBytes, mOffset);
		} else {
			return mBytes;
		}
	}

	public int getLength() {
		return mOffset;
	}

	public void setOffset(int offset) {
		mOffset = offset;
	}

	public int getDataRemain() {
		return mLength - mOffset;
	}

	public int seek(int offset) {
		mOffset += offset;
		return mOffset;
	}

	public boolean flush(OutputStream stream) {
		try {
			stream.write(mBytes);
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean load(InputStream stream) {
		try {
			int offset = 0;
			int length = mBytes.length;

			while (true) {
				int rdlen = stream.read(mBytes, offset, length);
				if (rdlen < length) {
					if (rdlen > 0) {
						offset += rdlen;
						length -= rdlen;
					} else {
						return false;
					}
				} else {
					break;
				}
			}

			return true;
		} catch (IOException e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean writeValue8(byte value) {
		if (mOffset >= mLength) {
			if (mAutoExtend) {
				if (mLength > 0) {
					mBytes = CavanArray.cloneByLength(mBytes, mLength * 2);
				} else {
					mBytes = new byte[16];
				}

				mLength = mBytes.length;
			} else {
				return false;
			}
		}

		mBytes[mOffset++] = value;

		return true;
	}

	public boolean writeBool(boolean value) {
		return writeValue8(CavanJava.getBoolValueByte(value));
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

	public int readValue8() {
		if (mOffset < mLength) {
			return mBytes[mOffset++] & 0xFF;
		}

		return 0;
	}

	public boolean readBool() {
		return readValue8() != 0;
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

	public int readValue16() {
		return (readValue8() & 0xFF) | ((readValue8() & 0xFF) << 8);
	}

	public int readValue32() {
		return (readValue16() & 0xFFFF) | ((readValue16() & 0xFFFF) << 16);
	}

	public long readValue64() {
		return (readValue32() & 0xFFFFFFFF) | (((long) (readValue32() & 0xFFFFFFFF)) << 32);
	}

	public short readValueBe16() {
		return (short) ((readValue8() << 8) | (readValue8() & 0xFF));
	}

	public int readValueBe32() {
		return (((int) readValueBe16()) << 16) | (readValueBe16() & 0xFFFF);
	}

	public long readValueBe64() {
		return (((long) readValueBe32()) << 32) | (readValueBe32() & 0xFFFFFFFF);
	}
}
