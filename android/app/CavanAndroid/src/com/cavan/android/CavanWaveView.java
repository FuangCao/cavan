package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class CavanWaveView extends View {

	private Paint mPaint = new Paint();
	private int mBackColor = Color.BLUE;

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

	public void setBackColor(int color) {
		mBackColor = color;
	}

	public void setLineColor(int color) {
		mPaint.setColor(color);
	}

	public void setLineWidth(float width) {
		mPaint.setStrokeWidth(width);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.drawColor(mBackColor);
		canvas.drawLine(0, 0, getWidth(), getHeight(), mPaint);
		super.onDraw(canvas);
	}
}
