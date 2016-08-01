package com.jwaoo.android;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanSquareWaveGenerator;

public class JwaooDepthDecoder {

	public static final double DEPTH_STEP = 1.0 / JwaooToySensor.SENSOR_COUNT;

	private int mCount;
	private double mDepth;
	private double mDepthAlign;

	private long mTime;
	private long mTimeInConsume = 1000;
	private long mTimeOutConsume = 1000;

	private boolean mPullOut;

	private CavanSquareWaveGenerator mGenerators[] = new CavanSquareWaveGenerator[JwaooToySensor.SENSOR_COUNT];

	public JwaooDepthDecoder(double fuzz, long timeMin, long timeMax) {
		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i] = new CavanSquareWaveGenerator(fuzz, timeMin, timeMax);
		}
	}

	public void setValueFuzz(double fuzz) {
		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setValueFuzz(fuzz);
		}
	}

	public void setTimeMin(long time) {
		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setTimeMin(time);
		}
	}

	public void setTimeMax(long time) {
		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setTimeMax(time);
		}
	}

	public double getDepth() {
		return mDepth;
	}

	private double getPrediction(long time, long consume) {
		double prediction = (time - mTime) * DEPTH_STEP / consume;
		if (prediction < DEPTH_STEP) {
			return prediction;
		}

		return DEPTH_STEP;
	}

	public double putValue(int[] capacitys) {
		int count;

		CavanAndroid.logE(String.format("capacity: [%4d, %4d, %4d]", capacitys[0], capacitys[1], capacitys[2]));

		for (count = 0; count < JwaooToySensor.SENSOR_COUNT && mGenerators[count].putValue(capacitys[count]); count++);

		for (int i = count + 1; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].putValue(capacitys[i]);
			mGenerators[i].setValue(false);
		}

		CavanAndroid.logE("count = " + count);

		long time = System.currentTimeMillis();

		if (count > mCount) {
			mPullOut = false;

			mTimeInConsume = time - mTime;
			mTime = time;

			mDepth = mDepthAlign = count * DEPTH_STEP;
		} else if (count < mCount) {
			mPullOut = true;

			mTimeOutConsume = time - mTime;
			mTime = time;

			mDepth = mDepthAlign = count * DEPTH_STEP;
		} else if (mPullOut) {
			double prediction = getPrediction(time, mTimeOutConsume);
			mDepth = mDepthAlign - prediction;
			if (mDepth < 0) {
				mDepth = 0;
			}
		} else {
			double prediction = getPrediction(time, mTimeInConsume);
			mDepth = mDepthAlign + prediction;
			if (mDepth > 1.0) {
				mDepth = 1.0;
			}
		}

		mCount = count;

		CavanAndroid.logE("depth = " + mDepth);

		return mDepth;
	}
}
