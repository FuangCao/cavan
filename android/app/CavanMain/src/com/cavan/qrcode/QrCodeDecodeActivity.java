package com.cavan.qrcode;

import java.io.IOException;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.Editable;
import android.view.Menu;
import android.view.MenuItem;
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
	private boolean mPreviewEnabled = true;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.qrcode_decoder);

		mQrCodeView = (CavanQrCodeView) findViewById(R.id.qrCodeView);
		mQrCodeView.setEventListener(this);

		mButton = (Button) findViewById(R.id.buttonQrCodeCopy);
		mButton.setOnClickListener(this);

		mEditText = (EditText) findViewById(R.id.editTextQrCode);

		mSurface = (SurfaceView) findViewById(R.id.surfaceViewQrCode);
		mHolder = mSurface.getHolder();
		mHolder.addCallback(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.qrcode_decoder, menu);
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_message_open:
			mPreviewEnabled = false;
			mQrCodeView.stopPreview();
			mEditText.getEditableText().clear();
			Intent intent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
			startActivityForResult(intent, 0);
			break;

		case R.id.action_message_scan:
			mPreviewEnabled = true;
			mQrCodeView.startPreview();
			break;
		}

		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onClick(View v) {
		if (v == mButton) {
			Editable text = mEditText.getText();
			if (text != null && text.length() > 0) {
				CavanAndroid.postClipboardText(this, text.toString());
			}
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.dLog("onActivityResult: requestCode = " + requestCode + ", resultCode = " + requestCode + ", data = " + data);

		try {
			Bitmap bitmap = MediaStore.Images.Media.getBitmap(getContentResolver(), data.getData());
			mQrCodeView.setBitmap(bitmap);
		} catch (Exception e) {
			e.printStackTrace();
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
		if (mPreviewEnabled) {
			if (camera != null) {
				mQrCodeView.startPreview();
			} else {
				CavanAndroid.showToast(this, R.string.open_camera_failed);
			}
		}
	}
}
