package com.cavan.qrcode;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.EditText;
import android.widget.ImageView;

import com.cavan.cavanmain.R;
import com.google.zxing.BarcodeFormat;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

public class QRCodeEncodeActivity extends Activity implements TextWatcher {

	private EditText mEditTextQRCode;
	private ImageView mImageViewQRCode;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			if (mHandler.hasMessages(msg.what)) {
				return;
			}

			String text = mEditTextQRCode.getText().toString();
			if (text.isEmpty()) {
				return;
			}

			try {
				BitMatrix matrix = new QRCodeWriter().encode(mEditTextQRCode.getText().toString(), BarcodeFormat.QR_CODE, mImageViewQRCode.getWidth(), mImageViewQRCode.getHeight());

				int width = matrix.getWidth();
				int height = matrix.getHeight();
				int[] pixels = new int[width * height];

				for (int y = height - 1; y >= 0; y--) {
					int offset = y * width;

					for (int x = width - 1; x >= 0; x--) {
						pixels[offset + x] = matrix.get(x, y) ? Color.BLACK : Color.WHITE;
					}
				}

				Bitmap bitmap = Bitmap.createBitmap(pixels, width, height, Config.ARGB_8888);
				mImageViewQRCode.setImageBitmap(bitmap);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.qrcode_encoder);

		mEditTextQRCode = (EditText) findViewById(R.id.editTextQRCode);
		mEditTextQRCode.addTextChangedListener(this);

		mImageViewQRCode = (ImageView) findViewById(R.id.imageViewQRCode);
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {}

	@Override
	public void afterTextChanged(Editable s) {
		mImageViewQRCode.setImageResource(R.drawable.ic_launcher);
		mHandler.sendEmptyMessageDelayed(0, 500);
	}
}
