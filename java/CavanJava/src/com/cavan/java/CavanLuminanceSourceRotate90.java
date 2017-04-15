package com.cavan.java;

import com.google.zxing.LuminanceSource;

public class CavanLuminanceSourceRotate90 extends LuminanceSource {

	private byte[] mBytes;

	public CavanLuminanceSourceRotate90(LuminanceSource source) {
		super(source.getHeight(), source.getWidth());

		int width = source.getWidth();
		int height = source.getHeight();
		byte[] matrix = source.getMatrix();
		byte[] bytes = new byte[matrix.length];

		for (int x = 0, index = 0; x < width; x++) {
			int end = index + height;

			for (int i = end - 1, offset = x; i >= index; i--, offset += width) {
				bytes[i] = matrix[offset];
			}

			index = end;
		}

		mBytes = bytes;
	}

	@Override
	public byte[] getRow(int y, byte[] row) {
		int width = getWidth();
		byte[] bytes = new byte[width];
		System.arraycopy(mBytes, y * width, bytes, 0, width);
		return bytes;
	}

	@Override
	public byte[] getMatrix() {
		return mBytes;
	}
}
