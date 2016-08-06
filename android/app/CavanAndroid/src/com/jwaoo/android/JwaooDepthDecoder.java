package com.jwaoo.android;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanSquareWaveCounter;

@SuppressWarnings("serial")
public class JwaooDepthDecoder extends CavanSquareWaveCounter {

	public static final double DEPTH_STEP = 1.0 / JwaooToySensor.SENSOR_COUNT;
	public static final double DEPTH_STEP_HALF = DEPTH_STEP / 2;

	private int mCount;
	private double mDepth;
	private double mDepthAlign;

	private boolean mPullOut;

	private JwaooDepthSquareWaveGenerator mGenerators[] = new JwaooDepthSquareWaveGenerator[JwaooToySensor.SENSOR_COUNT];

	public JwaooDepthDecoder(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);

		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i] = new JwaooDepthSquareWaveGenerator(fuzz, timeMin, timeMax);
		}
	}

	@Override
	public void setValueFuzz(double fuzz) {
		super.setValueFuzz(fuzz);

		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setValueFuzz(fuzz);
		}
	}

	@Override
	public void setTimeMin(long time) {
		super.setTimeMin(time);

		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setTimeMin(time);
		}
	}

	@Override
	public void setTimeMax(long time) {
		super.setTimeMax(time);

		for (int i = 0; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].setTimeMax(time);
		}
	}

	public double getDepth() {
		return mDepth;
	}

	public double putCapacityValue(double[] capacitys) {
		double sum = 0;

		for (int i = capacitys.length - 1; i >= 0; i--) {
			sum += capacitys[i];
		}

		putFreqValue(sum);

		int count;

		for (count = 0; count < JwaooToySensor.SENSOR_COUNT && mGenerators[count].putValue(capacitys[count]); count++);

		for (int i = count + 1; i < JwaooToySensor.SENSOR_COUNT; i++) {
			mGenerators[i].putValue(capacitys[i]);
			mGenerators[i].setValue(false);
		}

		CavanAndroid.logE("count = " + count);

		if (count < JwaooToySensor.SENSOR_COUNT) {
			if (count > 0) {
				if (count > mCount) {
					mPullOut = false;
					mDepth = mDepthAlign = count * DEPTH_STEP - DEPTH_STEP_HALF;
				} else if (count < mCount) {
					mPullOut = true;
					mDepth = mDepthAlign = count * DEPTH_STEP - DEPTH_STEP_HALF;
				} else if (mPullOut) {
					if (count > 1) {
						double depth = mGenerators[count - 1].getDepth() + mGenerators[count - 2].getDepth();
						if (depth < 1.5 && depth > 0.5) {
							mDepth = mDepthAlign - DEPTH_STEP * (1.5 - depth);
						}
					} else {
						double depth = mGenerators[0].getDepth();
						if (depth < 0.5) {
							mDepth = mDepthAlign - DEPTH_STEP * (0.5 - depth);
						}
					}
				} else {
					double depth = mGenerators[count].getDepth() + mGenerators[count - 1].getDepth();
					if (depth > 0.5) {
						if (depth < 1.5) {
							depth -= 0.5;
						} else {
							depth = 1;
						}

						mDepth = mDepthAlign + DEPTH_STEP * depth;
					}
				}
			} else {
				mDepth = mDepthAlign = 0;
			}
		} else {
			mDepth = mDepthAlign = 1;
		}

		mCount = count;

		CavanAndroid.logE("depth = " + mDepth);

		return mDepth;
	}
}
