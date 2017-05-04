package com.cavan.qrcode;

import java.io.IOException;

import android.Manifest;
import android.app.Activity;
import android.hardware.Camera;
import android.os.Bundle;
import android.text.Editable;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanAndroidListeners.CavanQrCodeViewListener;
import com.cavan.android.CavanQrCodeCamera;
import com.cavan.android.CavanQrCodeView;
import com.cavan.cavanmain.R;
import com.google.zxing.Result;

@SuppressWarnings("deprecation")
public class QrCodeDecodeActivity extends Activity implements OnClickListener, Callback, CavanQrCodeViewListener {

	private Button mButton;
	private EditText mEditText;
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private CavanQrCodeView mQrCodeView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.qrcode_decoder);

		mQrCodeView = (CavanQrCodeView) findViewById(R.id.qrCodeView);
		mQrCodeView.setOnClickListener(this);
		mQrCodeView.setEventListener(this);

		mButton = (Button) findViewById(R.id.buttonQrCodeCopy);
		mButton.setOnClickListener(this);

		mEditText = (EditText) findViewById(R.id.editTextQrCode);

		mSurface = (SurfaceView) findViewById(R.id.surfaceViewQrCode);
		mHolder = mSurface.getHolder();
		mHolder.addCallback(this);
	}

	@Override
	public void onClick(View v) {
		if (v == mButton) {
			Editable text = mEditText.getText();
			if (text != null && text.length() > 0) {
				CavanAndroid.postClipboardText(this, text.toString());
			}
		} else {
			mQrCodeView.startPreview();
		}
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		CavanAndroid.pLog("requestCode = " + requestCode);

		if (CavanAndroid.checkPermissions(permissions, grantResults, Manifest.permission.CAMERA)) {
			mQrCodeView.openCamera(mSurface.getWidth(), mSurface.getHeight());
		} else {
			CavanAndroid.showToast(this, R.string.request_camera_permission_failed);
		}
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		CavanAndroid.pLog();

		if (CavanQrCodeCamera.checkAndRequestPermissions(this)) {
			mQrCodeView.openCamera(mSurface.getWidth(), mSurface.getHeight());
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		CavanAndroid.pLog();
		mQrCodeView.setSurfaceSize(width, height);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		CavanAndroid.pLog();
		mQrCodeView.closeCamera();
	}

	@Override
	public boolean onDecodeComplete(Result result) {
		mEditText.setText(result.getText());
		return true;
	}

	@Override
	public void onDecodeStart() {
		mEditText.getText().clear();
	}

	@Override
	public boolean doCameraInit(Camera camera) {
		try {
			camera.setPreviewDisplay(mHolder);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	@Override
	public void onCameraOpened(Camera camera) {
		if (camera != null) {
			mQrCodeView.startPreview();
		} else {
			CavanAndroid.showToast(this, R.string.open_camera_failed);
		}
	}
}
