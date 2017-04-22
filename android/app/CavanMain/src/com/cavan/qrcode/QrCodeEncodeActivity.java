package com.cavan.qrcode;

import java.io.OutputStream;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanQrCode;
import com.cavan.cavanmain.R;
import com.cavan.java.CavanFile;

public class QrCodeEncodeActivity extends Activity implements TextWatcher, OnClickListener {

	private Bitmap mBitmap;
	private Button mButtonSave;
	private EditText mEditTextQrCode;
	private ImageView mImageViewQrCode;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			if (mHandler.hasMessages(msg.what)) {
				return;
			}

			mBitmap = CavanQrCode.encodeBitmap(mEditTextQrCode.getText().toString(), mImageViewQrCode.getWidth(), mImageViewQrCode.getHeight());
			if (mBitmap != null) {
				mImageViewQrCode.setImageBitmap(mBitmap);
				mImageViewQrCode.setVisibility(View.VISIBLE);
				mButtonSave.setEnabled(true);
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.qrcode_encoder);

		mButtonSave = (Button) findViewById(R.id.buttonQrCodeSave);
		mButtonSave.setOnClickListener(this);

		mEditTextQrCode = (EditText) findViewById(R.id.editTextQrCode);
		mEditTextQrCode.addTextChangedListener(this);

		mImageViewQrCode = (ImageView) findViewById(R.id.imageViewQrCode);
		mImageViewQrCode.setOnClickListener(this);
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {}

	@Override
	public void afterTextChanged(Editable s) {
		mButtonSave.setEnabled(false);
		mImageViewQrCode.setVisibility(View.INVISIBLE);
		mHandler.sendEmptyMessageDelayed(0, 500);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonSave) {
			if (mBitmap != null) {
				CavanFile file = new CavanFile(Environment.getExternalStorageDirectory(), "CavanQrCode.png");
				OutputStream stream = file.openOutputStream();
				if (stream != null) {
					mBitmap.compress(CompressFormat.PNG, 100, stream);
					CavanFile.close(stream);
					CavanAndroid.showToast(this, R.string.saved, file.getPath());
				}
			}
		} else {
			CavanAndroid.setSoftInputEnable(this, mEditTextQrCode, false);
		}
	}
}
