package com.cavan.android;

import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Message;

@SuppressWarnings("deprecation")
public class CavanQrCodeCamera extends CavanThreadedHandler implements AutoFocusCallback, PreviewCallback {

	private static final int AUTO_FOCUS_OVERTIME = 3000;

	private static final int MSG_OPEN_CAMERA = 1;
	private static final int MSG_START_PREVIEW = 2;
	private static final int MSG_CAPTURE_FRAME = 3;
	private static final int MSG_POST_FRAME = 4;

	public static interface EventListener {
		boolean doCameraInit(Camera camera);
		void onCameraOpened(Camera camera);
		void onStartAutoFocus(Camera camera);
		void onFrameCaptured(byte[] bytes, Camera camera);
	}

	private static CavanSingleInstanceHelper<CavanQrCodeCamera> mInstanceHelper = new CavanSingleInstanceHelper<CavanQrCodeCamera>() {

		@Override
		public CavanQrCodeCamera createInstance(Object... args) {
			return new CavanQrCodeCamera((EventListener) args[0]);
		}

		@Override
		public void initInstance(CavanQrCodeCamera instance, Object... args) {
			instance.setEventListener((EventListener) args[0]);
		}
	};

	public static CavanQrCodeCamera getInstance(EventListener listener) {
		return mInstanceHelper.getInstance(listener);
	}

	private Camera mCamera;
	private EventListener mListener;

	private CavanQrCodeCamera(EventListener listener) {
		super(CavanQrCodeCamera.class);
		setEventListener(listener);
	}

	public synchronized void setEventListener(EventListener listener) {
		mListener = listener;
	}

	public synchronized Camera getCamera() {
		return mCamera;
	}

	public synchronized Size getPreviwSize() {
		if (mCamera == null) {
			return null;
		}

		Parameters params = mCamera.getParameters();
		if (params == null) {
			return null;
		}

		return params.getPreviewSize();
	}

	public synchronized Camera openCamera() {
		if (mCamera == null) {
			try {
				mCamera = Camera.open();
				if (mCamera != null) {
					if (mListener.doCameraInit(mCamera)) {
						mCamera.setDisplayOrientation(90);
						mListener.onCameraOpened(mCamera);
					} else {
						closeCamera();
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				closeCamera();
			}
		}

		return mCamera;
	}

	public void openCameraAsync() {
		sendEmptyMessage(MSG_OPEN_CAMERA);
	}

	public synchronized void closeCamera() {
		if (mCamera != null) {
			try {
				mCamera.cancelAutoFocus();
				mCamera.stopPreview();
				mCamera.release();
			} catch (Exception e) {
				e.printStackTrace();
			}

			mCamera = null;
		}
	}

	public synchronized boolean startPreview() {
		if (openCamera() == null) {
			return false;
		}

		try {
			mCamera.startPreview();
		} catch (Exception e) {
			e.printStackTrace();
			closeCamera();
			return false;
		}

		mListener.onStartAutoFocus(mCamera);

		try {
			mCamera.autoFocus(CavanQrCodeCamera.this);
		} catch (Exception e) {
			e.printStackTrace();
		}

		sendEmptyMessageDelayed(MSG_CAPTURE_FRAME, AUTO_FOCUS_OVERTIME);

		return true;
	}

	public void startPreviewAsync() {
		sendEmptyMessage(MSG_START_PREVIEW);
	}

	public synchronized void stopPreview() {
		if (mCamera != null) {
			mCamera.stopPreview();
		}
	}

	public synchronized boolean captureFrame() {
		if (mCamera == null) {
			return false;
		}

		// CavanAndroid.dLog("setOneShotPreviewCallback");
		mCamera.setOneShotPreviewCallback(CavanQrCodeCamera.this);

		return true;
	}

	@Override
	public void handleMessage(Message msg) {
		removeMessages(msg.what);

		switch (msg.what) {
		case MSG_OPEN_CAMERA:
			openCamera();
			break;

		case MSG_START_PREVIEW:
			startPreview();
			break;

		case MSG_CAPTURE_FRAME:
			captureFrame();
			break;

		case MSG_POST_FRAME:
			if (mCamera != null) {
				mListener.onFrameCaptured((byte[]) msg.obj, mCamera);
			}
			break;
		}
	}

	@Override
	public void onAutoFocus(boolean success, Camera camera) {
		// CavanAndroid.dLog("onAutoFocus: success = " + success);

		if (mCamera != null) {
			if (success) {
				sendEmptyMessage(MSG_CAPTURE_FRAME);
			} else {
				mCamera.autoFocus(this);
			}
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		if (mCamera != null) {
			obtainMessage(MSG_POST_FRAME, data).sendToTarget();
		}
	}
}
