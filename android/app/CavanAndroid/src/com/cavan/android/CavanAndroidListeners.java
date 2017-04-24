package com.cavan.android;

import com.google.zxing.Result;

import android.hardware.Camera;
import android.widget.EditText;

@SuppressWarnings("deprecation")
public class CavanAndroidListeners {

	public interface CavanKeyboardViewListener {
		void onStartInput(EditText view);
		void onStopInput(EditText view);
		void onEditTextClick(EditText view);
		void onEditTextLongClick(EditText view);
	}

	public interface CavanBusyLockListener {
		void onBusyLockAcquired(Object owner);
		void onBusyLockReleased(Object owner);
	}

	public interface CavanQrCodeCameraListener {
		boolean doCameraInit(Camera camera);
		void onCameraOpened(Camera camera);
		void onStartAutoFocus(Camera camera);
		void onFrameCaptured(byte[] bytes, Camera camera);
	}

	public interface CavanQrCodeViewListener {
		void onDecodeStart();
		boolean onDecodeComplete(Result result);
		boolean doCameraInit(Camera camera);
		void onCameraOpened(Camera camera);
	}
}