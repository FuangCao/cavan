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

	public static BitMatrix encode(String text, int width, int height) {
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
		BitMatrix matrix = encode(text, width, height);
		if (matrix == null) {
			return null;
		}

		return convertBitMatrixToBitmap(matrix, foreColor, backColor);
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
