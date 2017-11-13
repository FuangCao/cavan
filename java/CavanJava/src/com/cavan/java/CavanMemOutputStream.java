package com.cavan.java;

import java.io.IOException;
import java.io.OutputStream;

public class CavanMemOutputStream extends OutputStream {

	private int mLength;
	private byte[] mBytes;

	public CavanMemOutputStream(int length) {
		mBytes = new byte[length];
	}

	public CavanMemOutputStream() {
		this(32);
	}

	public synchronized byte[] getBytes() {
		return mBytes;
	}

	public synchronized int getLength() {
		return mLength;
	}

	public synchronized byte[] expand(int length) {
		if (mLength + length < mBytes.length) {
			return mBytes;
		}

		byte[] bytes = new byte[(mLength + length) << 1];
		System.arraycopy(mBytes, 0, bytes, 0, mLength);
		mBytes = bytes;

		return bytes;
	}

	public synchronized void clear() {
		mLength = 0;
	}

	@Override
	public synchronized void write(byte[] b, int off, int len) throws IOException {
		byte[] bytes = expand(len);
		System.arraycopy(b, off, bytes, mLength, len);
		mLength += len;
	}

	@Override
	public synchronized void write(int b) throws IOException {
		byte[] bytes = expand(1);
		bytes[mLength++] = (byte) b;
	}

	public void write(byte[] bytes) throws IOException {
		write(bytes, 0, bytes.length);
	}

	public void write(String text) throws IOException {
		write(text.getBytes());
	}

	@Override
	public String toString() {
		return new String(mBytes, 0, mLength);
	}
}
