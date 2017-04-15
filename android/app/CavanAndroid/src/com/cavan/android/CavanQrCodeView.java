package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;

import com.cavan.java.CavanLuminanceSourceRotate90;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.qrcode.QRCodeReader;

@SuppressWarnings("deprecation")
public class CavanQrCodeView extends View implements PreviewCallback, AutoFocusCallback {

	private static final int MSG_AUTO_FOCUS_TIMEOUT = 1;
	private static final int MSG_DECODE_COMPLETE = 2;

	public interface DecodeEventListener {
		void onDecodeStart();
		void onDecodeComplete(Result result);
	}

	private Paint mPaint = new Paint();

	private int mWinX;
	private int mWinY;
	private int mWinWidth;

	private int mVideoWidth;
	private int mVideoHeight;

	private int mSurfaceWidth;
	private int mSurfaceHeight;

	private int mQrCodeX;
	private int mQrCodeY;
	private int mQrCodeWidth;
	private int mQrCodeHeight;

	private QRCodeReader mQrCodeReader = new QRCodeReader();

	private Camera mCamera;
	private DecodeEventListener mListener;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_AUTO_FOCUS_TIMEOUT:
				if (mCamera != null) {
					CavanAndroid.dLog("setOneShotPreviewCallback");
					mCamera.setOneShotPreviewCallback(CavanQrCodeView.this);
				}
				break;

			case MSG_DECODE_COMPLETE:
				Result result = (Result) msg.obj;
				if (result == null) {
					startPreview();
				} else {
					if (mCamera != null) {
						mCamera.stopPreview();
					}

					if (mListener != null) {
						mListener.onDecodeComplete(result);
					}
				}
				break;
			}
		}
	};

	private CavanHandlerThread mThread = new CavanHandlerThread() {

		@Override
		public boolean handleMessage(Message msg) {
			if (mListener != null) {
				Result result;

				try {
					PlanarYUVLuminanceSource sourceRaw = new PlanarYUVLuminanceSource((byte[]) msg.obj, mVideoWidth, mVideoHeight, mQrCodeX, mQrCodeY, mQrCodeWidth, mQrCodeHeight, false);
					CavanLuminanceSourceRotate90 source = new CavanLuminanceSourceRotate90(sourceRaw);
					result = CavanQrCode.decode(mQrCodeReader, source);
				} catch (Exception e) {
					e.printStackTrace();
					result = null;
				}

				onDecodeComplete(result);
			}

			return true;
		}
	};

	public void onDecodeComplete(Result result) {
		Message message = mHandler.obtainMessage(MSG_DECODE_COMPLETE, result);
		mHandler.sendMessage(message);
	}

	public CavanQrCodeView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		init();
	}

	public CavanQrCodeView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		init();
	}

	public CavanQrCodeView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	public CavanQrCodeView(Context context) {
		super(context);
		init();
	}

	private void init() {
		mPaint.setARGB(0x00, 0x00, 0x00, 0x00);
		mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
	}

	public void setDecodeEventListener(DecodeEventListener listener) {
		mListener = listener;
	}

	public DecodeEventListener getDecodeEventListener() {
		return mListener;
	}

	public synchronized Camera openCamera(int width, int height) {
		if (mCamera == null) {
			mCamera = Camera.open();
			if (mCamera == null) {
				return null;
			}
		}

		mCamera.setDisplayOrientation(90);

		Size size = mCamera.getParameters().getPreviewSize();
		mVideoWidth = size.width;
		mVideoHeight = size.height;

		mSurfaceWidth = width;
		mSurfaceHeight = height;

		updateQrCodeRange();

		return mCamera;
	}

	public synchronized void closeCamera() {
		if (mCamera != null) {
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}

	public void startPreview() {
		if (mListener != null) {
			mListener.onDecodeStart();
		}

		if (mCamera != null) {
			mCamera.startPreview();
			mCamera.autoFocus(this);
			mHandler.sendEmptyMessageDelayed(MSG_AUTO_FOCUS_TIMEOUT, 1000);
		}
	}

	public void updateQrCodeRange() {
		CavanAndroid.dLog("mWinX = " + mWinX);
		CavanAndroid.dLog("mWinY = " + mWinY);
		CavanAndroid.dLog("mWinWidth = " + mWinWidth);
		CavanAndroid.dLog("mVideoWidth = " + mVideoWidth);
		CavanAndroid.dLog("mVideoHeight = " + mVideoHeight);
		CavanAndroid.dLog("mSurfaceWidth = " + mSurfaceWidth);
		CavanAndroid.dLog("mSurfaceHeight = " + mSurfaceHeight);

		if (mSurfaceHeight > 0) {
			mQrCodeX = mWinY * mVideoWidth / mSurfaceHeight;
			mQrCodeWidth = mWinWidth * mVideoWidth / mSurfaceHeight;
		}

		if (mSurfaceWidth > 0) {
			mQrCodeY = mWinX * mVideoHeight / mSurfaceWidth;
			mQrCodeHeight= mWinWidth * mVideoHeight / mSurfaceWidth;
		}

		CavanAndroid.pLog("mQrCodeX = " + mQrCodeX);
		CavanAndroid.pLog("mQrCodeY = " + mQrCodeY);
		CavanAndroid.pLog("mQrCodeWidth = " + mQrCodeWidth);
		CavanAndroid.pLog("mQrCodeHeight = " + mQrCodeHeight);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		int width = canvas.getWidth();
		int height = canvas.getHeight();

		CavanAndroid.dLog("width = " + width);
		CavanAndroid.dLog("height = " + height);

		int left, right, top, bottom;

		if (height < width) {
			top = height / 5;
			bottom = height - top;
			mWinWidth = bottom - top + 1;

			left = (width - mWinWidth) / 2;
			right = left + mWinWidth - 1;
		} else {
			left = top = width / 5;
			bottom = right = width - left;
			mWinWidth = right - left + 1;
		}

		mWinX = (int) (getX() + left);
		mWinY = (int) (getY() + top);

		updateQrCodeRange();

		canvas.drawRect(left, top, right, bottom, mPaint);
	}

	@Override
	public void onAutoFocus(boolean success, Camera camera) {
		CavanAndroid.pLog("success = " + success);

		if (mCamera != null) {
			if (success) {
				mHandler.removeMessages(MSG_AUTO_FOCUS_TIMEOUT);
				mCamera.setOneShotPreviewCallback(this);
			} else {
				mCamera.autoFocus(this);
			}
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		CavanAndroid.pLog();

		if (mCamera != null) {
			Size size = mCamera.getParameters().getPreviewSize();

			Handler handler = mThread.getHandler();
			Message message = handler.obtainMessage(0, size.width, size.height, data);
			handler.sendMessage(message);
		}
	}
}
