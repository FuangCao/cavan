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

	private static final int AUTO_FOCUS_OVERTIME = 3000;

	private static final int MSG_AUTO_FOCUS_COMPLETE = 1;
	private static final int MSG_DECODE_COMPLETE = 2;

	public interface DecodeEventListener {
		void onDecodeStart();
		boolean onDecodeComplete(Result result);
	}

	private double mBorderRatio = 0.1;
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
			case MSG_AUTO_FOCUS_COMPLETE:
				CavanAndroid.dLog("MSG_AUTO_FOCUS_COMPLETE");

				if (mCamera != null) {
					CavanAndroid.dLog("setOneShotPreviewCallback");
					mCamera.setOneShotPreviewCallback(CavanQrCodeView.this);
				}
				break;

			case MSG_DECODE_COMPLETE:
				CavanAndroid.dLog("MSG_DECODE_COMPLETE");

				Result result = (Result) msg.obj;
				if (result != null && onDecodeComplete(result)) {
					if (mCamera != null) {
						mCamera.stopPreview();
					}

					if (mListener != null) {
						mListener.onDecodeComplete(result);
					}
				} else {
					startPreview();
				}
				break;
			}
		}
	};

	private CavanHandlerThread mThread = new CavanHandlerThread() {

		@Override
		public boolean handleMessage(Message msg) {
			Result result;

			try {
				PlanarYUVLuminanceSource sourceRaw = new PlanarYUVLuminanceSource((byte[]) msg.obj, mVideoWidth, mVideoHeight, mQrCodeX, mQrCodeY, mQrCodeWidth, mQrCodeHeight, false);
				CavanLuminanceSourceRotate90 source = new CavanLuminanceSourceRotate90(sourceRaw);
				result = CavanQrCode.decode(mQrCodeReader, source);
			} catch (Exception e) {
				e.printStackTrace();
				result = null;
			}

			Message message = mHandler.obtainMessage(MSG_DECODE_COMPLETE, result);
			mHandler.sendMessage(message);
			return true;
		}
	};

	public boolean onDecodeComplete(Result result) {
		if (mListener != null) {
			return mListener.onDecodeComplete(result);
		}

		return false;
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
		CavanAndroid.dLog("closeCamera");

		if (mCamera != null) {
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}

	public void startPreview() {
		CavanAndroid.dLog("startPreview");

		if (mListener != null) {
			mListener.onDecodeStart();
		}

		if (mCamera != null) {
			mCamera.startPreview();
			mCamera.autoFocus(this);
			mHandler.sendEmptyMessageDelayed(MSG_AUTO_FOCUS_COMPLETE, AUTO_FOCUS_OVERTIME);
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

	public double getBorderRatio() {
		return mBorderRatio;
	}

	public void setBoarderRatio(double ratio) {
		if (ratio < 1) {
			mBorderRatio = ratio;
			postInvalidate();
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		int width = canvas.getWidth();
		int height = canvas.getHeight();

		CavanAndroid.dLog("width = " + width);
		CavanAndroid.dLog("height = " + height);

		int left, right, top, bottom;

		if (height < width) {
			top = (int) (height * mBorderRatio);
			bottom = height - top;
			mWinWidth = bottom - top + 1;

			left = (width - mWinWidth) / 2;
			right = left + mWinWidth - 1;
		} else {
			left = top = (int) (width * mBorderRatio);
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
				mHandler.sendEmptyMessage(MSG_AUTO_FOCUS_COMPLETE);
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
