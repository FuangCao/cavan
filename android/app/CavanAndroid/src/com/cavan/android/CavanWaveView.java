package com.cavan.android;

import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class CavanWaveView extends View {

	private Paint mPaint = new Paint();
	private int mBackColor = Color.BLACK;

	private int mZoom = 1;
	private double mValueMin;
	private double mValueRange;
	private List<Integer> mPoints = new LinkedList<Integer>();

	public CavanWaveView(Context context) {
		super(context);
		init();
	}

	public CavanWaveView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	public CavanWaveView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		init();
	}

	public CavanWaveView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		init();
	}

	private void init() {
		mPaint.setColor(Color.YELLOW);
	}

	public void setZoom(int zoom) {
		if (zoom > 0) {
			mZoom = zoom;
		}
	}

	public void setBackColor(int color) {
		mBackColor = color;
	}

	public void setLineColor(int color) {
		mPaint.setColor(color);
	}

	public void setLineWidth(float width) {
		mPaint.setStrokeWidth(width);
	}

	public void setValueRange(double min, double max) {
		mValueMin = min;
		mValueRange = max - min;
	}

	public boolean addValue(double value) {
		int height = getHeight();
		int point = (int) ((value - mValueMin) * height / mValueRange);
		if (point < 0) {
			point = 0;
		} else if (point >= height) {
			point = height - 1;
		}

		synchronized (mPoints) {
			mPoints.add(height - point - 1);
		}

		if (CavanAndroid.isMainThread()) {
			invalidate();
		} else {
			postInvalidate();
		}

		return true;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.drawColor(mBackColor);

		synchronized (mPoints) {
			int count = mPoints.size();
			int width = getWidth();
			int maxCount = width / mZoom;

			while (count > maxCount) {
				mPoints.remove(0);
				count--;
			}

			if (count > 1) {
				int y1 = mPoints.get(--count);

				for (int x = width - 1; count > 0; x -= mZoom) {
					int y0 = mPoints.get(--count);

					canvas.drawLine(x - mZoom, y0, x, y1, mPaint);
					y1 = y0;
				}
			}
		}

		super.onDraw(canvas);
	}
}
