package com.cavan.blesensor;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Handler;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

import com.cavan.android.CavanBleChar;
import com.cavan.android.CavanBleScanner;
import com.cavan.android.CavanBleUart;
import com.cavan.android.CavanAndroid;

public class MainActivity extends Activity {

	public static int BLE_SCAN_RESULT = 1;

	private CavanBleUart mBleUart;
	private BluetoothDevice mDevice;
	private MySurfaceView mSurfaceView;
	private Handler mHandler = new Handler();

	class MySurfaceView extends SurfaceView implements Callback {

		private boolean mRunning;
		private int mHeight;
		private float mValue;
		private Paint mPaint;
		private float[] mPoints;
		private SurfaceHolder mHolder;

		public MySurfaceView(Context context) {
			super(context);

			mHolder = getHolder();
			mHolder.addCallback(this);

			mPaint = new Paint();
			mPaint.setStrokeWidth(2);
			mPaint.setColor(Color.YELLOW);
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			CavanAndroid.logP();

			mRunning = true;

			new Thread() {

				@Override
				public void run() {
					while (mRunning) {
						addPoint(mValue);
					}
				}
			}.start();
		}

		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			CavanAndroid.logP();
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			CavanAndroid.logP();
			mRunning = false;
		}

		public void addPoint(float point) {
			Canvas canvas = mHolder.lockCanvas();
			if (canvas == null) {
				return;
			}

			canvas.drawColor(Color.BLACK);

			if (mPoints == null) {
				mPoints = new float[canvas.getWidth() * 2];
				mHeight = canvas.getHeight() / 2;

				for (int i = 0; i < mPoints.length; i += 2) {
					mPoints[i] = i / 2;
					mPoints[i + 1] = mHeight;
				}
			}

			int index = mPoints.length - 2;

			for (int i = 0; i < index; i += 2) {
				mPoints[i] = mPoints[i + 2] - 1;
				mPoints[i + 1] = mPoints[i + 3];
			}

			mPoints[index] = index / 2;
			mPoints[index + 1] = mHeight + (float) (point * mHeight / 2 / 9.8);

			canvas.drawLines(mPoints, mPaint);

			mHolder.unlockCanvasAndPost(canvas);
		}

		public void setValue(float value) {
			mValue = value;
		}

		public void addValue(float value) {
			mValue = (mValue * 19 + value) / 20;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mSurfaceView = new MySurfaceView(this);
		setContentView(mSurfaceView);

		CavanBleScanner.show(this, BLE_SCAN_RESULT);
	}

	@Override
	protected void onDestroy() {
		if (mBleUart != null) {
			mBleUart.disconnect();
		}

		super.onDestroy();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (requestCode == BLE_SCAN_RESULT && resultCode == RESULT_OK && data != null) {
			mDevice = data.getParcelableExtra("device");
			if (mDevice == null) {
				finish();
			}

			mHandler.post(new Runnable() {

				@Override
				public void run() {

					mBleUart = new CavanBleUart(mDevice) {

						@Override
						protected void onDisconnected() {
							CavanBleScanner.show(MainActivity.this, BLE_SCAN_RESULT);
						}

						@Override
						protected void onDataReceived(CavanBleChar bleChar, byte[] data) {
							if (data.length == 14) {
								// double accX = ((double) (short) ((data[0] & 0xFF) << 8 | (data[1] & 0xFF))) * 9.8 / 16384;
								double accY = ((double) (short) ((data[2] & 0xFF) << 8 | (data[3] & 0xFF))) * 9.8 / 16384;
								// double accZ = ((double) (short) ((data[4] & 0xFF) << 8 | (data[5] & 0xFF))) * 9.8 / 16384;
								// double temp = (((double) (short) ((data[6] & 0xFF) << 8 | (data[7] & 0xFF))) + 13200) / 280 - 13;
								// double gyrX = ((double) (short) ((data[8] & 0xFF) << 8 | (data[9] & 0xFF))) / 131;
								// double gyrY = ((double) (short) ((data[10] & 0xFF) << 8 | (data[11] & 0xFF))) / 131;
								// double gyrZ = ((double) (short) ((data[12] & 0xFF) << 8 | (data[13] & 0xFF))) / 131;

								// CavanAndroid.logE(String.format("ACC: [%f, %f, %f]", accX, accY, accZ));
								// CavanAndroid.logE(String.format("GYR: [%f, %f, %f]", gyrX, gyrY, gyrZ));
								// CavanAndroid.logE("temp = " + temp);
								mSurfaceView.setValue((float) accY);;
							}
						}
					};

					if (!mBleUart.connect(MainActivity.this)) {
						finish();
					}
				}
			});
		} else {
			finish();
		}
	}
}
