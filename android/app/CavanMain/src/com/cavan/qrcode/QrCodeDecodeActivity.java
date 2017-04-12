package com.cavan.qrcode;

import java.io.IOException;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.view.TextureView;
import android.view.TextureView.SurfaceTextureListener;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanHandlerThread;
import com.cavan.android.CavanQrCode;
import com.cavan.cavanmain.R;
import com.google.zxing.Result;

@SuppressWarnings("deprecation")
public class QrCodeDecodeActivity extends Activity implements OnClickListener, AutoFocusCallback, PreviewCallback, SurfaceTextureListener {

	private static final int MSG_START_DECODE = 0;
	private static final int MSG_DECODE_COMPLETE = 1;

	private Camera mCamera;
	private Button mButton;
	private EditText mEditText;
	private TextureView mTextureView;

	private CavanHandlerThread mThread = new CavanHandlerThread() {

		@Override
		public boolean handleMessage(Message msg) {
			CavanAndroid.dLog("decodePlanarYUV");

			Result result = CavanQrCode.decodePlanarYUV((byte[]) msg.obj, msg.arg1, msg.arg2);
			onDecodeComplete(result);
			return true;
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_START_DECODE:
				if (mCamera != null) {
					CavanAndroid.dLog("setOneShotPreviewCallback");
					mCamera.setOneShotPreviewCallback(QrCodeDecodeActivity.this);
				}
				break;

			case MSG_DECODE_COMPLETE:
				Result result = (Result) msg.obj;
				if (result == null) {
					startDecode();
				} else {
					stopDecode(result.getText());
				}
				break;
			}
		}
	};

	public void startDecode() {
		mEditText.getText().clear();

		if (mHandler.hasMessages(MSG_START_DECODE)) {
			return;
		}

		if (mCamera != null) {
			mCamera.startPreview();
			mCamera.autoFocus(this);
			mHandler.sendEmptyMessageDelayed(MSG_START_DECODE, 1000);
		}
	}

	public void stopDecode(String text) {
		mEditText.setText(text);

		if (mCamera != null) {
			mCamera.stopPreview();
		}
	}

	public void onDecodeComplete(Result result) {
		Message message = mHandler.obtainMessage(MSG_DECODE_COMPLETE, result);
		mHandler.sendMessage(message);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.qrcode_decoder);

		mThread.start();

		mButton = (Button) findViewById(R.id.buttonQrCodeCopy);
		mButton.setOnClickListener(this);

		mEditText = (EditText) findViewById(R.id.editTextQrCode);

		mTextureView = (TextureView) findViewById(R.id.textureViewQrCode);
		mTextureView.setSurfaceTextureListener(this);
		mTextureView.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		if (v == mButton) {
			Editable text = mEditText.getText();
			if (text != null && text.length() > 0) {
				CavanAndroid.postClipboardText(this, text.toString());
			}
		} else if (mCamera != null) {
			startDecode();
		}
	}

	@Override
	public void onAutoFocus(boolean success, Camera camera) {
		CavanAndroid.pLog("success = " + success);

		if (success) {
			mHandler.sendEmptyMessage(MSG_START_DECODE);
		} else {
			mCamera.autoFocus(this);
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		CavanAndroid.pLog();

		Size size = camera.getParameters().getPreviewSize();
		Handler handler = mThread.getHandler();
		Message message = handler.obtainMessage(0, size.width, size.height, data);
		handler.sendMessage(message);
	}

	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
		CavanAndroid.pLog();

		if (mCamera == null) {
			mCamera = Camera.open();
		}

		mCamera.setDisplayOrientation(90);

		try {
			mCamera.setPreviewTexture(surface);
			startDecode();
		} catch (IOException e) {
			e.printStackTrace();
			mCamera.release();
			mCamera = null;
		}
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
		CavanAndroid.pLog();
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		CavanAndroid.pLog();

		if (mCamera != null) {
			mCamera.release();
		}

		return true;
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {}
}
