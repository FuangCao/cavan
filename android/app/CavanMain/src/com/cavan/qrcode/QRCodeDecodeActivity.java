package com.cavan.qrcode;

import java.io.IOException;

import com.cavan.android.CavanAndroid;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.LuminanceSource;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.common.HybridBinarizer;
import com.google.zxing.qrcode.QRCodeReader;

import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;

@SuppressWarnings("deprecation")
public class QRCodeDecodeActivity extends Activity implements Callback, OnClickListener, AutoFocusCallback, PreviewCallback {

	private Camera mCamera;
	private SurfaceView mSurfaceView;
	private SurfaceHolder mSurfaceHolder;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mSurfaceView = new SurfaceView(this);
		mSurfaceView.setOnClickListener(this);

		mSurfaceHolder = mSurfaceView.getHolder();
		mSurfaceHolder.addCallback(this);

		setContentView(mSurfaceView);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		CavanAndroid.pLog();

		int index = Camera.getNumberOfCameras();

		while (--index > 0) {
			CameraInfo info = new CameraInfo();
			Camera.getCameraInfo(index, info);

			if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
				break;
			}
		}

		CavanAndroid.dLog("index = " + index);

		if (index >= 0) {
			mCamera = Camera.open();
			mCamera.setDisplayOrientation(90);

			try {
				mCamera.setPreviewDisplay(mSurfaceHolder);
				mCamera.startPreview();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		CavanAndroid.pLog();
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		CavanAndroid.pLog();

		if (mCamera != null) {
			mCamera.release();
		}
	}

	@Override
	public void onClick(View v) {
		if (mCamera != null) {
			mCamera.autoFocus(this);
		}
	}

	@Override
	public void onAutoFocus(boolean success, Camera camera) {
		CavanAndroid.pLog("success = " + success);

		if (success) {
			mCamera.setOneShotPreviewCallback(this);
		} else {
			mCamera.autoFocus(this);
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		CavanAndroid.pLog();

		Size size = camera.getParameters().getPreviewSize();
		LuminanceSource source = new PlanarYUVLuminanceSource(data, size.width, size.height, 0, 0, size.width, size.height, false);
		QRCodeReader reader = new QRCodeReader();
		BinaryBitmap image = new BinaryBitmap(new HybridBinarizer(source));
		try {
			Result result = reader.decode(image);
			CavanAndroid.showToast(getApplicationContext(), "result = " + result);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
