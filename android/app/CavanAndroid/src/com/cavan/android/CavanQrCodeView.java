package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.hardware.Camera;
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
public class CavanQrCodeView extends View implements CavanQrCodeCamera.EventListener {

	private static final int MSG_DECODE_START = 1;
	private static final int MSG_DECODE_COMPLETE = 2;
	private static final int MSG_QRCODE_MATRIX = 3;
	private static final int MSG_CAMERA_OPENED = 4;

	public interface EventListener {
		void onDecodeStart();
		boolean onDecodeComplete(Result result);
		boolean doCameraInit(Camera camera);
		void onCameraOpened(Camera camera);
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

	private EventListener mListener;
	private QRCodeReader mQrCodeReader = new QRCodeReader();
	private CavanQrCodeCamera mCameraHandler = CavanQrCodeCamera.getInstance(this);

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_DECODE_START:
				if (mListener != null) {
					mListener.onDecodeStart();
				}
				break;

			case MSG_DECODE_COMPLETE:
				Result result = (Result) msg.obj;
				if (result != null && mListener != null && mListener.onDecodeComplete(result)) {
					stopPreview();
				} else {
					startPreview();
				}
				break;

			case MSG_QRCODE_MATRIX:
				Size size = mCameraHandler.getPreviwSize();
				if (size != null) {
					mVideoWidth = size.width;
					mVideoHeight = size.height;
				} else {
					mVideoWidth = mSurfaceWidth;
					mVideoHeight = mSurfaceHeight;
				}

				/* CavanAndroid.dLog("mWinX = " + mWinX);
				CavanAndroid.dLog("mWinY = " + mWinY);
				CavanAndroid.dLog("mWinWidth = " + mWinWidth);
				CavanAndroid.dLog("mVideoWidth = " + mVideoWidth);
				CavanAndroid.dLog("mVideoHeight = " + mVideoHeight);
				CavanAndroid.dLog("mSurfaceWidth = " + mSurfaceWidth);
				CavanAndroid.dLog("mSurfaceHeight = " + mSurfaceHeight); */

				if (mSurfaceHeight > 0) {
					mQrCodeX = mWinY * mVideoWidth / mSurfaceHeight;
					mQrCodeWidth = mWinWidth * mVideoWidth / mSurfaceHeight;
				}

				if (mSurfaceWidth > 0) {
					mQrCodeY = mWinX * mVideoHeight / mSurfaceWidth;
					mQrCodeHeight= mWinWidth * mVideoHeight / mSurfaceWidth;
				}

				/* CavanAndroid.dLog("mQrCodeX = " + mQrCodeX);
				CavanAndroid.dLog("mQrCodeY = " + mQrCodeY);
				CavanAndroid.dLog("mQrCodeWidth = " + mQrCodeWidth);
				CavanAndroid.dLog("mQrCodeHeight = " + mQrCodeHeight); */
				break;

			case MSG_CAMERA_OPENED:
				if (mListener != null) {
					mListener.onCameraOpened((Camera) msg.obj);
				}
				break;
			}
		}
	};

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

	public void setEventListener(EventListener listener) {
		mListener = listener;
	}

	public EventListener getEventListener() {
		return mListener;
	}

	public synchronized void openCamera(int width, int height) {
		CavanAndroid.dLog("openCamera: width = " + width + ", height = " + height);

		mSurfaceWidth = width;
		mSurfaceHeight = height;
		mCameraHandler.openCameraAsync();
	}

	public synchronized void setSurfaceSize(int width, int height) {
		CavanAndroid.dLog("setSurfaceSize: width = " + width + ", height = " + height);

		mSurfaceWidth = width;
		mSurfaceHeight = height;
		updateQrCodeMatrix();
	}

	public void closeCamera() {
		CavanAndroid.dLog("closeCamera");
		mCameraHandler.closeCamera();
	}

	public boolean startPreview() {
		// CavanAndroid.dLog("startPreview");
		return mCameraHandler.startPreview();
	}

	public void stopPreview() {
		CavanAndroid.dLog("stopPreview");
		mCameraHandler.stopPreview();
	}

	public void updateQrCodeMatrix() {
		mHandler.sendEmptyMessage(MSG_QRCODE_MATRIX);
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

		// CavanAndroid.dLog("width = " + width);
		// CavanAndroid.dLog("height = " + height);

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

		updateQrCodeMatrix();

		canvas.drawRect(left, top, right, bottom, mPaint);
	}

	@Override
	public boolean doCameraInit(Camera camera) {
		if (mListener == null || mListener.doCameraInit(camera)) {
			updateQrCodeMatrix();
			return true;
		}

		return false;
	}

	@Override
	public void onCameraOpened(Camera camera) {
		mHandler.obtainMessage(MSG_CAMERA_OPENED, camera).sendToTarget();
	}

	@Override
	public void onStartAutoFocus(Camera camera) {
		mHandler.sendEmptyMessage(MSG_DECODE_START);
	}

	@Override
	public void onFrameCaptured(byte[] bytes, Camera camera) {
		try {
			PlanarYUVLuminanceSource sourceYUV = new PlanarYUVLuminanceSource(bytes, mVideoWidth, mVideoHeight, mQrCodeX, mQrCodeY, mQrCodeWidth, mQrCodeHeight, false);
			CavanLuminanceSourceRotate90 source = new CavanLuminanceSourceRotate90(sourceYUV);
			Result result = CavanQrCode.decode(mQrCodeReader, source);
			// CavanAndroid.dLog("result = " + result);
			mHandler.obtainMessage(MSG_DECODE_COMPLETE, result).sendToTarget();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
