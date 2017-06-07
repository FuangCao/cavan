package com.cavan.android;

import android.Manifest;
import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Message;

import com.cavan.android.CavanAndroidListeners.CavanQrCodeCameraListener;

@SuppressWarnings("deprecation")
public class CavanQrCodeCamera extends CavanThreadedHandler implements AutoFocusCallback, PreviewCallback {

	public static final String[] PERMISSIONS = {
		Manifest.permission.CAMERA,
	};

	private static final int MAX_CAPTURE_TIMES = 16;
	private static final int AUTO_FOCUS_OVERTIME = 3000;

	private static final int MSG_OPEN_CAMERA = 1;
	private static final int MSG_START_PREVIEW = 2;
	private static final int MSG_CAPTURE_FRAME = 3;
	private static final int MSG_POST_FRAME = 4;

	public static boolean checkAndRequestPermissions(Activity activity, int requestCode) {
		return CavanAndroid.checkAndRequestPermissions(activity, requestCode, PERMISSIONS);
	}

	public static boolean checkAndRequestPermissions(Activity activity) {
		return checkAndRequestPermissions(activity, 0);
	}

	private static CavanSingleInstanceHelper<CavanQrCodeCamera> mInstanceHelper = new CavanSingleInstanceHelper<CavanQrCodeCamera>() {

		@Override
		public CavanQrCodeCamera createInstance(Object... args) {
			return new CavanQrCodeCamera((CavanQrCodeCameraListener) args[0]);
		}

		@Override
		public void initInstance(CavanQrCodeCamera instance, Object... args) {
			instance.setEventListener((CavanQrCodeCameraListener) args[0]);
		}
	};

	public static CavanQrCodeCamera getInstance(CavanQrCodeCameraListener listener) {
		return mInstanceHelper.getInstance(listener);
	}

	private Camera mCamera;
	private int mCaptureTimes;
	private CavanQrCodeCameraListener mListener;

	private CavanQrCodeCamera(CavanQrCodeCameraListener listener) {
		super(CavanQrCodeCamera.class);
		setEventListener(listener);
	}

	synchronized public void setEventListener(CavanQrCodeCameraListener listener) {
		if (listener == null) {
			throw new IllegalArgumentException("CavanQrCodeCameraListener is null");
		}

		mListener = listener;
	}

	synchronized public Camera getCamera() {
		return mCamera;
	}

	synchronized boolean isCameraOpened() {
		return (mCamera != null);
	}

	synchronized public Size getPreviwSize() {
		if (mCamera == null) {
			return null;
		}

		Parameters params = mCamera.getParameters();
		if (params == null) {
			return null;
		}

		return params.getPreviewSize();
	}

	synchronized public Camera openCamera() {
		if (mCamera == null) {
			mCaptureTimes = 0;

			try {
				mCamera = Camera.open();
				if (mCamera != null) {
					if (mListener.doCameraInit(mCamera)) {
						mCamera.setDisplayOrientation(90);
					} else {
						closeCamera();
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				closeCamera();
			}

			mListener.onCameraOpened(mCamera);
		}

		return mCamera;
	}

	public void openCameraAsync() {
		sendEmptyMessage(MSG_OPEN_CAMERA);
	}

	synchronized public void closeCamera() {
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

	synchronized public boolean startPreview() {
		if (mCamera == null) {
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

		// CavanAndroid.dLog("mCaptureTimes = " + mCaptureTimes);

		if (mCaptureTimes > 0) {
			mCaptureTimes--;
			sendEmptyMessage(MSG_CAPTURE_FRAME);
		} else {
			try {
				mCamera.autoFocus(CavanQrCodeCamera.this);
			} catch (Exception e) {
				e.printStackTrace();
			}

			sendEmptyMessageDelayed(MSG_CAPTURE_FRAME, AUTO_FOCUS_OVERTIME);
		}

		return true;
	}

	public void startPreviewAsync() {
		sendEmptyMessage(MSG_START_PREVIEW);
	}

	synchronized public void stopPreview() {
		if (mCamera != null) {
			mCamera.stopPreview();
		}
	}

	synchronized public boolean captureFrame() {
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
				mCaptureTimes = MAX_CAPTURE_TIMES;
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
