package com.cavan.java;

public class CavanProtoBuff {

	public static final int TYPE_BOOL = 1;
	public static final int TYPE_VALUE = 2;
	public static final int TYPE_BYTES = 3;

	private byte[] mBytes;
	private int mOffset;

	public void init(byte[] bytes) {
		mBytes = bytes;
		mOffset = 0;
	}

	public void init(int size) {
		mBytes = new byte[size];
		mOffset = 0;
	}

	public void seek(int offset) {
		mOffset = offset;
	}

	public int length() {
		return mOffset;
	}

	public int size(boolean value) {
		return 1;
	}

	public int size(long value) {
		int size = 1;

		while (value != 0) {
			value >>>= 8;
			size++;
		}

		return size;
	}

	public int size(byte[] bytes) {
		int length = bytes.length;
		int size = 1;

		if (length > 15) {
			length >>= 11;
			size++;

			while (length != 0) {
				length >>= 7;
				size++;
			}
		}

		return size + bytes.length;
	}

	public int size(String text) {
		return size(text.getBytes());
	}

	public void write(boolean value) {
		byte header = TYPE_BOOL << 5;

		if (value) {
			header |= 1;
		}

		mBytes[mOffset++] = header;
	}

	public void write(long value) {
		int offset = mOffset + 1;
		int length = 0;

		while (value != 0) {
			mBytes[offset++] = (byte) value;
			value >>>= 8;
			length++;
		}

		mBytes[mOffset] = (byte) (TYPE_VALUE << 5 | length);
		mOffset = offset;
	}

	public void write(byte[] bytes) {
		int length = bytes.length;

		if (length < 16) {
			mBytes[mOffset++] = (byte) (TYPE_BYTES << 5 | length);
		} else {
			mBytes[mOffset++] = (byte) (TYPE_BYTES << 5 | 1 << 4 | (length & 0x0F));
			length >>= 4;

			do {
				mBytes[mOffset++] = (byte) (length & 0x7F);
				length >>>= 7;
			} while (length != 0);

			length = bytes.length;
		}

		System.arraycopy(bytes, 0, mBytes, mOffset, length);
		mOffset += length;
	}

	public void write(String text) {
		write(text.getBytes());
	}

	public int getType() {
		return (mBytes[mOffset] >> 5) & 0x07;
	}

	public int getLength() {
		return mBytes[mOffset] & 0x1F;
	}

	public boolean isBool() {
		return (getType() == TYPE_BOOL);
	}

	public boolean isValue() {
		return (getType() == TYPE_VALUE);
	}

	public boolean isBytes() {
		return (getType() == TYPE_BYTES);
	}

	public int readLength() {
		int length = getLength();
		mOffset++;
		return length;
	}

	public boolean readBool() {
		if (getType() != TYPE_BOOL) {
			return false;
		}

		return (readLength() != 0);
	}

	public long readValue() {
		if (getType() != TYPE_VALUE) {
			return 0;
		}

		int length = readLength();
		int offset = mOffset + length;
		long value = 0;

		while (offset > mOffset) {
			value = value << 8 | (mBytes[--offset] & 0xFF);
		}

		mOffset += length;

		return value;
	}

	public byte[] readBytes() {
		if (getType() != TYPE_BYTES) {
			return null;
		}

		int length = readLength();
		if (length > 15) {
			int offset = 4;

			length &= 0x0F;

			while (true) {
				byte value = mBytes[mOffset++];

				length |= (value & 0x7F) << offset;

				if (value < 0) {
					offset += 7;
				} else {
					break;
				}
			}
		}

		byte[] bytes = new byte[length];

		System.arraycopy(mBytes, mOffset, bytes, 0, length);
		mOffset += length;

		return bytes;
	}

	public String readText() {
		byte[] bytes = readBytes();
		if (bytes == null) {
			return null;
		}

		return new String(bytes);
	}

	public Object read() {
		switch (getType()) {
		case TYPE_BOOL:
			return readBool();

		case TYPE_VALUE:
			return readValue();

		case TYPE_BYTES:
			return readBytes();
		}

		return null;
	}
}
