package com.cavan.android;

import java.util.Hashtable;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.EncodeHintType;
import com.google.zxing.LuminanceSource;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.HybridBinarizer;
import com.google.zxing.qrcode.QRCodeReader;
import com.google.zxing.qrcode.QRCodeWriter;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;
import com.google.zxing.qrcode.encoder.ByteMatrix;
import com.google.zxing.qrcode.encoder.Encoder;
import com.google.zxing.qrcode.encoder.QRCode;

public class CavanQrCode {

	public static Hashtable<EncodeHintType, String> getEncodeHints() {
		Hashtable<EncodeHintType, String> hints = new Hashtable<EncodeHintType, String>();
		hints.put(EncodeHintType.CHARACTER_SET, "utf-8");
		return hints;
	}

	public static Hashtable<DecodeHintType, String> getDecodeHints() {
		Hashtable<DecodeHintType, String> hints = new Hashtable<DecodeHintType, String>();
		hints.put(DecodeHintType.CHARACTER_SET, "utf-8");
		return hints;
	}

	public static Bitmap convertBitMatrixToBitmap(BitMatrix matrix, int foreColor, int backColor) {
		int width = matrix.getWidth();
		int height = matrix.getHeight();
		int[] colors = new int[width * height];

		for (int y = height - 1; y >= 0; y--) {
			int offset = y * width;

			for (int x = width - 1; x >= 0; x--) {
				colors[offset + x] = matrix.get(x, y) ? foreColor : backColor;
			}
		}

		return Bitmap.createBitmap(colors, width, height, Config.ARGB_8888);
	}

	public static Bitmap convertByteMatrixToBitmap(ByteMatrix matrix, int foreColor, int backColor) {
		int width = matrix.getWidth();
		int height = matrix.getHeight();
		int[] colors = new int[width * height];

		for (int y = height - 1; y >= 0; y--) {
			int offset = y * width;

			for (int x = width - 1; x >= 0; x--) {
				colors[offset + x] = matrix.get(x, y) == 0 ? backColor : foreColor;
			}
		}

		return Bitmap.createBitmap(colors, width, height, Config.ARGB_8888);
	}

	public static Bitmap convertByteMatrixToBitmap(ByteMatrix matrix, int width, int height, int foreColor, int backColor) {
		int widthRaw = matrix.getWidth();
		int heightRaw = matrix.getHeight();
		int zoom = Math.min(width / widthRaw, height / heightRaw);

		if (zoom < 2) {
			return convertByteMatrixToBitmap(matrix, foreColor, backColor);
		}

		width = widthRaw * zoom;
		height = heightRaw * zoom;

		int[] colors = new int[width * height];

		for (int y = heightRaw - 1; y >= 0; y--) {
			for (int x = widthRaw - 1; x >= 0; x--) {
				int color = matrix.get(x, y) == 0 ? backColor : foreColor;
				int yoffset = (y * width + x) * zoom;
				int yoffset_end = yoffset + width * zoom;

				while (yoffset < yoffset_end) {
					int xoffset_end = yoffset + zoom;

					for (int i = yoffset; i < xoffset_end; i++) {
						colors[i] = color;
					}

					yoffset += width;
				}
			}
		}

		return Bitmap.createBitmap(colors, width, height, Config.ARGB_8888);
	}

	public static QRCode encode(String text) {
		if (text == null || text.isEmpty()) {
			return null;
		}

		try {
			return Encoder.encode(text, ErrorCorrectionLevel.H, getEncodeHints());
		} catch (WriterException e) {
			e.printStackTrace();
			return null;
		}
	}

	public static BitMatrix encodeBitMatrix(String text, int width, int height) {
		if (text == null || text.isEmpty()) {
			return null;
		}

		try {
			return new QRCodeWriter().encode(text, BarcodeFormat.QR_CODE, width, height, getEncodeHints());
		} catch (WriterException e) {
			e.printStackTrace();
			return null;
		}
	}

	public static Bitmap encodeBitmap(String text, int width, int height, int foreColor, int backColor) {
		QRCode code = encode(text);
		if (code == null) {
			return null;
		}

		return convertByteMatrixToBitmap(code.getMatrix(), width, height, foreColor, backColor);
	}

	public static Bitmap encodeBitmap(String text, int width, int height, int foreColor) {
		return encodeBitmap(text, width, height, foreColor, Color.WHITE);
	}

	public static Bitmap encodeBitmap(String text, int width, int height) {
		return encodeBitmap(text, width, height, Color.BLACK);
	}

	public static Result decode(LuminanceSource source) {
		BinaryBitmap image = new BinaryBitmap(new HybridBinarizer(source));

		try {
			return new QRCodeReader().decode(image, getDecodeHints());
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static Result decodePlanarYUV(byte[] data, int width, int height) {
		LuminanceSource source = new PlanarYUVLuminanceSource(data, width, height, 0, 0, width, height, false);
		return decode(source);
	}
}
