package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;

public class CavanCursorView extends View {

	public static int WIDTH = 30;

	private WindowManager mManager;
	private WindowManager.LayoutParams mParams;
	private Paint mPaint;

	public CavanCursorView(Context context, LayoutParams params) {
		super(context);

		mPaint = new Paint();
		mPaint.setColor(Color.RED);

		mManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

		mParams = params;
		params.width = WIDTH;
		params.height = WIDTH;
		mManager.addView(this, params);
	}

	public void setPosition(int x, int y) {
		mParams.x = x;
		mParams.y = y;
		mManager.updateViewLayout(this, mParams);
	}

	public void addPosition(int x, int y) {
		setPosition(mParams.x + x, mParams.y + y);
	}

	public int getCursorX() {
		return (int) (mParams.x + WIDTH / 2);
	}

	public int getCursorY() {
		return (int) (mParams.y + WIDTH / 2);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		CavanAndroid.dLog("onDraw");

		mPaint.setColor(Color.RED);

		float radius = WIDTH / 2;

		canvas.drawCircle(radius, radius, radius, mPaint);
	}

}
