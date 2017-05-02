package com.cavan.java;

import com.google.zxing.LuminanceSource;

public class CavanLuminanceSource extends LuminanceSource {

	private byte[] mBytes;

	public CavanLuminanceSource(byte[] bytes, int width, int height) {
		super(width, height);
		mBytes = bytes;
	}

	public CavanLuminanceSource(LuminanceSource source) {
		this(source.getMatrix(), source.getWidth(), source.getHeight());
	}

	public static CavanLuminanceSource createLuminanceSource(byte[] matrix, int width, int height, int rotate) {
		byte[] bytes = new byte[matrix.length];

		if (rotate >= 270) {
			for (int x = width - 1, index = 0; x >= 0; x--) {
				int end = index + height;

				for (int i = x; index < end; i += width, index++) {
					bytes[index] = matrix[i];
				}
			}

			return new CavanLuminanceSource(bytes, height, width);
		} else if (rotate >= 180) {
			for (int i = matrix.length - 1, j = 0; i >= 0; i--, j++) {
				bytes[i] = matrix[j];
			}

			return new CavanLuminanceSource(bytes, width, height);
		} else if (rotate >= 90) {
			for (int x = 0, index = 0; x < width; x++) {
				int end = index + height;

				for (int i = x, j = end - 1; j >= index; i += width, j--) {
					bytes[j] = matrix[i];
				}

				index = end;
			}

			return new CavanLuminanceSource(bytes, height, width);
		} else {
			System.arraycopy(matrix, 0, bytes, 0, bytes.length);

			return new CavanLuminanceSource(bytes, width, height);
		}
	}

	public static CavanLuminanceSource createLuminanceSource(LuminanceSource source, int rotate) {
		return createLuminanceSource(source.getMatrix(), source.getWidth(), source.getHeight(), rotate);
	}

	public byte[] getBytes() {
		return mBytes;
	}

	public int[] getColors() {
		int[] colors = new int[mBytes.length];

		for (int i = colors.length - 1; i >= 0; i--) {
			int value = mBytes[i] & 0xFF;
			colors[i] = (0xFF << 24) | (value << 16) | (value << 8) | value;
		}

		return colors;
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
