package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;

import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.qrcode.QRCodeReader;

@SuppressWarnings("deprecation")
public class CavanQrCodeView extends View implements PreviewCallback, AutoFocusCallback {

	private static final int MSG_START_DECODE = 0;
	private static final int MSG_DECODE_COMPLETE = 1;

	public interface DecodeEventListener {
		void onDecodeStart();
		void onDecodeComplete(Result result);
	}

	private Paint mPaint = new Paint();

	private int mViewWidth;
	private int mViewHeight;

	private QRCodeReader mQrCodeReader = new QRCodeReader();
	private Rect mQrCodeRect = new Rect();
	private int mQrCodeWidth;

	private Camera mCamera;
	private DecodeEventListener mListener;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_START_DECODE:
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
			if (mViewWidth > 0 && mViewHeight > 0 && mListener != null) {
				int width = msg.arg1;
				int height = msg.arg2;
				int qrLeft = mQrCodeRect.left * height / mViewWidth;
				int qrTop = mQrCodeRect.top * width / mViewHeight;
				int qrWidth = mQrCodeWidth * height / mViewWidth;
				int qrHeight = mQrCodeWidth * width / mViewHeight;

				PlanarYUVLuminanceSource source = new PlanarYUVLuminanceSource((byte[]) msg.obj, width, height, qrLeft, qrTop, qrWidth, qrHeight, false);
				onDecodeComplete(CavanQrCode.decode(mQrCodeReader, source));
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

	public int getViewWidth() {
		return mViewWidth;
	}

	public int getViewHeight() {
		return mViewHeight;
	}

	public void setDecodeEventListener(DecodeEventListener listener) {
		mListener = listener;
	}

	public DecodeEventListener getDecodeEventListener() {
		return mListener;
	}

	public synchronized Camera openCamera() {
		if (mCamera == null) {
			mCamera = Camera.open();
			if (mCamera == null) {
				return null;
			}
		}

		mCamera.setDisplayOrientation(90);

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
		if (mHandler.hasMessages(MSG_START_DECODE)) {
			return;
		}

		if (mListener != null) {
			mListener.onDecodeStart();
		}

		if (mCamera != null) {
			mCamera.startPreview();
			mCamera.autoFocus(this);
			mHandler.sendEmptyMessageDelayed(MSG_START_DECODE, 1000);
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.drawARGB(0x7F, 0x00, 0x00, 0x00);

		int width = canvas.getWidth();
		int height = canvas.getHeight();

		if (width != mViewWidth || height != mViewHeight) {
			mViewWidth = width;
			mViewHeight = height;

			mQrCodeWidth = Math.min(width, height);

			int left = mQrCodeWidth / 5;
			int right = mQrCodeWidth - left;

			mQrCodeWidth = right - left + 1;
			mQrCodeRect.left = mQrCodeRect.top = left;
			mQrCodeRect.right = mQrCodeRect.bottom = right;
		}

		canvas.drawRect(mQrCodeRect, mPaint);
	}

	@Override
	public void onAutoFocus(boolean success, Camera camera) {
		CavanAndroid.pLog("success = " + success);

		if (mCamera != null) {
			if (success) {
				mHandler.sendEmptyMessage(MSG_START_DECODE);
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
